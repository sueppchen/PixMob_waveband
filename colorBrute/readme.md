structure in all files:
- "V xx 00 gg rr bb 08 12 00 yy
  + V  = 1 (valid) / 0 (invalid) 
  + xx = CRC1 value
  + yy = CRC2  value plain
  + gg = green value plain
  + rr = red   value plain
  + bb = blue  value plain
  + all tests where made with an mode 0, attack of 1, random 0, release 2, hold 2, group 0

- value is in range 0x00 - 0x3f
- all values are hex values and seperated by whitespaces
- invalid: when crc could not be calculated (or the LED does not lite up)
  
structure in folder
- filenames are blue_[number].list
- every file contains 32768 lines
