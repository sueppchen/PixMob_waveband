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

  + data is [transmitted](https://github.com/sueppchen/PixMob_waveband/wiki/transmission) with a [checksum](https://github.com/sueppchen/PixMob_waveband/wiki/checksum) only by RF and encoded with special [6b8b line code](https://github.com/sueppchen/PixMob_waveband/wiki/6b8b-line-code) to stay in sync.

## what we need to get it running:
-
  hardware: 
   - arduino 
   - TI CC1101
   - levelshifter (if the arduino is 5V variant)
   - ( soic08 clamp: good for development - but the most hardware-work is done) 
   - connect all together (see [connection](https://github.com/sueppchen/PixMob_waveband/wiki/connection) )
   - put the library into your arduino sketchbook/libraries folder
   - now you can use the pixmob library with example(s)
    
## thanks to:
 [Daniel Weidman](https://github.com/danielweidman)
 [Serge](https://github.com/Serge-45)
 
