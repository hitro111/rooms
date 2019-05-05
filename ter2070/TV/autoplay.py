import subprocess
import os.path
import time
import sys
import os
import select
import RPi.GPIO as GPIO

global pr
global bg_pr
started=False
counter=0

GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

i=0
while i<300:
    sys.stdout.write("\n")
    i=i+1
start = 0
end = 0
    
while True:
    if GPIO.input(4) == False:
        start = time.time()
        while GPIO.input(4) == False:
            time.sleep(0.01)
        end = time.time()
        if end - start > 0.05:
            break
    time.sleep(0.1)
    
time.sleep(0.3)
bg_pr = subprocess.Popen(['omxplayer', '--no-osd', '--loop',  '/home/pi/Videos/bg.mp4'],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE, close_fds=True)

while True:      
    if GPIO.input(4) == False:
        start = time.time()
        while GPIO.input(4) == False:
            time.sleep(0.01)
        end = time.time()
        
        if end - start < 0.1:
            continue;
    
        file = ''

        if end - start > 4:
            try:
                bg_pr.stdin.write('q')
                bg_pr.stdin.flush()
            except: 
                pass
            try:
                pr.stdin.write('q')
                pr.stdin.flush()
            except: 
                pass
            break
        elif end - start > 2.9:
            file = '/home/pi/Videos/fail.mp4'
        elif end - start > 1.9:
            file = '/home/pi/Videos/success.mp4'
        elif end - start > 1.1:
            file = '/home/pi/Videos/5.mp4'
        elif end - start > 0.9:
            file = '/home/pi/Videos/4.mp4'
        elif end - start > 0.7:
            file = '/home/pi/Videos/3.mp4'
        elif end - start > 0.5:
            file = '/home/pi/Videos/2.mp4'
        elif end - start > 0.3:
            file = '/home/pi/Videos/1.mp4'
        
        pr = subprocess.Popen(['omxplayer', '--no-osd', file],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE, close_fds=True)
        try:
            bg_pr.stdin.write('q')
            bg_pr.stdin.flush()
        except:
            pass
        
        ex = False
        while ex == False:
            if GPIO.input(4) == False:
                start = time.time()
                while GPIO.input(4) == False:
                    time.sleep(0.01)
                end = time.time()
                if end - start > 0.1:
                    ex = True
                    break
            time.sleep(0.01)
        
        time.sleep(0.2)
        
        bg_pr = subprocess.Popen(['omxplayer', '--no-osd', '--loop', '/home/pi/Videos/bg.mp4'],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE, close_fds=True)
        try:
            pr.stdin.write('q')
            pr.stdin.flush()
        except: 
            pass
        
    time.sleep(0.1)
