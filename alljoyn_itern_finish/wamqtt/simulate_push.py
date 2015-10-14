from subprocess import call
import os
import signal
import subprocess

while(1):
    user_input = raw_input("Enter something: ")
    if user_input=='on':
        print "turn on WISE"
        #call(["sh","/home/xiao/basic/basic_client.sh"])
        subprocess.Popen('sh /home/xiao/basic/chat_client.sh 000',shell=True,preexec_fn=os.setsid)        
    
    elif user_input=='off':
        print "turn off WISE"
        os.killpg(pro.pid,signal.SIGINT)
    else :
        print "you entered ", user_input

