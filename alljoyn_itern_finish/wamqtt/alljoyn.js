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
var start= new Date().getTime();
var devDataArray = [];
var flag_4010=0,flag_4012=0,flag_4200=0;//first turn on flag
var time_4200=0;//heartbeat
var time_4012=0;
var time_4010=0;
var devList=[];
var index_4010,index_4012,index_4200;//compute index of device in devList
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
    };
    simWISE4010Data={
        name: null,
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
        name: null,
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
        name: null,
        tag: {
            WISE4200_Hum: 48.86,
            WISE4200_Tmp: 26.37
        }
    },

/*
  */
    simWISE4010Cfg={
        ID: "WISE4010",
        Tag: [
            {
                ID: "WISE4010_AI1",
                TID: 1,
                Dsc: null
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
                Dsc: null
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
                Dsc: null
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

//sendData = setInterval(function(){publishData();}, publishInterval);

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
		console.log("data====");
        console.log(JSON.stringify(sendData));
		console.log("====data");
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
			
			var s=JSON.stringify(msg);
			var data=s.split(",");
			//avoid when device turn off, but had store data into memory
			//and keep displaying device's data
			console.log("msg");
			simWISE4010Data.name=null;
			simWISE4012Data.name=null;	
			simWISE4200Data.name=null;

			if(data[1]=="WISE-4010"){
				parseData(msg,simWISE4010Data,simWISE4010Cfg);//msg  append to 4010 data/cfg
				time_4010=0;// time reset, if time >5 , mean 4010 turn off
				if(flag_4010==0){ // 4010 first turn on 
					parseConfig();
					flag_4010=1; 
				}			
			}else if(data[1]=="WISE-4012E"){
				parseData(msg,simWISE4012Data,simWISE4012Cfg);
				time_4012=0;
				
				if(flag_4012==0){
					parseConfig();
					flag_4012=1;
				}			
			}else if(data[1]=="WISE-4200"){
				parseData(msg,simWISE4200Data,simWISE4200Cfg);
				time_4200=0;
				if(flag_4200==0){
					parseConfig();
					flag_4200=1;
				}			
			}
			
			 if(data[1]!="WISE-4010"){//if is not data about 4010, and it start count++
				time_4010++;
				console.log("time_4010=",time_4010);
				
				if(time_4010==6){       // mean 4010 turn off
					parseConfig();		//send 4010 turn off message to cloud
					flag_4010=0;        
				}
				
				if(time_4010>100){      // avoid overflow
					time_4010=15;
				}
			}
			if(data[1]!="WISE-4012E"){	
				time_4012++;
				console.log("time_4012=",time_4012);
				
				if(time_4012==6){
					parseConfig();		
					flag_4012=0;
				}
				
				if(time_4012>100){
					time_4012=15;
				}
			}
			if(data[1]!="WISE-4200"){
				time_4200++;
				console.log("time_4200=",time_4200);
				
				if(time_4200==6){
					parseConfig();		
					flag_4200=0;
				}
				
				if(time_4200>100){
					time_4200=15;
				}
			}
			publishData(simWISE4010Data,simWISE4012Data,simWISE4200Data);
		},
	interface, "Chat");


function parseData(msg,object_data,object_cfg){	
	var s=JSON.stringify(msg);
	var data=s.split(",");

	var i=2;//remove index of head and name 
	
	object_data.name=data[1]; // apend name into DATA
	
	for(var x in object_data.tag){//apend device's data
		object_data.tag[x]=parseInt(data[i],10);
		i++;
	}
	
	for(var j=0;j<object_cfg.Tag.length;j++){//append device's cfg
		object_cfg.Tag[j].Dsc=data[i];
		i++;
	}
}
function publishData(WISE4010Data,WISE4012Data,WISE4200Data)
{
	devDataArray.splice(0,devDataArray.length);
    if (connectStatus)
    {
        if(WISE4010Data.name!=null){//if name =null,mean that do not receive  4010 data 
			devDataArray.push(WISE4010Data);
		}
        if(WISE4012Data.name!=null){
			devDataArray.push(WISE4012Data);
		}

        if(WISE4200Data.name!=null){
			devDataArray.push(WISE4200Data);
		}
		console.log("data====");
        //console.log(JSON.stringify(devDataArray));
		//console.log("====data");
        alljoynSendData(devDataArray);
    }
}

function parseConfig(){

	console.log("parseconfig first");
	devList.splice(0,devList.length);
	if(simWISE4010Cfg.Tag[0].Dsc!=null){ //if Dsc=null , mean that no receive 4010 cfg
		devList.push(simWISE4010Cfg);
	}
	if(simWISE4012Cfg.Tag[0].Dsc!=null){
		devList.push(simWISE4012Cfg);
	}	

	if(simWISE4200Cfg.Tag[0].Dsc!=null){
		devList.push(simWISE4200Cfg);
	}

	console.log("parseconfig second");

	if(time_4010>5){//if time>5 , mean that 4010 turn off
		console.log("pop 4010===========");
		index_4010=devList.indexOf(simWISE4010Cfg);
		if(index_4010>-1){//index =-1 ,mean simWISE4010Cfg is not in devList 
			devList.splice(index_4010,1);
		}
	}
	if(time_4012>5){
		console.log("pop 4012===========");
		index_4012=devList.indexOf(simWISE4012Cfg);
		if(index_4012>-1){
			devList.splice(index_4012,1);
		}
	}

	if(time_4200>5){
		console.log("pop 4200===========");
		index_4200=devList.indexOf(simWISE4200Cfg);
		if(index_4200>-1){
			devList.splice(index_4200,1);
		}
	}


	console.log("===cfg");
	alljoynSendConfig(devList);
	console.log("cfg===");
}

client.on('message', function (topic, message)
{
    if(message == 'Auto-Discover')
	{
		console.log("auto discover first");
        devList=[];
		 if(simWISE4010Cfg.Tag[0].Dsc!=null){ //if Dsc=null, mean do not receive 4010 cfg
			devList.push(simWISE4010Cfg);
		}
        if(simWISE4012Cfg.Tag[0].Dsc!=null){
			devList.push(simWISE4012Cfg);
		}	
		
        if(simWISE4200Cfg.Tag[0].Dsc!=null){
			devList.push(simWISE4200Cfg);
		}


		console.log("auto discover middle");
		if(time_4010>5){//index =-1 ,mean simWISE4010Cfg is not in devList 
			console.log("pop 4010===========");
			index_4010=devList.indexOf(simWISE4010Cfg);
			if(index_4010>-1){
				devList.splice(index_4010,1);
			}
		}
		 if(time_4012>5){
			console.log("pop 4012===========");
			index_4012=devList.indexOf(simWISE4012Cfg);
			if(index_4012>-1){
				devList.splice(index_4012,1);
			}
		}
		
		if(time_4200>5){
			console.log("pop 4200===========");
			index_4200=devList.indexOf(simWISE4200Cfg);
			if(index_4200>-1){
				devList.splice(index_4200,1);
			}
		}

		
        console.log("===cfg");
        alljoynSendConfig(devList);
        console.log("cfg===");
    }
});

var end=new Date().getTime();
var time=end-start;
console.log("time=",time);
