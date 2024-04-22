# PixMob_waveband reverse engineering
--

the project:
-
  reverse engineering pixmob RF enabled waveband (868MHz european edition)
  
  got it working thanks to https://github.com/danielweidman/pixmob-ir-reverse-engineering
  and  https://www.hackster.io/abouhatab/reusing-pixmob-waveband-without-flipper-zero-040f3a

the object:
-
  + got a waveband labeled "cement v1.1"
  ![all](https://github.com/sueppchen/PixMob_waveband/assets/58486836/6f24268f-cfc5-4daa-93ae-c9d2c14f122d)
  
  with [MCU](https://github.com/sueppchen/PixMob_waveband/wiki/MCU) , [EEPROM](https://github.com/sueppchen/PixMob_waveband/wiki/EEprom) , [RF](https://github.com/sueppchen/PixMob_waveband/wiki/RF)

  + serial input data is 8bit big endian @500us per bit, no startbits, no stopbits coded with special 6b8b line code to stay in sync
    - rules for 6b8b: max two 1 together, max four 0 togeter, max a single 1 @start and end, max two 0 @start and end. 0x55 and 0xaa are forbiden (sync)
    - each telegram starts with 0x55 0x55 for sync, then a single zero bit, a one bit, checksumA, byte 0 - byte 7, checksumB
    - byte 0 = mode
      * mode = 0x00: RX-mode continous as long a valid input with CRC is received  
      * mode = 0x10: like mode = 0x00 but only flash once until message is changed
      * mode = 0x11: like mode = 0x00 but the message is repeated until new valid message arrives even if there is no RX or power is cycled (values are stored in eeprom)
      * mode = 0x04: dual-flash-mode - like mode = 0x00 but byte-order is changed to flash with two colors
      * mode = 0x14: like mode = 0x04 but slower flashes
      * mode = 0x02: EEprom-read-mode - like mode = 0x00, but read values from EEprom
      * mode = 0x12: combination of mode = 0x10 and mode = 0x02
      * mode = 0x13: combination of mode = 0x11 and mode = 0x02
      * mode = 0x0f: EEprom-write-mode
    
    - LED values = 0 - 63 with dimmer-curve 0.5% - 94%
    - RX bytes:  < mode, G, R, B, attack/random (AAArrr), release/hold (RRRHHH), group >
    - dual-flash bytes: < mode, G1, R1, B1, G2, R2, B2 >
    - EEprom-read bytes: < mode, End/Start (EESSSS), attack/random/end (AAArEE), release/hold (RRRHHH), ??, ??, group (rGGGGG) >
      * byte 6.5 = random.0
      * byte x.x = random.1 (havn't found that bit yet... it is always 1 - maybe a mode-bit)
      * byte 2.2 = random.2
      * byte 1.0 = start.0
      * byte 1.1 = start.1
      * byte 1.2 = start.2
      * byte 1.3 = start.3
      * byte 1.4 = end.4
      * byte 1.5 = end.5
      * byte 2.0 = end.6
      * byte 2.1 = end.7
    - EEprom-write bytes: < mode, b1, b2, b3, b4, subMode, group >
      * subMode = 0x00: write Color to EEprom b1=green, b2=red, b3=blue, b4=memory-location(0x00 - 0x0f)
      * subMode = 0x00: set background-color: b1=green, b2=red, b3=blue, b4 > 0x10
      * subMode = 0x01: write master-group: b1=rrGGGG, b2=BBBBRR, b3=masterGroup, b4={ignored} (rr is LSB, RR is MSB)
      * subMode = 0x02: write group-register: b1=rrGGGG, b2=BBBBRR, b3=register number(0-7), b4=register Value(0-31) (rr is LSB, RR is MSB)
    - timing values are bit-coded in byte 4 and byte 5 (3 bit each)
      * attack  4:5..3 (0-7) = 0ms, 30ms, 100ms, 200ms ,500ms, 1000ms, 2000ms, 4000ms
      * random  4:2..0 (0-7) = 0%, 10%, 20%, 35%, 50%, 65%, 80%, 95%
      * release 5:5..3 (0-7) = bgcolor, 30ms, 100ms, 200ms, 500ms, 1000ms, 2000ms, 4000ms
      * hold    5:2..0 (0-7) = 0ms, 30ms, 100ms, 200ms, 500ms, 1000ms, 2500ms, infinite
    - Background timeout range is from about 0.5 second to about 60 seconds according to the HOLD value (from 0 to 7) in the message setting the background color.  

    ![timing1](https://github.com/sueppchen/PixMob_waveband/assets/58486836/7edfed66-dd12-40f3-a30b-8f9e245992b8)

    ![timing2](https://github.com/sueppchen/PixMob_waveband/assets/58486836/72125370-ecef-4c2f-8bf7-4f00ef518a4f)

    - checksum is an XOR checksum of a 12 bit subchecksum of each databyte
      * initial Value ^ 12bit checksum byte 0 ^ 12bit checksum byte 1 ... 12bit checksum byte 6
      * 6 bits of this checksum are before and 6 bit after transmition 



what we need to get it running:
-
  hardware: arduino Pro micro(atMega32u4), levelshifter, TI CC1101, (soic08 clamp)
  
  basic.ino:
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
  
  play.ino:
   + simple programm to play with pixmob batch.
   + data is send by basic CLI
     - w\r\n    mode byte 1, byte 2, byte 3, byte 4, byte 5, byte 6 \r\n (all hex, 6 bit)
     - f (interactiv)
    
