/*
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <alljoyn/AllJoynStd.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/Init.h>
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/ProxyBusObject.h>
#include <qcc/Log.h>
#include <qcc/String.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <time.h>
using namespace ajn;

/* constants. */
static const char* CHAT_SERVICE_INTERFACE_NAME = "org.alljoyn.bus.samples.chat";
static const char* NAME_PREFIX = "org.alljoyn.bus.samples.chat.";
static const char* CHAT_SERVICE_OBJECT_PATH = "/chatService";
static const SessionPort CHAT_PORT = 27;

/* static data. */
static ajn::BusAttachment* s_bus = NULL;
static qcc::String s_advertisedName;
static qcc::String s_joinName;
static qcc::String s_sessionHost;
static SessionId s_sessionId = 0;
static bool s_joinComplete = false;
static volatile sig_atomic_t s_interrupt = false;

static void CDECL_CALL SigIntHandler(int sig)
{
    QCC_UNUSED(sig);
    s_interrupt = true;
}

/*
 * get a line of input from the the file pointer (most likely stdin).
 * This will capture the the num-1 characters or till a newline character is
 * entered.
 *
 * @param[out] str a pointer to a character array that will hold the user input
 * @param[in]  num the size of the character array 'str'
 * @param[in]  fp  the file pointer the sting will be read from. (most likely stdin)
 *
 * @return returns the same string as 'str' if there has been a read error a null
 *                 pointer will be returned and 'str' will remain unchanged.
 */
char*get_line(char*str, size_t num, FILE*fp)
{
    char*p = fgets(str, num, fp);

    // fgets will capture the '\n' character if the string entered is shorter than
    // num. Remove the '\n' from the end of the line and replace it with nul '\0'.
    if (p != NULL) {
        size_t last = strlen(str) - 1;
        if (str[last] == '\n') {
            str[last] = '\0';
        }
    }

    return s_interrupt ? NULL : p;
}

/* Bus object */
class ChatObject : public BusObject {
  public:

    ChatObject(BusAttachment& bus, const char* path) : BusObject(path), chatSignalMember(NULL)
    {
        QStatus status;

        /* Add the chat interface to this object */
        const InterfaceDescription* chatIntf = bus.GetInterface(CHAT_SERVICE_INTERFACE_NAME);
        assert(chatIntf);
        AddInterface(*chatIntf);

        /* Store the Chat signal member away so it can be quickly looked up when signals are sent */
        chatSignalMember = chatIntf->GetMember("Chat");
        assert(chatSignalMember);

        /* Register signal handler */
        status =  bus.RegisterSignalHandler(this,
                                            static_cast<MessageReceiver::SignalHandler>(&ChatObject::ChatSignalHandler),
                                            chatSignalMember,
                                            NULL);

        if (ER_OK != status) {
            printf("Failed to register signal handler for ChatObject::Chat (%s)\n", QCC_StatusText(status));
        }
    }

    /** Send a Chat signal */
    QStatus SendChatSignal(const char* msg) {

        MsgArg chatArg("s", msg);
        uint8_t flags = 0;
        if (0 == s_sessionId) {
            printf("Sending Chat signal without a session id\n");
        }
        return Signal(NULL, s_sessionId, *chatSignalMember, &chatArg, 1, 0, flags);
    }

    /** Receive a signal from another Chat client */
    void ChatSignalHandler(const InterfaceDescription::Member* member, const char* srcPath, Message& msg)
    {
        QCC_UNUSED(member);
        QCC_UNUSED(srcPath);
        printf("%s: %s\n", msg->GetSender(), msg->GetArg(0)->v_string.str);
    }

  private:
    const InterfaceDescription::Member* chatSignalMember;
};

