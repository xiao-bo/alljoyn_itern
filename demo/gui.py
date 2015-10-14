#!/usr/bin/env python
import threading
import os
import signal
import sys
import subprocess
import time
from PyQt4.QtGui import *
from PyQt4.QtCore import *
from PyQt4 import QtCore


class MyWidget(QWidget):
    def __init__(self,parent=None):
        super(MyWidget,self).__init__(parent)
        self.createLayout()
        
    def createLayout(self):
        
        ##4010 part######
        ##image
        self.label_img_4010=QLabel(self)
        pixmap=QPixmap("red.png")
        self.label_img_4010.setGeometry(350,40,50,50)
        self.label_img_4010.setPixmap(pixmap)
        self.label_img_4010.move(350,40)
        
        ##4010 push button config
        
        self.btn_4010_on=QPushButton("on",self)
        self.btn_4010_off=QPushButton("off",self)
        self.btn_4010_on.setStyleSheet('font-size:18pt;')
        self.btn_4010_off.setStyleSheet('font-size:18pt;')
        self.btn_4010_off.setEnabled(False) 
        
        ## 4010 button click
        self.btn_4010_on.clicked.connect(self.Click_4010_on)
        self.btn_4010_off.clicked.connect(self.Click_4010_off)
        #self.btn_4010_on.clicked.connect(self.test)
        #self.btn_4010_off.clicked.connect(self.test)

        ##4010 label
        label_4010=QLabel(self)
        label_4010.setText("4010")
        label_4010.setStyleSheet('font-size:18pt;') 
       

        ### position

        self.btn_4010_on.move(150,40)
        self.btn_4010_off.move(250,40)
        label_4010.move(50,40)
        ## 4010 finish###
        
        
        ##4012 part######
        
        ##image
        self.label_img_4012=QLabel(self)
        pixmap=QPixmap("red.png")
        self.label_img_4012.setGeometry(350,120,50,50)
        self.label_img_4012.setPixmap(pixmap)
        self.label_img_4012.move(350,120)
        
        ##4012 push button config
        
        self.btn_4012_on=QPushButton("on",self)
        self.btn_4012_off=QPushButton("off",self)
        self.btn_4012_on.setStyleSheet('font-size:18pt;')
        self.btn_4012_off.setStyleSheet('font-size:18pt;')
        self.btn_4012_off.setEnabled(False) 
        
        ## 4012 button click
        self.btn_4012_on.clicked.connect(self.Click_4012_on)
        self.btn_4012_off.clicked.connect(self.Click_4012_off)

        ##4012 label
        label_4012=QLabel(self)
        label_4012.setText("4012")
        label_4012.setStyleSheet('font-size:18pt;') 
        
        ### position

        self.btn_4012_on.move(150,120)
        self.btn_4012_off.move(250,120)
        label_4012.move(50,120)
        
        ## 4012 finish###

        ##4200 part######
        
        
        self.label_img_4200=QLabel(self)
        pixmap=QPixmap("red.png")
        self.label_img_4200.setGeometry(350,200,50,50)
        self.label_img_4200.setPixmap(pixmap)
        self.label_img_4200.move(350,200)
        

        ##4200 push button config
      
        self.btn_4200_on=QPushButton("on",self)
        self.btn_4200_off=QPushButton("off",self)
        self.btn_4200_on.setStyleSheet('font-size:18pt;')
        self.btn_4200_off.setStyleSheet('font-size:18pt;')
        self.btn_4200_off.setEnabled(False) 
        
        ## 4200 button click
        self.btn_4200_on.clicked.connect(self.Click_4200_on)
        self.btn_4200_off.clicked.connect(self.Click_4200_off)

        ##4200 label
        label_4200=QLabel(self)
        label_4200.setText("4200")
        label_4200.setStyleSheet('font-size:18pt;') 
       

        ### position

        self.btn_4200_on.move(150,200)
        self.btn_4200_off.move(250,200)
        label_4200.move(50,200)

        ## 4200 finish###

       
        self.setWindowTitle("client device")
        self.resize(500,280)
        self.show()



    ###button function


    def Click_4010_on(self):
        ##call alljoyn 
        subprocess.Popen("sh /home/xiao/demo/sh/chat_client_4010.sh",shell=True)
                
        ##button mutual exclusion

        self.btn_4010_on.setEnabled(False)
        self.btn_4010_off.setEnabled(True)
        
        ##change image 
        pixmap=QPixmap("green.png")
        self.label_img_4010.setGeometry(350,40,50,50)
        self.label_img_4010.setPixmap(pixmap)

    def Click_4010_off(self):
        
        ##kill process
        p=subprocess.Popen(['ps','-fu','xiao'],stdout=subprocess.PIPE)
        out,err=p.communicate()
        for line in out.splitlines():
            if '/home/xiao/demo/bin/chat_4010' in line:
                pid=int(line.split()[1])
                os.kill(pid,signal.SIGINT)
                print pid
        
        ##change image 
        pixmap=QPixmap("red.png")
        self.label_img_4010.setGeometry(350,40,50,50)
        self.label_img_4010.setPixmap(pixmap)
        
        ##button mutual exclusion
        self.btn_4010_on.setEnabled(True)
        self.btn_4010_off.setEnabled(False)
    
    
    def Click_4012_on(self):
        
        ## call alljoyn 
        #subprocess.Popen("./bin/chat_4012 -j training 4012",shell=True,preexec_fn=os.setsid)
        subprocess.Popen("sh /home/xiao/demo/sh/chat_client_4012.sh",shell=True)
        
        ##change image 
        pixmap=QPixmap("green.png")
        self.label_img_4012.setGeometry(350,120,50,50)
        self.label_img_4012.setPixmap(pixmap)
        
        
        ##button mutual exclusion
        self.btn_4012_on.setEnabled(False)
        self.btn_4012_off.setEnabled(True)
        
    
    def Click_4012_off(self):
        
        ##kill process
        p=subprocess.Popen(['ps','-fu','xiao'],stdout=subprocess.PIPE)
        out,err=p.communicate()
        for line in out.splitlines():
            if '/home/xiao/demo/bin/chat_4012' in line:
                pid=int(line.split()[1])
                os.kill(pid,signal.SIGINT)
                print pid

        ##change image 
        pixmap=QPixmap("red.png")
        self.label_img_4012.setGeometry(350,120,50,50)
        self.label_img_4012.setPixmap(pixmap)
        
        ##button mutual exclusion
        self.btn_4012_on.setEnabled(True)
        self.btn_4012_off.setEnabled(False)


    def Click_4200_on(self):
        
        
        
        ## call alljoyn 
        
        subprocess.Popen("sh /home/xiao/demo/sh/chat_client_4200.sh ",shell=True)
        #subprocess.Popen("sh /home/xiao/basic/chat_client_4200.sh",shell=True,preexec_fn=os.setsid)
    
        ##change image 
        pixmap=QPixmap("green.png")
        self.label_img_4200.setGeometry(350,200,50,50)
        self.label_img_4200.setPixmap(pixmap)
        
        ##button mutual exclusion
        self.btn_4200_on.setEnabled(False)
        self.btn_4200_off.setEnabled(True)
    
    def Click_4200_off(self):
        
        ##kill process
        p=subprocess.Popen(['ps','-fu','xiao'],stdout=subprocess.PIPE)
        out,err=p.communicate()
        for line in out.splitlines():
            if '/home/xiao/demo/bin/chat_4200' in line:
                pid=int(line.split()[1])
                os.kill(pid,signal.SIGINT)
                print pid
        
        
        
        ##change image 
        pixmap=QPixmap("red.png")
        self.label_img_4200.setGeometry(350,200,50,50)
        self.label_img_4200.setPixmap(pixmap)
        
        ##button mutual exclusion
        self.btn_4200_on.setEnabled(True)
        self.btn_4200_off.setEnabled(False)
        


         

def main():
    app = QApplication(sys.argv)
    widget = MyWidget()


    app.exec_()

if __name__=='__main__':
    main()


