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
  
  beside is a small eeprom device which contents group information and maybe some predefined color effects
  if the first byte of the eeprom is altered, the MCU reads back all eeprom-data and resets every value to factory default.
  eeprom1.txt and eeprom2.txt are programmed devices, eeprom_reset.txt are factory defaults
  
  
  the MCU is unlabeled but it should be a dirt cheep device because the product is a "giveaway"
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
    
  + PWM is running at about 25kHz but has some jitter in values (+- 0.5 %)  --> maybe softpwm
  + LED is active low
  
  + @startup the MCU reads back few values from eeprom @adresses 00, 03, 02, 01, (if value@03 == 0x01) then read 08, else read 0b), 04

  + serial input data is 8bit big endian @500us per bit, no startbits, no stopbits coded with special 6b8b line code to stay in sync
    - each telegram starts with 0x55 0x55 for sync, then a single zero bit, a one bit, crc11:6, 7 databits, crc5:0
    - databits are: mode(unknown not tested yet), green, red, blue, attack+random, release + hold, group (not tested yet)
    - rules for 6b8b: max two 1 together, max four 0 togeter, max a single 1 @start and end, max two 0 @start and end. 0x55 and 0xaa are forbiden (sync)
    - LED values = 0-63
    - timing values are bit-coded (3 bit each)
      * attack (0-7) = 0ms, 16ms, 80ms, ? ,440ms, ?, ?, ?
      * random (0-7) = ??????,5%
      * release(0-7) = bgcolor, 16ms, 80ms, ?, 440ms, ?, ?, ?
      * hold   (0-7) = 48ms, 32ms, 115ms, 215ms, 516ms, ?, infinite
  
  
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
      