class MyBusListener : public BusListener, public SessionPortListener, public SessionListener {
    void FoundAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
    {
        printf("FoundAdvertisedName(name='%s', transport = 0x%x, prefix='%s')\n", name, transport, namePrefix);

        if (s_sessionHost.empty()) {
            const char* convName = name + strlen(NAME_PREFIX);
            printf("Discovered chat conversation: \"%s\"\n", convName);

            /* Join the conversation */
            /* Since we are in a callback we must enable concurrent callbacks before calling a synchronous method. */
            s_sessionHost = name;
            s_bus->EnableConcurrentCallbacks();
            SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
            QStatus status = s_bus->JoinSession(name, CHAT_PORT, this, s_sessionId, opts);
            if (ER_OK == status) {
                printf("Joined conversation \"%s\"\n", convName);
            } else {
                printf("JoinSession failed (status=%s)\n", QCC_StatusText(status));
            }
            uint32_t timeout = 20;
            status = s_bus->SetLinkTimeout(s_sessionId, timeout);
            if (ER_OK == status) {
                printf("Set link timeout to %d\n", timeout);
            } else {
                printf("Set link timeout failed\n");
            }
            s_joinComplete = true;
        }
    }
    void LostAdvertisedName(const char* name, TransportMask transport, const char* namePrefix)
    {
        QCC_UNUSED(namePrefix);
        printf("Got LostAdvertisedName for %s from transport 0x%x\n", name, transport);
    }
    void NameOwnerChanged(const char* busName, const char* previousOwner, const char* newOwner)
    {
        printf("NameOwnerChanged: name=%s, oldOwner=%s, newOwner=%s\n", busName, previousOwner ? previousOwner : "<none>",
               newOwner ? newOwner : "<none>");
    }
    bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts)
    {
        if (sessionPort != CHAT_PORT) {
            printf("Rejecting join attempt on non-chat session port %d\n", sessionPort);
            return false;
        }

        printf("Accepting join session request from %s (opts.proximity=%x, opts.traffic=%x, opts.transports=%x)\n",
               joiner, opts.proximity, opts.traffic, opts.transports);
        return true;
    }

    void SessionJoined(SessionPort sessionPort, SessionId id, const char* joiner)
    {
        QCC_UNUSED(sessionPort);

        s_sessionId = id;
        printf("SessionJoined with %s (id=%d)\n", joiner, id);
        s_bus->EnableConcurrentCallbacks();
        uint32_t timeout = 20;
        QStatus status = s_bus->SetLinkTimeout(s_sessionId, timeout);
        if (ER_OK == status) {
            printf("Set link timeout to %d\n", timeout);
        } else {
            printf("Set link timeout failed\n");
        }
    }
};

/* More static data. */
static ChatObject* s_chatObj = NULL;
static MyBusListener s_busListener;

