 ADVNACED:
   + pinmapping like basic - plus extra
     - Arduino GND--> soic8 8 GND    advanced only
     - Arduino  0 --> soic8 5 RED    advanced only
     - Arduino  1 --> soic8 7 GREEN  advanced only
     - Arduino  2 --> soic8 2 SDA    advanced only
     - Arduino  3 --> soic8 3 SCL    advanced only
     - Arduino  8 --> soic8 6 BLUE   advanced only
     - Arduino 5V --> D1 VCC the + labeled pad on backside. if you input power to soic8 Pin1 the LEDs will not light up.
  
   + the "advanced" arduino script is able to bruteforce the crc12 values for given 7 databytes
   + send custom hex messages or plain values
   + if you only want to brute force some CRCs, unsolder R7, connect arduino 7 directly to soic8 4 and comment out #define TRANSMITER
   + the checksum is a bit tricky, there are 12 bit of checksum and there seems to be no simple rule(bitwise Xor, bytewise Xor, crc12, cut crc16, shifted crc16) to build it...BUT:
     - some combinations give the same checksum --> yeah: brute force until the result is equal see crcBrute.py
     - some combinations give NO checksum --> someone told me to have a look at the things you DON'T see... 
     - I got nearly all values from minimum 00,04 to 3f,3f with small gaps --> 00,00 seems to be forbidden

I started a little bruteforce-session ... the results are in the folder ../colorBrute/
 + a single pixmob-arduino core will take about 512 days to brute all 262144 RGB-checksums - much to long
 + so ... lets speed things up:
 + first idea use more in parallel
   - i only have 3 pi zero W (plus 1 pi zero 2 ) and also only 4 arduino pro micro as 3.3V variant and no more level shifters.
   - arduinoIDE is not running on PIzeroW --> I use GTKterm this is not that comfortable. (select all does only select all visible not all in buffer!)
   - so i only can use 5 pixmobs in parallel each zeroW one and the zero2 two.
   - i have to stitch the files per hand
   - I will automate this a bit... next time

MULTI FRONTEND:
 + this is made for automatic bruteforcing with more clients. the idea is that one pi zero can handle all traffic and store all forced data.
 + flow:
   - pi requests all serial interfaces with magiccharacter to get the number of connected endpoints  
   - pi sends color-data to the free enpoint
   - the endpoint forces the crc
   - color and crc are reported back to pi
   - pi stores the values in a file and marks the endpoint as idle again
   - repeat
 + each arduino can handle 2 endpoints without additional hardware or 8 endpoints with mcp23017 port extender

readList.py:
+ process the brute force data in colorBrute folder
+ verify the values
  - all my values seem to be +1 in CRC
  - there are no CRC values with ( even crcA and a crcB = 0x05)  and ( odd crcA and crcB = 0x3f) 

