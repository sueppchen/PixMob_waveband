#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  crc_variants.py
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

import numpy as np

def main(args):    
    reDictBE = [0x21, 0x35, 0x2c, 0x34, 0x66, 0x26, 0xac, 0x24,   # // 00-07
                0x46, 0x56, 0x44, 0x54, 0x64, 0x6d, 0x4c, 0x6c,    # // 08-0f
                0x92, 0xb2, 0xa6, 0xa2, 0xb4, 0x94, 0x86, 0x96,    # // 10-17
                0x42, 0x62, 0x2a, 0x6a, 0xb6, 0x36, 0x22, 0x32,    # // 18-1f
                0x31, 0xB1, 0x95, 0xB5, 0x91, 0x99, 0x85, 0x89,    # // 20-27
                0xa5, 0xa4, 0x8c, 0x84, 0xa1, 0xa9, 0x8d, 0xad,    # // 28-2f
                0x9a, 0x8a, 0x5a, 0x4a, 0x49, 0x59, 0x52, 0x51,    # // 30-37
                0x25, 0x2d, 0x69, 0x29, 0x4D, 0x45, 0x61, 0x65]    # // 38-3f

    reDictLE = [0x84, 0xac, 0x34, 0x2c, 0x66, 0x64, 0x35, 0x24,    #
                0x62, 0x6a, 0x22, 0x2a, 0x26, 0xb6, 0x32, 0x36,    #
                0x49, 0x4d, 0x65, 0x45, 0x2d, 0x29, 0x61, 0x69,    #
                0x42, 0x46, 0x54, 0x56, 0x6d, 0x6c, 0x44, 0x4c,    #
                0x8c, 0x8d, 0xa9, 0xad, 0x89, 0x99, 0xa1, 0x91,    #
                0xa5, 0x25, 0x31, 0x21, 0x85, 0x95, 0xb1, 0xb5,    #
                0x59, 0x51, 0x5a, 0x52, 0x92, 0x9a, 0x4a, 0x8a,    #
                0xa4, 0xb4, 0x96, 0x94, 0xb2, 0xa2, 0x86, 0xa6]    #

    newList=   [[True, 47, 0, 59, 15, 0, 8, 18, 0, 49],
                [True, 47, 0, 41, 23, 0, 8, 18, 0, 49],
                [True, 47, 0, 27, 35, 0, 8, 18, 0, 49],
                [True, 47, 0, 46, 41, 0, 8, 18, 0, 49],
                [True, 47, 0, 2, 37, 1, 8, 18, 0, 49],
                [True, 47, 0, 50, 0, 2, 8, 18, 0, 49],
                [True, 47, 0, 6, 46, 2, 8, 18, 0, 49],
                [True, 47, 0, 29, 15, 3, 8, 18, 0, 49],
                [True, 47, 0, 16, 41, 3, 8, 18, 0, 49],
                [True, 47, 0, 54, 18, 4, 8, 18, 0, 49],
                [True, 47, 0, 27, 59, 4, 8, 18, 0, 49],
                [True, 47, 0, 18, 45, 5, 8, 18, 0, 49],
                [True, 47, 0, 10, 16, 6, 8, 18, 0, 49],
                [True, 47, 0, 49, 33, 6, 8, 18, 0, 49],
                [True, 47, 0, 8, 34, 7, 8, 18, 0, 49],
                [True, 47, 0, 44, 43, 7, 8, 18, 0, 49],
                [True, 47, 0, 13, 57, 9, 8, 18, 0, 49],
                [True, 47, 0, 10, 29, 12, 8, 18, 0, 49],
                [True, 47, 0, 17, 48, 12, 8, 18, 0, 49],
                [True, 47, 0, 40, 4, 13, 8, 18, 0, 49],
                [True, 47, 0, 42, 11, 13, 8, 18, 0, 49],
                [True, 47, 0, 15, 18, 14, 8, 18, 0, 49],
                [True, 47, 0, 61, 21, 14, 8, 18, 0, 49],
                [True, 47, 0, 19, 38, 14, 8, 18, 0, 49],
                [True, 47, 0, 11, 59, 14, 8, 18, 0, 49],
                [True, 47, 0, 23, 44, 15, 8, 18, 0, 49],
                [True, 47, 0, 31, 58, 15, 8, 18, 0, 49]]

    bits = 16
    
    SHIFT=0b01011001   #shift 8 to 6, (6+5 transform Input), short | reflectOUT, reflectIN, (1+0 transform output)


    polyZoo12 = [0x61e, 0x987, 0xa33, 0x829, 0xddf, 0xbdf, 0x83e, 0xb75,
                 0x8f3, 0xf89, 0x817, 0xc07, 0xbff, 0xb41, 0xb91, 0xe98,
                 0xc05, 0x993, 0xa6f, 0xc06, 0xbae, 0x8f8, 0xa4f]

    polyZoo16 = [0x8d95, 0xfdbf, 0x8016, 0xc7ab, 0x979e, 0x9627, 0xe433, 0xc5db, 
                0xac6f, 0xb82d, 0x88f9, 0x86f2, 0x978a, 0xd175, 0x8fdb, 0xed2f, 
                0xc4bd, 0xb57d, 0xefff, 0xa001, 0x8003, 0x8810, 0x9747, 0xd04b, 
                0x808d, 0x935a, 0xa10e, 0x968b, 0xbaad, 0xd3e9, 0xd015, 0x8ee7, 
                0xcbab, 0xf94f, 0xac9a, 0xb7b1, 0xa755, 0xadc9, 0x82c4, 0x9eb2, 
                0xc86c, 0xe92f]
