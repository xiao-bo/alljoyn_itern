from subprocess import call
import os
import signal
import subprocess

while(1):
    user_input = raw_input("Enter something: ")
    if user_input=='on':
        print "turn on WISE"
        #call(["sh","/home/xiao/basic/basic_client.sh"])
        #pro=subprocess.Popen('sh /home/xiao/basic/basic_client.sh ',stdout=subprocess.PIPE,shell=True,preexec_fn=os.setsid)        
        pro=subprocess.Popen('sh /home/xiao/wamqtt/sh/chat_client_4010.sh ',stdout=subprocess.PIPE,shell=True,preexec_fn=os.setsid)        
    
    elif user_input=='off':
        print "turn off WISE"
        os.killpg(pro.pid,signal.SIGTER)
    else :
        print "you entered ", user_input