#ifdef __cplusplus
extern "C" {
#endif

/** Send usage information to stdout and exit with EXIT_FAILURE. */
static void Usage()
{
    printf("Usage: chat [-h] [-s <name>] | [-j <name>]\n");
    exit(EXIT_FAILURE);
}

/** Parse the the command line arguments. If a problem occurs exit via Usage(). */
static void ParseCommandLine(int argc, char** argv)
{
    /* Parse command line args */
    for (int i = 1; i < argc; ++i) {
        if (0 == ::strcmp("-s", argv[i])) {
            if ((++i < argc) && (argv[i][0] != '-')) {
                s_advertisedName = NAME_PREFIX;
                s_advertisedName += argv[i];
            } else {
                printf("Missing parameter for \"-s\" option\n");
                Usage();
            }
        } else if (0 == ::strcmp("-j", argv[i])) {
            if ((++i < argc) && (argv[i][0] != '-')) {
                s_joinName = NAME_PREFIX;
                s_joinName += argv[i];
            } else {
                printf("Missing parameter for \"-j\" option\n");
                Usage();
            }
        } else if (0 == ::strcmp("-h", argv[i])) {
            Usage();
        } /*else {
            printf("Unknown argument \"%s\"\n", argv[i]);
            Usage();
        }*/
    }
}

/** Validate the data obtained from the command line. If invalid exit via Usage(). */
void ValidateCommandLine()
{
    /* Validate command line */
    if (s_advertisedName.empty() && s_joinName.empty()) {
        printf("Must specify either -s or -j\n");
        Usage();
    } else if (!s_advertisedName.empty() && !s_joinName.empty()) {
        printf("Cannot specify both -s  and -j\n");
        Usage();
    }
}

/** Create the interface, report the result to stdout, and return the result status. */
QStatus CreateInterface(void)
{
    /* Create org.alljoyn.bus.samples.chat interface */
    InterfaceDescription* chatIntf = NULL;
    QStatus status = s_bus->CreateInterface(CHAT_SERVICE_INTERFACE_NAME, chatIntf);

    if (ER_OK == status) {
        chatIntf->AddSignal("Chat", "s",  "str", 0);
        chatIntf->Activate();
    } else {
        printf("Failed to create interface \"%s\" (%s)\n", CHAT_SERVICE_INTERFACE_NAME, QCC_StatusText(status));
    }

    return status;
}

/** Start the message bus, report the result to stdout, and return the status code. */
QStatus StartMessageBus(void)
{
    QStatus status = s_bus->Start();

    if (ER_OK == status) {
        printf("BusAttachment started.\n");
    } else {
        printf("Start of BusAttachment failed (%s).\n", QCC_StatusText(status));
    }

    return status;
}

/** Register the bus object and connect, report the result to stdout, and return the status code. */
QStatus RegisterBusObject(void)
{
    QStatus status = s_bus->RegisterBusObject(*s_chatObj);

    if (ER_OK == status) {
        printf("RegisterBusObject succeeded.\n");
    } else {
        printf("RegisterBusObject failed (%s).\n", QCC_StatusText(status));
    }

    return status;
}

/** Connect, report the result to stdout, and return the status code. */
QStatus ConnectBusAttachment(void)
{
    QStatus status = s_bus->Connect();

    if (ER_OK == status) {
        printf("Connect to '%s' succeeded.\n", s_bus->GetConnectSpec().c_str());
    } else {
        printf("Failed to connect to '%s' (%s).\n", s_bus->GetConnectSpec().c_str(), QCC_StatusText(status));
    }

    return status;
}

/** Request the service name, report the result to stdout, and return the status code. */
QStatus RequestName(void)
{
    QStatus status = s_bus->RequestName(s_advertisedName.c_str(), DBUS_NAME_FLAG_DO_NOT_QUEUE);

    if (ER_OK == status) {
        printf("RequestName('%s') succeeded.\n", s_advertisedName.c_str());
    } else {
        printf("RequestName('%s') failed (status=%s).\n", s_advertisedName.c_str(), QCC_StatusText(status));
    }

    return status;
}

/** Create the session, report the result to stdout, and return the status code. */
QStatus CreateSession(TransportMask mask)
{
    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true, SessionOpts::PROXIMITY_ANY, mask);
    SessionPort sp = CHAT_PORT;
    QStatus status = s_bus->BindSessionPort(sp, opts, s_busListener);

    if (ER_OK == status) {
        printf("BindSessionPort succeeded.\n");
    } else {
        printf("BindSessionPort failed (%s).\n", QCC_StatusText(status));
    }

    return status;
}

/** Advertise the service name, report the result to stdout, and return the status code. */
QStatus AdvertiseName(TransportMask mask)
{
    QStatus status = s_bus->AdvertiseName(s_advertisedName.c_str(), mask);

    if (ER_OK == status) {
        printf("Advertisement of the service name '%s' succeeded.\n", s_advertisedName.c_str());
    } else {
        printf("Failed to advertise name '%s' (%s).\n", s_advertisedName.c_str(), QCC_StatusText(status));
    }

    return status;
}

/** Begin discovery on the well-known name of the service to be called, report the result to
   stdout, and return the result status. */
QStatus FindAdvertisedName(void)
{
    /* Begin discovery on the well-known name of the service to be called */
    QStatus status = s_bus->FindAdvertisedName(s_joinName.c_str());

    if (status == ER_OK) {
        printf("org.alljoyn.Bus.FindAdvertisedName ('%s') succeeded.\n", s_joinName.c_str());
    } else {
        printf("org.alljoyn.Bus.FindAdvertisedName ('%s') failed (%s).\n", s_joinName.c_str(), QCC_StatusText(status));
    }

    return status;
}

