# PixMob_waveband reverse engineering

the project:
  reverse engineering pixmob RF enabled waveband (868MHz european edition)
  
  got it working thanks to https://github.com/danielweidman/pixmob-ir-reverse-engineering
  and  https://www.hackster.io/abouhatab/reusing-pixmob-waveband-without-flipper-zero-040f3a

the object:

  + got a waveband labeled "cement v1.1"
  ![all](https://github.com/sueppchen/PixMob_waveband/assets/58486836/6f24268f-cfc5-4daa-93ae-c9d2c14f122d)
  
  + U1 is the RF (maybe CMT2210LH)
  ![RF](https://github.com/sueppchen/PixMob_waveband/assets/58486836/27d4b4d4-64a9-4391-908f-28166be6646a)
  
  + witch runs on 2.7V(VR1) and is switched on and off(Q1) by scl-line
  ![transistor+vreg](https://github.com/sueppchen/PixMob_waveband/assets/58486836/2c1df685-c942-491d-9bc7-c8434d9b0ffb)
  
  + the Xtal frequency is 24.8117MHzfor europe(and 26.1522 for US variant)
  + multiplied by 35 we have the nice 868.4MHz for europe and 915,327 for US.
  
  ![XTAL+EEPROM](https://github.com/sueppchen/PixMob_waveband/assets/58486836/8d73dcba-42db-4f61-b4b3-9b6028d15b26)
  
  + beside is a small i2c-eeprom device which contents group information and maybe some predefined color effects
    - if the first byte of the eeprom is altered, the MCU reads back all eeprom-data and resets every value to factory default.
    - eeprom1.txt and eeprom2.txt are programmed devices, eeprom_reset.txt are factory defaults
    - address is 0b10100000 for reading and 0b10100001 for writing.
  
  
  + the MCU is unlabeled but it should be a dirt cheep device because the product is a "giveaway"
    ( https://cpldcpu.wordpress.com/2019/08/12/the-terrible-3-cent-mcu/)
  
  ![MCU](https://github.com/sueppchen/PixMob_waveband/assets/58486836/497c3a8c-62c1-48c1-b1f2-a0d007095368)
  
  
    so my choice would be the PFS154-S08 because it has 3 pwm outputs
  
  + MCU Pinout:
   - 1 VCC   
   - 2 sda   
   - 3 scl + /standby    
   - 4 RX
   - 5 PWM Red   
   - 6 PWM Blue   
   - 7 PWM Green   
   - 8 GND 
    
  + PWM is running at about 25kHz but has some jitter in values (+- 0.5 %)  --> maybe it is softpwm
  + LED is active low
  
  + @startup the MCU reads back few values from eeprom @adresses 00, 03, 02, 01, (if value@03 == 0x01) then read 08, else read 0b), 04

  + serial input data is 8bit big endian @500us per bit, no startbits, no stopbits coded with special 6b8b line code to stay in sync
    - each telegram starts with 0x55 0x55 for sync, then a single zero bit, a one bit, crc11:6, 7 databits, crc5:0
    - databits are: mode(unknown not tested yet), green, red, blue, attack+random, release + hold, group (not tested yet)
    - rules for 6b8b: max two 1 together, max four 0 togeter, max a single 1 @start and end, max two 0 @start and end. 0x55 and 0xaa are forbiden (sync)
    - LED values = 0 - 63 with dimmer-curve 0.5% - 94%
    - timing values are bit-coded in byte 5 and byte 6 (3 bit each)
      * attack  5:5..3 (0-7) = 0ms, 30ms, 100ms, 200ms ,500ms, 1000ms, 2000ms, 4000ms
      * random  5:2..0 (0-7) = 0%, 10%, 20%, 35%, 50%, 65%, 80%, 95%
      * release 6:5..3 (0-7) = bgcolor, 30ms, 100ms, 200ms, 500ms, 1000ms, 2000ms, 4000ms
      * hold    6:2..0 (0-7) = 0ms, 30ms, 100ms, 200ms, 500ms, 1000ms, 2500ms, infinite
  
  
what we need:
  hardware: arduino Pro micro(atMega32u4), levelshifter, TI CC1101, (soic08 clamp)
  
  BASIC:
   + basic Pin Mapping:
     - Arduino  6 --> (levelshifter) --> CC1101 gdo2 (you don't need this pin) 
     - Arduino  7 --> levelshifter --> CC1101 gdo0
     - Arduino 10 --> levelshifter --> CC1101 CS
     - Arduino 13 --> arduino internal LED
     - Arduino 14 --> (levelshifter) --> CC1101 SO
     - Arduino 15 --> levelshifter --> CC1101 SCLK
     - Arduino 16 --> levelshifter --> CC1101 SI
     - 3.3V --> CC1101 3.3V input (my levelshifter does provide 3.3V power)
     - If you only have a quad levelshifter (like me) vou don't need to shift SO and gdo2.
  
   + the "basic" arduino script shows how to enable the waveband with an TI CC1101 transceiver and fixed messages
   + there is a basic CLI over USBserial@115200: type ? [enter] for help
  
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

I started a little bruteforce-session ... the results are in the folder colorBrute/
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

