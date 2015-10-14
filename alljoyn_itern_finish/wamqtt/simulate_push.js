var sys = require('sys');
var exec = require('child_process').exec;

var spawn = require('child_process').spawn;
var sh_process; // child process's name

process.stdin.setEncoding('utf8');

process.stdin.on('readable', function() {
	var chunk = process.stdin.read();// user input 
	

	if(chunk ==="on\n"){// turn on WISE , ie, run client.sh

		process.stdout.write("Turn on WISE\n");

		sh_process=spawn('sh',['/home/xiao/basic/chat_client.sh',40]);//run chat_client
	}else if(chunk ==="off\n"){//trun off WISE,ie,stop client
		
		//console.log("child process: "+sh_process.pid);//get child process'pid
		process.stdout.write("Turn off WISE\n");
		sh_process=spawn('kill',[sh_process.pid+1]);//kill client process

	}else if(chunk ==="quit\n"){
		process.exit();//exit node.js

	}else if(chunk !==null){ 
		process.stdout.write('data: ' + chunk);
		
	}
});

process.stdin.on('end', function() {
	  process.stdout.write('end');
});



function puts(error, stdout, stderr) { 
	sys.puts(stdout) 
}