/** Wait for join session to complete, report the event to stdout, and return the result status. */
QStatus WaitForJoinSessionCompletion(void)
{
    unsigned int count = 0;

    while (!s_joinComplete && !s_interrupt) {
        if (0 == (count++ % 100)) {
            printf("Waited %u seconds for JoinSession completion.\n", count / 100);
        }

#ifdef _WIN32
        Sleep(10);
#else
        usleep(10 * 1000);
#endif
    }

    return s_joinComplete && !s_interrupt ? ER_OK : ER_ALLJOYN_JOINSESSION_REPLY_CONNECT_FAILED;
}

/** Take input from stdin and send it as a chat message, continue until an error or
 * SIGINT occurs, return the result status. */
QStatus DoTheChat(void)
{
    const int bufSize = 1024;
    char buf[bufSize];
    QStatus status = ER_OK;
    while ((ER_OK == status) && (get_line(buf, bufSize, stdin))) {
        status = s_chatObj->SendChatSignal(buf);
		
    }

    return status;
}
int waite;
void sig_hander(int signo){
	if(signo==SIGINT){
		waite=0;
		signo=0;
	}
}

QStatus DoTheChat_Client(char *device)
{
	waite=1;
    QStatus status = ER_OK;
	srand(time(NULL));
	char ans[10000];
	while ((ER_OK==status)&&(waite==1)){
	
		signal(SIGINT,sig_hander);
		memset(ans,0,sizeof ans);//empty ans 

		int AI1,AI2,AI3,AI4;//rand value of AI ...DO..
		int DO1,DO2,DO3,DO4;

		///4010

		if(strcmp(device,"4010")==0){
			struct DeviceIO{

				char AI1[10],AI2[10],AI3[10],AI4[10];
				char DO1[10],DO2[10],DO3[10],DO4[10];
				char end[10];// because end of data processing has problem
			};
			struct Descript{
				char des_ai1[100],des_ai2[100],des_ai3[100],des_ai4[100],
				     des_do1[100],des_do2[100],des_do3[100],des_do4[100];
			};
			struct DeviceData{
				char head[10];//because front of data processing has problem
				char name[100];
				struct DeviceIO tag;
				struct Descript des;
			} WISE_4010;
			//WISE_4012.name="s";

			AI1=(rand()%100+1);
			AI2=(rand()%100+1);
			AI3=(rand()%100+1);
			AI4=(rand()%100+1);
			DO1=(rand()%2);
			DO2=(rand()%2);
			DO3=(rand()%2);
			DO4=(rand()%2);
			
			sprintf(WISE_4010.tag.AI1,"%d",AI1);
			sprintf(WISE_4010.tag.AI2,"%d",AI2);
			sprintf(WISE_4010.tag.AI3,"%d",AI3);
			sprintf(WISE_4010.tag.AI4,"%d",AI4);
			sprintf(WISE_4010.tag.DO1,"%d",DO1);
			sprintf(WISE_4010.tag.DO2,"%d",DO2);
			sprintf(WISE_4010.tag.DO3,"%d",DO3);
			sprintf(WISE_4010.tag.DO4,"%d",DO4);
			
			strcpy(WISE_4010.head,"head");
			strcpy(WISE_4010.name,"WISE-4010");
			strcpy(WISE_4010.des.des_ai1,"this is 4010 ai1");
			strcpy(WISE_4010.des.des_ai2,"this is 4010 ai2");
			strcpy(WISE_4010.des.des_ai3,"this is 4010 ai3");
			strcpy(WISE_4010.des.des_ai4,"this is 4010 ai4");
			strcpy(WISE_4010.des.des_do1,"this is 4010 do1");
			strcpy(WISE_4010.des.des_do2,"this is 4010 do2");
			strcpy(WISE_4010.des.des_do3,"this is 4010 do3");
			strcpy(WISE_4010.des.des_do4,"this is 4010 do4");
			strcpy(WISE_4010.tag.end,"end");


			strcat(ans,WISE_4010.head);
			strcat(ans,",");
			strcat(ans,WISE_4010.name);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.AI1);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.AI2);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.AI3);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.AI4);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.DO1);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.DO2);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.DO3);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.DO4);
			strcat(ans,",");
			strcat(ans,WISE_4010.des.des_ai1);
			strcat(ans,",");
			strcat(ans,WISE_4010.des.des_ai2);
			strcat(ans,",");
			strcat(ans,WISE_4010.des.des_ai3);
			strcat(ans,",");
			strcat(ans,WISE_4010.des.des_ai4);
			strcat(ans,",");
			strcat(ans,WISE_4010.des.des_do1);
			strcat(ans,",");
			strcat(ans,WISE_4010.des.des_do2);
			strcat(ans,",");
			strcat(ans,WISE_4010.des.des_do3);
			strcat(ans,",");
			strcat(ans,WISE_4010.des.des_do4);
			strcat(ans,",");
			strcat(ans,WISE_4010.tag.end);

		}

		///// 4012
		else if(strcmp(device,"4012")==0){
			int AI1,AI2;
			int DI1,DI2,DO1,DO2;
			struct DeviceIO{

				char AI1[10],AI2[10],DI1[10],DI2[10];
				char DO1[10],DO2[10];
				char end[10];// because end of data processing has problem
			};
			struct Descript{
				char des_ai1[100],des_ai2[100],
					 des_di1[100],des_di2[100],
					 des_do1[100],des_do2[100];
			};
			struct DeviceData{
				char head[10];//because front of data processing has problem
				char name[100];
				struct DeviceIO tag;
				struct Descript des;
			} WISE_4012;

			AI1=(rand()%100+1);
			AI2=(rand()%100+1);
			DI1=(rand()%2);
			DI2=(rand()%2);
			DO1=(rand()%2);
			DO2=(rand()%2);
			
			sprintf(WISE_4012.tag.AI1,"%d",AI1);
			sprintf(WISE_4012.tag.AI2,"%d",AI2);
			sprintf(WISE_4012.tag.DI1,"%d",DI1);
			sprintf(WISE_4012.tag.DI2,"%d",DI2);
			sprintf(WISE_4012.tag.DO1,"%d",DO1);
			sprintf(WISE_4012.tag.DO2,"%d",DO2);
			
			strcpy(WISE_4012.head,"head");
			strcpy(WISE_4012.name,"WISE-4012E");
			strcpy(WISE_4012.des.des_ai1,"this 4012 ai1");
			strcpy(WISE_4012.des.des_ai2,"this 4012 ai2");
			strcpy(WISE_4012.des.des_di1,"this 4012 di1");
			strcpy(WISE_4012.des.des_di2,"this 4012 di2");
			strcpy(WISE_4012.des.des_do1,"this 4012 do1");
			strcpy(WISE_4012.des.des_do2,"this 4012 do2");
			strcpy(WISE_4012.tag.end,"end");


			strcat(ans,WISE_4012.head);
			strcat(ans,",");
			strcat(ans,WISE_4012.name);
			strcat(ans,",");
			strcat(ans,WISE_4012.tag.AI1);
			strcat(ans,",");
			strcat(ans,WISE_4012.tag.AI2);
			strcat(ans,",");
			strcat(ans,WISE_4012.tag.DI1);
			strcat(ans,",");
			strcat(ans,WISE_4012.tag.DI2);
			strcat(ans,",");
			strcat(ans,WISE_4012.tag.DO1);
			strcat(ans,",");
			strcat(ans,WISE_4012.tag.DO2);
			strcat(ans,",");
			strcat(ans,WISE_4012.des.des_ai1);
			strcat(ans,",");
			strcat(ans,WISE_4012.des.des_ai2);
			strcat(ans,",");
			strcat(ans,WISE_4012.des.des_di1);
			strcat(ans,",");
			strcat(ans,WISE_4012.des.des_di2);
			strcat(ans,",");
			strcat(ans,WISE_4012.des.des_do1);
			strcat(ans,",");
			strcat(ans,WISE_4012.des.des_do2);
			strcat(ans,",");
			strcat(ans,WISE_4012.tag.end);
		}

		/////4200
		else if(strcmp(device,"4200")==0){
			int hum,tmp;
			struct Descript{
				char des_hum[100],des_tmp[100];
			};
			struct DeviceData{
				char head[10];//because front of data processing has problem
				char name[100];
				char hum[10],tmp[10];
				struct Descript des;
				char end[10];// because end of data processing has problem
			} WISE_4200;

			hum=(rand()%40+30);
			tmp=(rand()%25+15);
			
			sprintf(WISE_4200.hum,"%d",hum);
			sprintf(WISE_4200.tmp,"%d",tmp);
			
			strcpy(WISE_4200.head,"head");
			strcpy(WISE_4200.name,"WISE-4200");
			strcpy(WISE_4200.end,"end");
			strcpy(WISE_4200.des.des_hum,"this humidity");
			strcpy(WISE_4200.des.des_tmp,"this temperature");

			strcat(ans,WISE_4200.head);
			strcat(ans,",");
			strcat(ans,WISE_4200.name);
			strcat(ans,",");
			strcat(ans,WISE_4200.hum);
			strcat(ans,",");
			strcat(ans,WISE_4200.tmp);
			strcat(ans,",");

			strcat(ans,WISE_4200.des.des_hum);
			strcat(ans,",");
			strcat(ans,WISE_4200.des.des_tmp);
			strcat(ans,",");
			strcat(ans,WISE_4200.end);
		}else{
			
			strcat(ans,"====");//push data
		}
		
		
		status = s_chatObj->SendChatSignal(ans);
		sleep(3);
	}
	return status;
}

