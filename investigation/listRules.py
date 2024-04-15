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

INIT = 0x3202       # reference CRC = CRC of message "32 00 00 00 00 00 00 00 02" opposit of 0x0d3d

_NONE_ = 0x0000     # missing values are replaced by _NONE_ (0x0000) for calculation


           #   0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F
table  = [ 
           # mode
           [0x0000, 0x0c2d, 0x3d30, _NONE_, 0x1e2d, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_,  # 0
            0x0c3b, 0x1d1d, 0x1130, 0x262d, 0x222d, 0x330b, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 0x2c3b, 0x170b,  # 1 
            0x3b30, _NONE_, 0x171d, _NONE_, 0x2000, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_,  # 2
            0x2200, 0x1930, 0x2d1d, 0x162d, 0x3a16, 0x0126, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_],  # 3
           # mode0: green, mode2: mem(end<<4 | start ), mode4:green1,
           [0x0000, 0x0f0f, 0x1129, 0x1b23, 0x2800, 0x050d, 0x0b32, 0x0a12, 0x2d25, 0x3c14, 0x223a, 0x330b, 0x271f, 0x2808, 0x3901, 0x3c24,  # 0
            0x153c, 0x1019, 0x1f16, 0x0128, 0x0138, 0x041d, 0x1a33, 0x0b02, 0x331b, 0x363e, 0x0008, 0x2d05, 0x0e27, 0x143c, 0x1b33, 0x0a02,  # 1
            0x1131, 0x0b2a, 0x1031, 0x1514, 0x0e0f, 0x1534, 0x0100, 0x0405, 0x0425, 0x1009, 0x0e17, 0x152c, 0x1a1b, 0x0120, 0x1a3b, 0x1f1e,  # 2
            0x0e07, 0x1f36, 0x3911, 0x2820, 0x3313, 0x2222, 0x222a, 0x3919, 0x1e3e, 0x0505, 0x3636, 0x1b3b, 0x2d2d, 0x3616, 0x2d0d, 0x3333],  # 3
           # mode0: red, mode2: attack+random, mode4: red1
           [0x0000, 0x3a1b, 0x3f02, 0x0e2d, 0x0e2f, 0x020f, 0x0005, 0x0d25, 0x083f, 0x0b37, 0x0715, 0x041d, 0x0105, 0x0714, 0x3532, 0x3322,  # 0
            0x0103, 0x0713, 0x3d08, 0x041b, 0x312a, 0x373a, 0x3b18, 0x3810, 0x312c, 0x373c, 0x093b, 0x051b, 0x3e00, 0x0107, 0x3b1c, 0x3814,  # 1
            0x0308, 0x3c0f, 0x030c, 0x051c, 0x3a1f, 0x0838, 0x0004, 0x0b30, 0x0614, 0x3222, 0x0615, 0x3432, 0x3f07, 0x0d20, 0x3223, 0x3433,  # 2
            0x3324, 0x302c, 0x0003, 0x030b, 0x3817, 0x3b1f, 0x3224, 0x0938, 0x3913, 0x0b34, 0x3e07, 0x3227, 0x0104, 0x3323, 0x0c20, 0x3533],  # 3
           # mode0: blue, mode2: release+hold, mode4: blue1
           [0x0000, 0x3108, 0x1108, 0x0f07, 0x373b, 0x3a2f, 0x0b20, 0x0630, 0x0212, 0x0b25, 0x3e0d, 0x373a, 0x0b24, 0x2213, 0x2935, 0x1c1c,  # 0
            0x2426, 0x110f, 0x2007, 0x1838, 0x152f, 0x2006, 0x152e, 0x1c19, 0x3a2d, 0x0f04, 0x1528, 0x183c, 0x2930, 0x3318, 0x0210, 0x0b27,  # 1
            0x0937, 0x131f, 0x1e09, 0x2b20, 0x2636, 0x310e, 0x173e, 0x3839, 0x2217, 0x1c18, 0x3e09, 0x2931, 0x1a28, 0x0d10, 0x0006, 0x352f,  # 2
            0x331e, 0x3a29, 0x2422, 0x2d15, 0x2f05, 0x2632, 0x331a, 0x310a, 0x383f, 0x2f07, 0x1a2c, 0x1738, 0x173a, 0x0002, 0x0d14, 0x352b],  # 3
           # mode0: attack+random, mode2: none, mode4: green2
           [0x0000, 0x2037, 0x2E38, 0x3320, 0x3C18, 0x1E23, 0x0D08, 0x380D, 0x0D05, 0x0628, 0x2B2B, 0x2006, 0x1A36, 0x1F14, 0x3D1E, 0x0C03,  # 0
            0x2C39, 0x1D24, 0x3D13, 0x1609, 0x1010, 0x210D, 0x0C0E, 0x0723, 0x261F, 0x1702, 0x230C, 0x0137, 0x363E, 0x150E, 0x3539, 0x3E14,  # 1
            0x0B2D, 0x281D, 0x321A, 0x0307, 0x1900, 0x0F35, 0x3937, 0x0418, 0x082A, 0x1B3D, 0x3C15, 0x2A20, 0x2330, 0x3505, 0x2F02, 0x1E1F,  # 2
            0x3A0C, 0x3121, 0x3B07, 0x302A, 0x0513, 0x0E3E, 0x2D32, 0x180B, 0x2B1A, 0x3D2F, 0x340E, 0x1635, 0x2E09, 0x383C, 0x223B, 0x0921], # 3
           # mode0: release+hold, mode2: none, mode4: red2
           [0x0000, 0x0506, 0x3f19, 0x2420, 0x1532, 0x0e39, 0x090f, 0x2b33, 0x0b14, 0x0407, 0x2e1e, 0x210d, 0x3038, 0x0534, 0x3a34, 0x2412,  # 0
            0x230f, 0x3d29, 0x382f, 0x323a, 0x1236, 0x0c10, 0x2609, 0x291a, 0x0101, 0x1f27, 0x1006, 0x0b0d, 0x373c, 0x012a, 0x042c, 0x0b3f,  # 1
            0x0f13, 0x3905, 0x2d36, 0x3310, 0x2723, 0x3309, 0x2225, 0x3c1a, 0x3c03, 0x1d25, 0x1729, 0x0303, 0x3616, 0x223c, 0x360f, 0x2829,  # 2
            0x3725, 0x3836, 0x1a38, 0x152b, 0x1107, 0x1e14, 0x0e12, 0x0a3e, 0x0a15, 0x1e3f, 0x0f21, 0x142a, 0x1b12, 0x0f38, 0x1b0b, 0x111e],  # 3
           # mode0: group, mode2: group??, mode4: blue2
           [0x0000, 0x2F16, 0x0920, 0x033B, 0x273A, 0x2D08, 0x3A03, 0x1214, 0x2223, 0x330C, 0x1D3F, 0x0C10, 0x1826, 0x2F3F, 0x060B, 0x0312,  # 0
            0x3424, 0x313D, 0x1E2B, 0x2012, 0x3018, 0x3501, 0x1B32, 0x0A1D, 0x1C1A, 0x1903, 0x0805, 0x0237, 0x0F04, 0x3C27, 0x1331, 0x021E,  # 1
            0x112F, 0x220C, 0x192C, 0x1C35, 0x2715, 0x3C21, 0x0803, 0x2D0E, 0x0D1A, 0x2137, 0x3F1A, 0x242E, 0x3323, 0x2817, 0x1337, 0x162E,  # 2
            0x2F10, 0x3E3F, 0x1601, 0x072E, 0x141F, 0x0530, 0x0D35, 0x1E04, 0x3E39, 0x250D, 0x1106, 0x1B34, 0x2A26, 0x3112, 0x0A32, 0x340B]  # 3
            ]



