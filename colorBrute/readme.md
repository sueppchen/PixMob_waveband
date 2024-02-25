structure in all files:
- LC: XX 21 GG RR BB 46 A6 21 YY P: xx 00 gg rr bb 08 12 00 yy
  + XX = CRC1 value  in linecode --> xx = CRC1  value plain
  + YY = CRC2 value  in linecode --> yy = CRC2  value plain
  + GG = green value in linecode --> gg = green value plain
  + RR = red   value in linecode --> rr = red   value plain
  + BB = blue  value in linecode --> bb = blue  value plain
  + all tests where made with an mode 0, attack of 1, random 0, release 2, hold 2, group 0

- linecode is 6b8b big endian 
                0x21, 0x35, 0x2c, 0x34, 0x66, 0x26, 0xac, 0x24,    # // 00-07
                0x46, 0x56, 0x44, 0x54, 0x64, 0x6d, 0x4c, 0x6c,    # // 08-0f
                0x92, 0xb2, 0xa6, 0xa2, 0xb4, 0x94, 0x86, 0x96,    # // 10-17
                0x42, 0x62, 0x2a, 0x6a, 0xb6, 0x36, 0x22, 0x32,    # // 18-1f
                0x31, 0xB1, 0x95, 0xB5, 0x91, 0x99, 0x85, 0x89,    # // 20-27
                0xa5, 0xa4, 0x8c, 0x84, 0xa1, 0xa9, 0x8d, 0xad,    # // 28-2f
                0x9a, 0x8a, 0x5a, 0x4a, 0x49, 0x59, 0x52, 0x51,    # // 30-37
                0x25, 0x2d, 0x69, 0x29, 0x4D, 0x45, 0x61, 0x65     # // 38-3f

- plain value is 0x00 - 0x3f
- all values are hex values and seperated by whitespaces
- when crc could not be calculated (or the LED does not lite up) there is "CRC not found"
  
structure in folder
- filenames are blue_[hex value of blue].txt
- every file contains 4096 lines