#    polyZoo16 = [0x8d95] 

    startValue = [0, 0x1d0f, 0xffff]
#    startValue = [0]

    xorList = [0, 1, 0xaaaa, 0xffff]
#    xorList = [0]

    def reflect_data(x, width):
        # See: https://stackoverflow.com/a/20918545
        if width == 8:
            x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1)
            x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2)
            x = ((x & 0x0F) << 4) | ((x & 0xF0) >> 4)
        elif width == 12:
            x = ((x & 0x555) << 1) | ((x & 0xAAA) >> 1)
            x = ((x & 0x333) << 2) | ((x & 0xCCC) >> 2)
            x = ((x & 0x00F) << 8) | ((x & 0xF00) >> 8) | (x & 0x0F0)
        elif width == 16:
            x = ((x & 0x5555) << 1) | ((x & 0xAAAA) >> 1)
            x = ((x & 0x3333) << 2) | ((x & 0xCCCC) >> 2)
            x = ((x & 0x0F0F) << 4) | ((x & 0xF0F0) >> 4)
            x = ((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8)
        elif width == 32:
            x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1)
            x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2)
            x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4)
            x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8)
            x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16)
        else:
            raise ValueError('Unsupported width')
        return x

    def crc_poly(data, n, poly, crc=0, ref_in=False, ref_out=False, xor_out=0):
        
        g = 1 << n | poly  # Generator polynomial
            
        # Loop over the data
        for d in data:
            # Reverse the input byte if the flag is true
            if ref_in:
                d = reflect_data(d, 8)

            # XOR the top byte in the CRC with the input byte
            crc ^= d << (n - 8)

            # Loop over all the bits in the byte
            for _ in range(8):
                # Start by shifting the CRC, so we can check for the top bit
                crc <<= 1

                # XOR the CRC if the top bit is 1
                if crc & (1 << n):
                    crc ^= g

        # Reverse the output if the flag is true
        if ref_out:
            crc = reflect_data(crc, n)

        # Return the CRC value
        return crc ^ xor_out
    
#    lastCrc = 0
    match = 0
    hexstring=''
    
    shifting=6                     # 8 does nothing, set to 6 to remove bit six and seven from each byte and stitch all together
    
    helper=0
    daten=0
    crc=0

    
    
    for SHIFT in range(0,0x80):
