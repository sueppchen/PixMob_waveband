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
import time

fileName   = "blue_"
fileStart  = 0x0
fileEnd    = 0x0
fileEnding = ".list"

mainList=[]
trueList=[]
falseList = []

def main(args):

    def readFiles():
        global mainList
        global falseList
        global trueList
        
        for fileNumber in range(fileStart , (fileEnd + 1)):
            with io.open(fileName + str("%01d" % fileNumber) + fileEnding, 'r') as readFile:
                for line in readFile:
                    liste=[
                            bool(int(line.rstrip()[0:1], 0)),     #0 valid
                            int('0x'+line.rstrip()[2:4], 0),      #1 crcA
                            int('0x'+line.rstrip()[5:7], 0),      #2 mode
                            int('0x'+line.rstrip()[8:10], 0),     #3 green
                            int('0x'+line.rstrip()[11:13], 0),    #4 red
                            int('0x'+line.rstrip()[14:16], 0),    #5 blue
                            int('0x'+line.rstrip()[17:19], 0),    #6 att+rand
                            int('0x'+line.rstrip()[20:22], 0),    #7 rel+hold
                            int('0x'+line.rstrip()[23:25], 0),    #8 group
                            int('0x'+line.rstrip()[26:28], 0)     #9 crcB
                            ]
                    if(liste[0] == True):
                        trueList.append(liste)
                    else:
                        falseList.append(liste)
                    mainList.append(liste)
            readFile.close

    #read all lines of data
    readFiles()

    #process

    print("trueList")
    trueSameList = [ [] for _ in range(4096)]
    
    trueSame = [0] * 4096
    for line in trueList:
        trueSame[line[1]*64 + line[9]] += 1
        trueSameList[line[1]*64 + line[9]].append(line)
        
        
    '''for same in trueSameList[64]:
        print(" %s " % f'{same[3]:06b}' + 
              " %s " % f'{same[4]:06b}' +
              " %s " % f'{same[5]:06b}' +
              " ")
    print()
    '''
        
    for value in range(0, 4096):
        if (len(trueSameList[value]) == 0):
            print(str(value >> 6) + "+" +str(value & 0x3f) + " = 0")

    sortSame = sorted(trueSame,reverse=True)

    for value in range(0, 10):
        print(sortSame[value])


#    print("false = %d" % len(falseList))
#    falseSame = []
#    for line in falseList:
#        if((line[3] > 5) or (line[4] > 5) or (line[5] > 5)):
#            falseSame.append(line)
#            print("F %02x " % line[3] + "%02x " % line[4] + "%02x " % line[5])     

#        if (line[1] == 0):
#            if(line[9] == 5):
#                print(line)
    


#    for line in falseList:
#        print(line)

#    print(same)

    print("processed total of %d entries" %len(mainList))
 
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
