#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  readList.py
#  
#  Copyright 2024 sueppchen <sueppchen@DNS>
#  
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
from operator import itemgetter


polyBit =[0x2416, 0x082D, 0x371D, 0x2E3B, 0x3B30, 0x1126, 0x050B, 0x0A16, #  0 -  7
          0x142C, 0x0F1F, 0x1E3E, 0x1B3B, 0x1131, 0x0525, 0x2D0D, 0x1A1B, #  8 - 15
          0x3436, 0x0F2A, 0x3913, 0x3227, 0x0308, 0x0610, 0x0C20, 0x3F07, # 16 -
          0x3E0F, 0x3C1F, 0x383F, 0x1738, 0x0937, 0x3529, 0x0D14, 0x1A28, # 24
          0x1317, 0x262E, 0x2B1A, 0x1635, 0x0B2D, 0x311D, 0x223B, 0x2330, # 32
          0x2126, 0x250A, 0x0A15, 0x142A, 0x0F13, 0x1E26, 0x1B0B, 0x3616, # 40
          0x2C2D, 0x3F1C, 0x3E39, 0x1B34, 0x112F, 0x0519, 0x0A32, 0x3323] # 48


def setCRC(message):
    crc = 0x1b05
    for index in range(7):
        value = message[(index + 2)] & 0x3f
        code = reDictBE[value]
        for bit in range(8):
            if(code & (1<<bit)):
                crc ^= polyBit[index][bit]

    message[1] = (crc>>8) & 0x3f
    message[9] = crc & 0x3f
    return crc

def main(args):
    
    def ror12(value,bits):
        helper =  value >> bits
        helper += (value << (12-bits)) & 0xfff
        return helper

    def rol12(value,bits):
        helper =  (value << bits) & 0xfff
        helper += value >> (12-bits)
        return helper


    print("XOR with neigbour")
    
    for step in range(1,55):
        print("step = %s, " % f'{(step):02x}')
        for index in range(0,(56-step)):
            valueA = polyBit[(index+step)]
            valueB = polyBit[index]
            neighbor = valueA ^ valueB
            neighbor12 = ((neighbor >> 8) << 6) + (neighbor & 0x3f)
            print("[0x%s] " % f'{(valueB):04x}' + " ^ [0x%s] " % f'{(valueA):04x}' + "= [0x%s]" % f'{(neighbor):04x}'+ "= [0x%s]" % f'{(neighbor12):012b}',end ="\r\n")

    print("done")

 
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