def setCRC1(message,crc):
    crc ^= INIT
    for index in range(7):
        value = message[(index + 2)] & 0x3f;
        crc ^= table[index][value];

    message[1] = (crc>>8) & 0x3f;
    message[9] = crc & 0x3f;
    return crc;

def main(args):
    #process

    def showSetList():
        #generate walking on and off bit
        for bit in range(41,-1,-1):
            number = (1 << bit)
            mode   = (number >> 36) & 0x0
            green  = (number >> 30) & 0x3f
            red    = (number >> 24) & 0x3f
            blue   = (number >> 18) & 0x3f
            attRan = (number >> 12) & 0x3f
            relHol = (number >> 6) & 0x3f
            group  = (number >> 0) & 0x3f
            line = [ True, 0, mode, green,red,blue,attRan,relHol,group, 0]
            setCRC1(line,0x0000)
            crc = (line[1]<<8)+line[9]
            crcX = crc ^ 0x3202
            crcY = ((crcX >> 8) << 6) + (crcX & 0x3f)
            print("%s" % f'{line[2]:06b}' + 
    #              "%s" % f'{line[2]:06b}' +
                  "%s" % f'{line[3]:06b}' +
                  "%s" % f'{line[4]:06b}' +
                  "%s" % f'{line[5]:06b}' +
                  "%s" % f'{line[6]:06b}' +
                  "%s" % f'{line[7]:06b}' +
                  "%s" % f'{line[8]:06b}' +
    #              "%s" % f'{line[9]:06b}' +
                  " = " +
                  " %s " % f'{line[1]:06b}' +
                  " %s " % f'{line[9]:06b}' +
                  " = " +
                  " %s " % f'{crc:04x}' +
    #              " %s " % f'{crcX:04x}' +
    #              " %s " % f'{crcY:012b}' +
                  " ")

    def showResetList():
        for bit in range(41,-1,-1):
            number = 0x3FFFFFFFFFF ^ (1 << bit)
            mode   = (number >> 36) & 0x0
            green  = (number >> 30) & 0x3f
            red    = (number >> 24) & 0x3f
            blue   = (number >> 18) & 0x3f
            attRan = (number >> 12) & 0x3f
            relHol = (number >> 6) & 0x3f
            group  = (number >> 0) & 0x3f
            line = [ True, 0, mode, green,red,blue,attRan,relHol,group, 0]
            setCRC1(line,0x0000)
            crc = (line[1]<<8)+line[9]
            crcX = crc ^ 0x3202
            crcY = ((crcX >> 8) << 6) + (crcX & 0x3f)
            print("%s" % f'{line[2]:06b}' + 
    #              "%s" % f'{line[2]:06b}' +
                  "%s" % f'{line[3]:06b}' +
                  "%s" % f'{line[4]:06b}' +
                  "%s" % f'{line[5]:06b}' +
                  "%s" % f'{line[6]:06b}' +
                  "%s" % f'{line[7]:06b}' +
                  "%s" % f'{line[8]:06b}' +
    #              "%s" % f'{line[9]:06b}' +
                  " = " +
                  " %s " % f'{line[1]:06b}' +
                  " %s " % f'{line[9]:06b}' +
                  " = " +
                  " %s " % f'{crc:04x}' +
    #              " %s " % f'{crcX:04x}' +
    #              " %s " % f'{crcY:012b}' +
                  " ")

    showSetList()
    showResetList()

    
    def ror12(value,bits):
        helper =  value >> bits
        helper += (value << (12-bits)) & 0xfff
        return helper

    def rol12(value,bits):
        helper =  (value << bits) & 0xfff
        helper += value >> (12-bits)
        return helper


    print("XOR with neigbour")

    for step in range(1,62):
        print(step)
        for tab in range(1,2):       #startList, endlist+1
            for index in range(1,(64-step)):
                neighbor = table[tab][index+step] ^ table[tab][(index)]
                
                for i in range(64):
                    value = table[tab][i]
                    if(value == neighbor):
                        print("[0x%s] " % f'{(index):02x}' + " ^ [0x%s] " % f'{(index+step):02x}' + "= [0x%s]" % f'{(i):02x}')

    print("done")
 
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
