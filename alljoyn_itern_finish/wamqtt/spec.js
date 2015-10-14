// alljoyn variable part
var alljoyn=require('alljoyn');

var portNumber=27;
var InterfaceName='org.alljoyn.bus.samples.chat';
var ObjectName='/chatService';
/*
var InterfaceName='org.alljoyn.Bus.sample';
var ObjectName='/sample';

var portNumber=25;

*/

// mqtt variable part
var mqtt = require('mqtt');
var client,
    connectType = 1,
    host='wavmrd.cloudapp.net',
    myUsername='admin',
    myPassword='admin',
    connectStatus= 0,
    publishInterval = 5000,
    sendData,

    options_ws={
        port: 80,
        username: myUsername,
        password: myPassword
    },
    options_wss={
        port: 443,
        username: myUsername,
        password: myPassword,
        rejectUnauthorized: false
    },

    simWISE4010Data={
        name: "WISE4010",
        tag: {
            WISE4010_AI1: 0,
            WISE4010_AI2: 200,
            WISE4010_AI3: 500,
            WISE4010_AI4: 750,
            WISE4010_DO1: 0,
            WISE4010_DO2: 0,
            WISE4010_DO3: 1,
            WISE4010_DO4: 1
        }
    },
    simWISE4012Data={
        name: "WISE4012E",
        tag: {
            WISE4012E_AI1: 300,
            WISE4012E_AI2: 700,
            WISE4012E_DI1: 0,
            WISE4012E_DI2: 1,
            WISE4012E_DO1: 0,
            WISE4012E_DO2: 1
        }
    },
    simWISE4200Data={
        name: "WISE4200",
        tag: {
            WISE4200_Hum: 48.86,
            WISE4200_Tmp: 26.37
        }
    },

    simWISE4010Cfg={
        ID: "WISE4010",
        Tag: [
            {
                ID: "WISE4010_AI1",
                TID: 1,
                Dsc: ""
            },
            {
                ID: "WISE4010_AI2",
                TID: 1,
                Dsc: ""
            },
            {
                ID: "WISE4010_AI3",
                TID: 1,
                Dsc: ""
            },
            {
                ID: "WISE4010_AI4",
                TID: 1,
                Dsc: ""
            },
            {
                ID: "WISE4010_DO1",
                TID: 2,
                Dsc: ""
            },
            {
                ID: "WISE4010_DO2",
                TID: 2,
                Dsc: ""
            },
            {
                ID: "WISE4010_DO3",
                TID: 2,
                Dsc: ""
            },
            {
                ID: "WISE4010_DO4",
                TID: 2,
                Dsc: ""
            }
        ]
    },
    simWISE4012Cfg={
        ID: "WISE4012E",
        Tag: [
            {
                ID: "WISE4012E_AI1",
                TID: 1,
                Dsc: ""
            },
            {
                ID: "WISE4012E_AI2",
                TID: 1,
                Dsc: ""
            },
            {
                ID: "WISE4012E_DI1",
                TID: 2,
                Dsc: ""
            },
            {
                ID: "WISE4012E_DI2",
                TID: 2,
                Dsc: ""
            },
            {
                ID: "WISE4012E_DO1",
                TID: 2,
                Dsc: ""
            },
            {
                ID: "WISE4012E_DO2",
                TID: 2,
                Dsc: ""
            }
        ]
    },
    simWISE4200Cfg={
        ID: "WISE4200",
        Tag: [
            {
                ID: "WISE4200_Hum",
                TID: 1,
                Dsc: ""
            },
            {
                ID: "WISE4200_Tmp",
                TID: 1,
                Dsc: ""
            }
         ]
    };


// mqtt code part 

if(connectType == 0) //WebSocket
{
    client = mqtt.connect('ws://' + host + '/MQTT/', options_ws);
    console.log('websocket non-secure connect');
}
else //WebSocket tls
{
    client = mqtt.connect('wss://' + host + '/MQTT/', options_wss);
    console.log('websocket secure connect');
}

sendData = setInterval(function(){publishData();}, publishInterval);

function publishData(msg)
{
    if (connectStatus)
    {
		//console.log("Publish received: ", msg);
        var devDataArray = [];
        //devDataArray.push(msg);
        devDataArray.push(simWISE4010Data);
        //devDataArray.push(simWISE4012Data);
        //devDataArray.push(simWISE4200Data);
        alljoynSendData(devDataArray);
    }
}

client.on('message', function (topic, message)
{
    if(message == 'Auto-Discover')
    {
        var devList=[];
        devList.push(simWISE4010Cfg);
        devList.push(simWISE4012Cfg);
        devList.push(simWISE4200Cfg);
        alljoynSendConfig(devList);
    }
});

client.on('connect', function ()
{
    console.log('connect');
    connectStatus = 1;
    client.subscribe('alljoynCmd');
});

client.on('offline', function ()
{
    console.log('offline');
});

client.on('close', function ()
{
    connectStatus = 0;
    console.log('close');
});

client.on('error', function (error)
{
    console.log('error: ' + error);
});

function alljoynSendData(devDataArray)
{
    if(connectStatus == 1)
    {
        var sendData={},
            options={
                qos: 1,
                retain: true
            };
        sendData['Dev'] = devDataArray;
        client.publish('alljoynData', JSON.stringify(sendData), options);
        console.log(JSON.stringify(sendData));
    }
}

function alljoynSendConfig(devList)
{
    if(connectStatus == 1)
    {
        var sendCfg={},
            options={
                qos: 1,
                retain: true
            };
        sendCfg['Dev'] = devList;
        client.publish('alljoynCfg', JSON.stringify(sendCfg), options);
        console.log(JSON.stringify(sendCfg));
    }
}




//alljoyn code part 

//create a bus and sessionId 
/*
var bus = alljoyn.BusAttachment('myAppName');
var sessionId = 0;
//create interface description 
var interface = alljoyn.InterfaceDescription();

// create listener 
var listener = alljoyn.BusListener(
	function(name){
		console.log("FoundAdvertisedName", name);
		//join session, sessionId is used to send messages. 
		sessionId = bus.joinSession(name, portNumber, 0);
	
		//send a signal message 
		object.signal(null, sessionId, interface, "Chat", "Hello, I am a client!");
	},
	function(name){
		console.log("LostAdvertisedName", name);
	},
	function(name){
		console.log("NameOwnerChanged", name);
	}
);

// create the interface 
bus.createInterface(InterfaceName, interface);

// add a signal to the interface, specifying what kind of message we will accept 
// s = string, d = number, b = boolean 
interface.addSignal("Chat", "s",  "msg");

// register the listener on the bus 
bus.registerBusListener(listener)

// initialize the bus 
bus.start();



// create the BusObject that will send and receive messages 
var object = alljoyn.BusObject(ObjectName);

// start listening 
bus.connect();

// discover devices with prefix 'org.alljoyn' 
bus.findAdvertisedName(InterfaceName)


// create a SessionPortListener for session changes 
var portListener = alljoyn.SessionPortListener(
	function(port, joiner){
		console.log("AcceptSessionJoiner", port, joiner);

		//return true to accept the new session member 
		return true;
	},
	function(port, sessId, joiner){
		sessionId = sessId;
		console.log("SessionJoined", port, sessionId, joiner);
	}
);

	// add the chat interface to the BusObject 
	object.addInterface(interface);

	// this function will be called for each received message 
	bus.registerSignalHandler(object, 
		function(msg, info){
			//console.log(info);
			publishData(msg);
			console.log("Message received: ", msg);
		},
	interface, "Chat");

*/
