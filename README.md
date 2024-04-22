# PixMob_waveband reverse engineering
--

## the project:
-
  reverse engineering pixmob RF enabled waveband (868MHz european edition)
  
  got it working thanks to https://github.com/danielweidman/pixmob-ir-reverse-engineering
  and  https://www.hackster.io/abouhatab/reusing-pixmob-waveband-without-flipper-zero-040f3a

## the object:
-
  + got a waveband labeled "cement v1.1"
  ![all](https://github.com/sueppchen/PixMob_waveband/assets/58486836/6f24268f-cfc5-4daa-93ae-c9d2c14f122d)
  
  + with [MCU](https://github.com/sueppchen/PixMob_waveband/wiki/MCU) , [EEPROM](https://github.com/sueppchen/PixMob_waveband/wiki/EEprom) , [RF](https://github.com/sueppchen/PixMob_waveband/wiki/RF) , two [RGB-LEDs](https://github.com/sueppchen/PixMob_waveband/wiki/LED) and a diode to prevent battery polarity fault

  + data is [transmitted](https://github.com/sueppchen/PixMob_waveband/wiki/transmission) with a [checksum] only by RF and encoded with special [6b8b line code](https://github.com/sueppchen/PixMob_waveband/wiki/6b8b-line-code) to stay in sync.

## what we need to get it running:
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
    
