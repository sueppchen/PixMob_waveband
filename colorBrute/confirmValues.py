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

rFileName   = "crcRaw/blue_"
rFileStart  = 0
rFileEnd    = 7
rFileEnding = ".txt"

serialDevice = "/dev/ttyUSB0"
#serialDevice = "/dev/ttyACM0"
serialSpeed  = 115200

#write
wFileName   = "blue_"
wFileStart  = 0
wFileEnding = ".list"

#startBlock = 0
fileLength = 32768
writeBlocks = 512         #write each file in 512 blocks a 64 lines

mainList=[]
#trueList=[]
#falseList = []
proofList = []
failList = []

def main(args):
    #open terminal
    tty = serial.Serial(serialDevice, serialSpeed, timeout=1)


    def standby():
        #time.sleep(1)
        write_place  = ('1\n').encode()
        tty.write(write_place)
        print("1= %s " % tty.readline())
        tty.flush()

    def pixmobInit():
        if((serialDevice == "/dev/ttyUSB0")):
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
    
    def sendLine(line):                        #send single values with 'q'
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

    def sendLine2(line):                       #send all values together with 'w'
        write_place  = ('\n').encode()
        tty.write(write_place)
        tty.readline()
#        print("send: %s " % tty.readline())

        write_place  = ('w\n').encode()
        tty.write(write_place)
        tty.readline()
#        print("send: %s " % tty.readline())

        if( int( tty.read() ) == 9 ):
            #print( "ok" , end = " | ")
            helper = (line[1] << 48) + (line[3] << 40) + (line[4] << 32) + (line[5] << 24) + (line[6] << 16) + (line[7] << 8) + line[9]
            write_place  = ('%014x\n' % helper).encode()
            #print(write_place)
            tty.write(write_place)
            tty.readline()

        #time.sleep(1)
        response = tty.readline()
        
        return response
     
    def readFiles():
        global mainList
#        global falseList
#        global trueList
        
        for rFileNumber in range(rFileStart , rFileEnd+1):
            with io.open(rFileName + str("%02d" % rFileNumber) + rFileEnding, 'r') as readFile:
                lineNumber = 0 
                for line in readFile:
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
#                        trueList.append(liste)
                    else:
                        liste=[False,
                               0x00,                          # crcA 
                               0x00,                          # mode
                               (lineNumber & 0x3f),           # green
                               (lineNumber >> 6),             # red    
                               rFileNumber,                   # blue
                               0x08,                          # att+rand
                               0x12,                          # rel + hold
                               0x00,                          # group
                               0x00,                          # crcB
                               0x00                           #10 new crcB
                               ]   
#                        falseList.append(liste)
                    mainList.append(liste)
                    lineNumber += 1
            readFile.close

    def proofData(block):
        global failList
        global proofList
            
        proofList=[]
        for entry in range(0, int(fileLength / writeBlocks)): # 0 - 64
            lineNumber = (block * int(fileLength / writeBlocks) + entry)
            zeile = mainList[lineNumber]                                  #65
            if(zeile[0] == True):
                print(zeile, end = " : ")
    #            zeile[9] -= 1
                if(sendLine2( zeile) != b''):
                    print(" -1")
                    zeile[10] = zeile[9]
                else:
                    print("fail", end=",")
                    zeile[9] += 1
                    if(sendLine2( zeile) != b''):
                        print(" 0")
                        zeile[10] = zeile[9]
                    else:
                        print("fail",end=",")
                        zeile[9] -= 2
                        if(sendLine2( zeile) != b''):
                            print("-2")
                            zeile[10] = zeile[9]
                        else:
                            print("other!")
                            zeile[0] = False
                            failList.append(zeile)
            proofList.append(zeile)

            if(len(proofList) > (int(fileLength /writeBlocks) -1)):
                break

    def writeFile():
        global failList
        global proofList
        print("writing %d lines " % len(mainList) +
               "in %d files " % int(len(mainList) / fileLength) +
               "each in %d blocks " % writeBlocks +
               "with %d lines." % int(fileLength / writeBlocks) +
               "\r\n starting with file %d" % wFileStart
               )
               
        for outfile in range(wFileStart, int(len(mainList) / fileLength)):
    #    for outfile in range(0,2):
            print("block %d:" % int (outfile * writeBlocks) + "(line %d)" % (int (outfile * writeBlocks) * int(fileLength / writeBlocks)))
            proofData(int (outfile * writeBlocks))    #1 * 64
            with io.open(wFileName + str("%01d" % wFileNumber) + wFileEnding, 'w') as writeFile:
                for line in proofList:
                    writeFile.write("%01x " % int(line[0]) +
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
                    writeFile.write("\r\n")
            
            for repeat in range(1, writeBlocks):
                print("block %d:" % (int (outfile * writeBlocks) + repeat ) + "(line %d)" % ((int (outfile * writeBlocks) + repeat) * int(fileLength / writeBlocks) ) )
                
                proofData(int (outfile * writeBlocks) + repeat)  #1*64 + 1
                
                with io.open(wFileName + str("%01d" % wFileNumber) + wFileEnding, 'a') as writeFile:
                    for line in proofList:
                        writeFile.write("%01x " % int(line[0]) +
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
                        writeFile.write("\r\n")
                proofList=[]
                
                if(len(failList) > 0):
                    with io.open("faillist.txt", 'a') as failFile:
                        for line in failList:
                            failFile.write(str(line) + "\r\n")
                    failList = []                                    #clear


    #init device
    pixmobInit()
    #alle daten einlesen
    readFiles()

    #auswerten
    writeFile()
    
    standby()
    tty.close

    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
