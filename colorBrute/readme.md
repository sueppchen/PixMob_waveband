structure in all *.list files:
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

confirmValues.py
+ take the raw bruteForce Data and send it back to arduino, check if all is right, the LED lights up and if true, put the line into the *.list file
or if not test the crc before or 2 before (sometimes pixmob lights up late and the value is wrong)

crc_variants.py:
+ this script is able to calculate crc-checksums vith variable parameters

processData.py:
+ read the *.list file and do ... what ever you want... sort it, pick special combinations, find e structure 