#    for SHIFT in range(0x10,0x11):
        for outFx in xorList:
            for init in startValue:
                for key in polyZoo16:
                    
                    crcCalc=[]
                    
                    for entry in newList:
                        # hex originals
                        #print(entry)
                        
                        entry[1] = 0                                #crcA = 0
                        entry[9] = 0                                #crcB = 0
                        helper=0
                        for value in entry[2:]:                     # ignore valid/invalid and crcA
                            if(((SHIFT >> 5) & 0x3) == 1):          # shift left 2
                                value  = value << 2
                            if(((SHIFT >> 5) & 0x3) == 2):          # use 6b8b BE translation
                                value = reDictBE[value]
                            if(((SHIFT >> 5) & 0x3) == 3):          # use 6b8b LE translation
                                value = reDictLE[value]
                            helper  = helper << shifting            # do 6bit or 8 bit processing
                            helper += value
                        
                        #print(": %012x " % helper0, end="\r\n")
                        
                        if(SHIFT & 0x10):                           #short frame from 9 to 7 byte
                            if(shifting==8):
                                helper = helper >> 8                # = 56 bit
                            else:
                                helper = helper >> 6                # = 42 bit
                                
                        hexstring= str(hex(helper)[2:])
                        
                        if(SHIFT & 0x10):                           # short frame
                            if(shifting == 8):                      #  - fill up to 56 bits / 14 hex digits
                                while(len(hexstring) < 14):
                                    hexstring = '0' + hexstring
                            else:                                   #  - fill up to 42 bits / 12 hex digits
                                while(len(hexstring) < 12):
                                    hexstring = '0' + hexstring
                        else:                                       # long frame
                            if(shifting == 8):                      #  - fill up to 72 bits / 18 hex digits
                                while(len(hexstring) < 18):
                                    hexstring = '0' + hexstring
                            else:                                   #  - fill up to 54 bits / 14 hex digits
                                while(len(hexstring) < 14):
                                    hexstring = '0' + hexstring
                                                    
                        #print (hexstring)
                        
                        daten = bytearray.fromhex(hexstring)
                        #print (daten)
                                        
#                        if(SHIFT & 0x10):                              #short frame to 7 byte
#                            daten = daten[1:][:7]       
                        #print (daten)

                        crc = crc_poly(daten, bits, key, crc=init, ref_in=(SHIFT & 0x4), ref_out=(SHIFT & 0x8), xor_out=outFx)
                        
                        if(bits>12 and ((SHIFT & 0x3) == 0)):    #shift until 2x6 bit remain
                            crc=crc>>(bits-12)
                            
                        if(bits>12 and ((SHIFT & 0x3) == 1)):    # split into 2x8 bit and shift to 2x6 bit
                            helperH = ((crc >> 10) & 0x3F) 
                            helperL = ((crc >> 2) & 0x3F)
                            crc =  (helperH << 6) + helperL

                        if(bits>12 and ((SHIFT & 0x3) == 2)):   #cut until 2x6 bit remain
                            crc &= 0xfff

                        if(bits>12 and ((SHIFT & 0x3) == 3)):   # split to 2x8 bit and cut to 2x 6bit
                            helperH = ((crc >> 8) & 0x3F) 
                            helperL = (crc & 0x3F)
                            crc =  (helperH << 6) + helperL
                        
                        crcCalc.append(crc)
                   # print(crcCalc)
                    match = 1
#                    for number in range(1,len(crcCalc)):
                    for number in range(1,5):
                        if (crcCalc[number] != crcCalc[number-1]):
                            match = 0
                    
                    if(match == 1):
                        print("mean =  " +str(crcCalc))
                        print(daten)
                        print("S= %x " % SHIFT + " FX=%x" % outFx + " INIT = %x" %init + " KEY= %x" %key )
                     


    return 0
 
    
if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
