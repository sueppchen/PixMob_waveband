#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  readList.py
#  
#  Copyright 2024 sueppchen <sueppchen@DNS>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  
#  
import io
import serial
import time

blueStart = 0x0                  #first file
blueEnd   = 0x9                  #last file
serialDevice = "/dev/ttyAMA0"
serialSpeed  = 9600

global fileLength
fileLength = 4096



def main(args):
    #open terminal
    global tty
    tty = serial.Serial(serialDevice, serialSpeed, timeout=1)

    global mainList
    global trueList
    global falseList
    global proofList

    mainList=[]
    trueList=[]
    falseList=[]
    proofList=[]


    def standby():
        #time.sleep(1)
        write_place  = ('1\n').encode()
        tty.write(write_place)
        print("1= %s " % tty.readline())
        tty.flush()

    def pixmobInit():
        if(serialDevice == "/dev/ttyUSB0"):
            time.sleep(4)
            print(": %s " % tty.readline())
            print(": %s " % tty.readline())
            print(": %s " % tty.readline())
            print(": %s " % tty.readline())
            print(": %s " % tty.readline())
            time.sleep(1)
            write_place  = ('s\n').encode()
            tty.write(write_place)
            print("s= %s " % tty.readline())
            tty.flush()
        standby()
    
    def sendLine(line):
        #line[9] -=1
        for i in range(0,9):
            write_place  = ('\n').encode()
            tty.write(write_place)
            tty.readline()
#            print("init: %s " % tty.readline())

        write_place  = ('q\n').encode()
        tty.write(write_place)
        tty.readline()

        for i in range(1,10):
            if( int( tty.read() ) == (i-1) ):
                #print( "ok" , end = " | ")
                write_place  = ('%d\n' % line[i]).encode()
                tty.write(write_place)
                readback = int(tty.readline()[1:3])

        #time.sleep(1)
        response = tty.readline()
        
        return response
    
    def readFiles():
        
        for blueNumber in range(blueStart , blueEnd+1):
            with io.open("blue_"+str("%02x" % blueNumber)+".txt", 'r') as datei:
                lineNumber = 0
                for line in datei:
                    if(line.rstrip()[34:60] != ''):
                        liste=[
                        True,
                        int('0x'+line.rstrip()[34:36], 0),    #1 crcA
                        int('0x'+line.rstrip()[37:39], 0),    #2 mode
                        int('0x'+line.rstrip()[40:42], 0),    #3 green
                        int('0x'+line.rstrip()[43:45], 0),    #4 red
                        int('0x'+line.rstrip()[46:48], 0),    #5 blue
                        int('0x'+line.rstrip()[49:51], 0),    #6 att+rand
                        int('0x'+line.rstrip()[52:54], 0),    #7 rel+hold
                        int('0x'+line.rstrip()[55:57], 0),    #8 group
                        int('0x'+line.rstrip()[58:60], 0),    #9 crcB
                        0x00                                  #10 new crcB
                        ]
                        # subtract 1
                        x = ((liste[1]*64 + liste[9]) - 1)    # -1
                        liste[1] = (x >> 6)
                        liste[9] = (x & 0x3f)
                        trueList.append(liste)
                    else:
                        liste=[False,
                               0x00,                          # crcA 
                               0x00,                          # mode
                               (lineNumber & 0x3f),           # green
                               (lineNumber >> 6),             # red    
                               blueNumber,                    # blue
                               0x08,                          # att+rand
                               0x12,                          # rel + hold
                               0x00,                          # group
                               0x00,                          # crcB
                               0x00                           #10 new crcB
                               ]   
                        falseList.append(liste)
                    mainList.append(liste)
                    lineNumber += 1
            datei.close

    
    def proofData(block):
        for entry in range(0,fileLength):
            zeile = mainList[(block*fileLength + entry)]
            if(zeile[0] == True):
                print(zeile, end = " : ")
    #            zeile[9] -= 1
                if(sendLine( zeile) != b''):
                    print(" -1")
                    zeile[10] = zeile[9]
                else:
                    print("fail", end="")
                    zeile[9] += 1
                    if(sendLine( zeile) != b''):
                        print(" 0")
                        zeile[10] = zeile[9]
                    else:
                        print("fail",end="")
                        zeile[9] -= 2
                        if(sendLine( zeile) != b''):
                            print("-2")
                            zeile[10] = zeile[9]
                        else:
                            print("other")
                            zeile[0] = False
            proofList.append(zeile)

            if(len(proofList) > (fileLength -1)):
                break


    #init device
    pixmobInit()
    #alle daten einlesen
    readFiles()
    
    #auswerten
    for outfile in range(0,int(len(mainList) / fileLength)):
#    for outfile in range(0,2):
        proofList=[]
        proofData(outfile)
        with io.open("proof_%02d.txt" % outfile, 'w') as datei:
            for counter in range(0,len(proofList)):
                line = proofList[counter]
                datei.write("%01x " % int(line[0]) +
                            "%02x " % line[1] +  
                            "%02x " % line[2] +  
                            "%02x " % line[3] +  
                            "%02x " % line[4] +  
                            "%02x " % line[5] +  
                            "%02x " % line[6] +  
                            "%02x " % line[7] +  
                            "%02x " % line[8] +  
                            "%02x" % line[10]
                            )
                if(counter < (len(proofList) -1)):
                    datei.write("\r\n")
    
    gleiche = [0] * 4096
    for zeile in trueList:
        gleiche[zeile[1]*64 + zeile[9]] += 1

    for value in range(0, 4096):
        if (gleiche[value] == 0):
            print(str(value >> 6) + "+" +str(value & 0x3f) + " = 0")

    for zeile in falseList:
        print(zeile)

    print(gleiche)

    print("len list = %d" % len(mainList))

    standby()
    tty.close
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