int CDECL_CALL main(int argc, char** argv)
{
    if (AllJoynInit() != ER_OK) {
        return 1;
    }
#ifdef ROUTER
    if (AllJoynRouterInit() != ER_OK) {
        AllJoynShutdown();
        return 1;
    }
#endif

    /* Install SIGINT handler. */
    signal(SIGINT, SigIntHandler);

    ParseCommandLine(argc, argv);
    ValidateCommandLine();

    QStatus status = ER_OK;

    /* Create message bus */
    s_bus = new BusAttachment("chat", true);

    if (s_bus) {
        if (ER_OK == status) {
            status = CreateInterface();
        }

        if (ER_OK == status) {
            s_bus->RegisterBusListener(s_busListener);
        }

        if (ER_OK == status) {
            status = StartMessageBus();
        }

        /* Create the bus object that will be used to send and receive signals */
        ChatObject chatObj(*s_bus, CHAT_SERVICE_OBJECT_PATH);

        s_chatObj = &chatObj;

        if (ER_OK == status) {
            status = RegisterBusObject();
        }

        if (ER_OK == status) {
            status = ConnectBusAttachment();
        }

        /* Advertise or discover based on command line options */
        if (!s_advertisedName.empty()) {
            /*
             * Advertise this service on the bus.
             * There are three steps to advertising this service on the bus.
             * 1) Request a well-known name that will be used by the client to discover
             *    this service.
             * 2) Create a session.
             * 3) Advertise the well-known name.
             */
            if (ER_OK == status) {
                status = RequestName();
            }

            const TransportMask SERVICE_TRANSPORT_TYPE = TRANSPORT_ANY;

            if (ER_OK == status) {
                status = CreateSession(SERVICE_TRANSPORT_TYPE);
            }

            if (ER_OK == status) {
                status = AdvertiseName(SERVICE_TRANSPORT_TYPE);
            }
			if (ER_OK == status) {
				status = DoTheChat();
			}
        } else {
            if (ER_OK == status) {
                status = FindAdvertisedName();
            }

            if (ER_OK == status) {
                status = WaitForJoinSessionCompletion();
            }
			if (ER_OK == status) {
				status = DoTheChat_Client(argv[3]);
				//status=DoTheChat_Client();
			}
        }

    } else {
        status = ER_OUT_OF_MEMORY;
    }

    /* Cleanup */
    delete s_bus;
    s_bus = NULL;

    printf("Chat exiting with status 0x%04x (%s).\n", status, QCC_StatusText(status));

#ifdef ROUTER
    AllJoynRouterShutdown();
#endif
    AllJoynShutdown();
    return (int) status;
}

#ifdef __cplusplus
}
#endif
