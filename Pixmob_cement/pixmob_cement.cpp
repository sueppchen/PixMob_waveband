
/*!
 *  @file pixmob_cement.cpp
 *
 *  @mainpage driver for the PixMob "cement V1.1" crowd-pixel
 *
 *  @section intro_sec Introduction
 *
 * 	driver for the PixMob "cement V1.1" crowd-pixel
 * 
 *  @section dependencies Dependencies
 *
 *  This library depends on the SmartRC-CC1101-Driver-Lib
 *
 *  @section author Author
 *
 *  Sueppchen and Serge-45
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v0.2 - first update
 */

#include "Arduino.h"

#include <pixmob_cement.h>

/*!
 *    @brief  Instantiates a new PixMob class
 */
Pixmob::Pixmob(){}

/*!
  @brief  Sets up the hardware and initializes output
  @param  pin
          output Pin to be used.
  @return true on success , false on error 
 */
bool Pixmob::begin(int pin){

  #if defined __AVR_ATmega168__ || defined __AVR_ATmega328P__
    ELECHOUSE_cc1101.setSpiPin(13, 12, 11, 10);
  #elif defined __AVR_ATmega1280__ || defined __AVR_ATmega2560__
    ELECHOUSE_cc1101.setSpiPin(52, 50, 51, 53);
  #elif __AVR_ATmega32U4__
    ELECHOUSE_cc1101.setSpiPin(15, 14, 16, 10);
  #elif ESP8266
    ELECHOUSE_cc1101.setSpiPin(14, 12, 13, 15);
  #elif ESP32
    ELECHOUSE_cc1101.setSpiPin(18, 19, 23, 5);
  #else
    ELECHOUSE_cc1101.setSpiPin(13, 12, 11, 10);
  #endif

  _pin = pin;
  
  /// set global values
  confirmRed    = CONFIRM_RED;
  confirmGreen  = CONFIRM_GREEN;
  confirmBlue   = CONFIRM_BLUE;

  globalAttack  = DEFAULT_ATTACK;
  globalHold    = DEFAULT_HOLD;
  globalRelease = DEFAULT_RELEASE;
  globalRandom  = DEFAULT_RANDOM;
  
  if(ELECHOUSE_cc1101.getCC1101()){                                       /// check if module is present
      ELECHOUSE_cc1101.Init();                                            ///  must be set to initialize the cc1101!
      ELECHOUSE_cc1101.setGDO0(_pin);                                     ///  set lib internal gdo pin (gdo0). Gdo2 not used here.
      pinMode(_pin, OUTPUT);                                              ///  switch gdo0 to output - the library does not provide that.
      ELECHOUSE_cc1101.setModulation(2);                                  ///  set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
      ELECHOUSE_cc1101.setCCMode(0);                                      ///  set 1 config for internal FiFo mode. 0 for Assync external Mode
      ELECHOUSE_cc1101.setMHZ(TX_FREQ);                                   ///  Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
      ELECHOUSE_cc1101.setChannel(0);                                     ///  Set the Channelnumber from 0 to 255. Default is cahnnel 0.
      return 1;
  }
  else{
      return 0;
  }
      
}

/*! *************************************************************************/
uint8_t Pixmob::lineCode(uint8_t inByte){                                 /// convert plain values into 6b8b line code
    inByte &= 0x3f;
    return dictTable[inByte];
}

/*! *************************************************************************/
void Pixmob::transmitBit(bool txBit){                                     /// transmit single bit
    digitalWrite(_pin,txBit);
    delayMicroseconds(BIT_TIME);
}

/*! *************************************************************************/
void Pixmob::transmitByte(uint8_t txByte){                                /// transmit byte
    for(uint8_t bitCounter = 0; bitCounter < 8; bitCounter ++) {
        bool bit = (((txByte) >> (bitCounter)) & 1);
        transmitBit( bit );
      }
}

/*!
  @brief  refresh the batch - to be done more than every 120ms
 */
void Pixmob::refresh(){                                                   /// transmit one frame
    ELECHOUSE_cc1101.SetTx();                                             /// enable transmitter
    delayMicroseconds(START_DELAY);                                       /// wait until hf is present
    
    /// send preamble
    transmitByte(PREAMBLE);                                               /// send preamble
    transmitByte(PREAMBLE);                                               /// ... twice
    
    /// resync
    transmitBit(SYNC1);
    transmitBit(SYNC2);
    
    /// send payload: crc1, mode, b1, b2, b3, b4, b5, b6, crc2
    for(uint8_t byteCounter = 0; byteCounter < 9; byteCounter ++ )
      transmitByte(TXbuffer[byteCounter] );
    
    /// switch off everything
    ELECHOUSE_cc1101.SetRx() ;                                            /// disable transmitter
    digitalWrite(_pin, 0);                                                /// and set the output to zero
    }

/*! *************************************************************************/
void Pixmob::setCRC() {                                                  /// set calculated CRC in the given message
    // read data from first byte to last byte
    // read data bytes starting from lsb bit (bit 0)
    // register and POLY are reversed, bit 0 is the most significant
    
    uint8_t *ptr = TXbuffer;   // pointer to the 1st message byte
    *ptr++;                    // start with second byte (1st is CRC)
    uint16_t reg = INITR;      // 12 bits of the INIT (reversed) value

    // 7*8 bits to process
    for (uint8_t i = 0; i < 7; i ++) {
      reg ^= (*ptr++);          // get next byte of data
      for (uint8_t count_bits = 8; count_bits != 0; count_bits--) {
        if (reg & 0x0001) {     // if most significant bit is 1
          reg >>= 1;            // shift the register
          reg ^= POLYR;         // and xor with polynom
        } else {                // if most significant bit is 0
          reg >>= 1;            // just shift the register
        }
      }
    }

    TXbuffer[0] = lineCode( (reg & 0x3f) );
    TXbuffer[8] = lineCode( (reg >> 6) );
  }

/*! *************************************************************************/
void Pixmob::generateTXbuffer(uint8_t * message){                           /// generate TX buffer from given message
    for( uint8_t i = 0; i < 7; i ++){
      TXbuffer[(i+1)] = lineCode( message[i] );
    }
    setCRC();
}

/*!
  @brief  set global FX timing
  @param  attack
          attack value 0 - 7.
  @param  hold
          hold value 0 - 7 (0 = background).
  @param  release
          release value 0 - 7 (7 = forever).
  @param  random
          random value 0 - 7 (0 = strait, 7 = extrem).
 */
void Pixmob::setFXtiming(uint8_t attack, uint8_t hold, uint8_t release, uint8_t random){
    globalAttack  = attack  & 0x7;
    globalHold    = hold    & 0x7;
    globalRelease = release & 0x7;
    globalRandom  = random  & 0x7;
}

/*!
  @brief  set the confirm color
  @param  red
          red value 0 - 255.
  @param  green
          green value 0 - 255.
  @param  blue
          blue value 0 - 255.
 */
void Pixmob::setConfirmColor(uint8_t red, uint8_t green, uint8_t blue){
  confirmRed   = red;
  confirmGreen = green;
  confirmBlue  = blue;
}

/// send Color RX
    /*! *************************************************************************/
    void Pixmob::rxSend(uint8_t mode, uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
        uint8_t message[7];
        message[0] = mode;                     
        message[1] = green >> 2;
        message[2] = red   >> 2;
        message[3] = blue  >> 2;
        message[4] = ((globalAttack) << 3) + (globalRandom);
        message[5] = ((globalRelease) << 3) + (globalHold);
        message[6] = group & 0x1f;
        generateTXbuffer(message);
        refresh();
        } 

    /*!
      @brief  send basic RGB FX color to batch
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  group
              group value 0 - 31 (0 = all).
     */
    void Pixmob::sendColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
      uint8_t mode = MODE_RX;
      rxSend( mode, red, green, blue, group);
    }
    /*!
      @brief  send basic RGB FX color to batch
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
     */
    void Pixmob::sendColor(uint8_t red, uint8_t green, uint8_t blue){
      uint8_t mode = MODE_RX;
      rxSend( mode, red, green, blue, 0);
    }

    /*!
      @brief  send basic RGB FX color to batch and show once
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  group
              group value 0 - 31 (0 = all).
     */
    void Pixmob::sendColorOnce(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
      uint8_t mode = MODE_RX | (1 << MODE_ONESHOT);
      rxSend( mode, red, green, blue, group);
    }
    /*!
      @brief  send basic RGB FX color to batch and show once
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
     */
    void Pixmob::sendColorOnce(uint8_t red, uint8_t green, uint8_t blue){
      uint8_t mode = MODE_RX | (1 << MODE_ONESHOT);
      rxSend( mode, red, green, blue, 0);
    }

    /*!
      @brief  send basic RGB FX color to batch and play forever
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  group
              group value 0 - 31 (0 = all).
     */
    void Pixmob::sendColorForever(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
      uint8_t mode = MODE_RX | (1 << MODE_ONESHOT)| (1 << MODE_FOREVER);
      rxSend( mode, red, green, blue, group);
    }
    /*!
      @brief  send basic RGB FX color to batch and play forever
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
     */
    void Pixmob::sendColorForever(uint8_t red, uint8_t green, uint8_t blue){
      uint8_t mode = MODE_RX | (1 << MODE_ONESHOT)| (1 << MODE_FOREVER);
      rxSend( mode, red, green, blue, 0);
    }


/// memory write
    /*! *************************************************************************/
    void Pixmob::batchWrite(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t submode, uint8_t group){
        uint8_t message[7];
        message[0] = MODE_WRITE;                     
        message[1] = b1 & 0x3f;
        message[2] = b2   & 0x3f;
        message[3] = b3  & 0x3f;
        message[4] = b4  & 0x3f;
        message[5] = submode  & 0x3f;
        message[6] = group & 0x1f;
        generateTXbuffer(message);
        refresh();
    }


    /*!
      @brief  set the background color
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
     */
    void Pixmob::setBackground(uint8_t red, uint8_t green, uint8_t blue){
        batchWrite( green >> 2, red >> 2, blue >> 2, SET_BG, SUBMODE_EEWRITE, 0);
    }
    /*!
      @brief  set the background color
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  group
              group value 0 - 31 (0 = all).
     */
    void Pixmob::setBackground(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
        batchWrite( green >> 2, red >> 2, blue >> 2, SET_BG, SUBMODE_EEWRITE, group);
    }

    /*!
      @brief  silent set the background color (while FX is playing)
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
     */
    void Pixmob::setBackgroundSilent(uint8_t red, uint8_t green, uint8_t blue){
        batchWrite( green >> 2, red >> 2, blue >> 2, SET_BG_SILENT, SUBMODE_EEWRITE, 0);
    }
    /*!
      @brief  silent set the background color (while FX is playing)
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  group
              group value 0 - 31 (0 = all).
     */
    void Pixmob::setBackgroundSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t group){
        batchWrite( green >> 2, red >> 2, blue >> 2, SET_BG_SILENT, SUBMODE_EEWRITE, group);
    }

    /*!
      @brief  store Color to eeprom + show color
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  mem
              memory bank 0 - 15.
     */
    void Pixmob::storeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem){
        uint8_t location =  STORE_COLOR | (mem & 0xf);
        batchWrite( green >> 2, red >> 2, blue >> 2, location, SUBMODE_EEWRITE, 0);
    }
    /*!
      @brief  store Color to eeprom + show color
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  mem
              memory bank 0 - 15.
      @param  group
              group value 0 - 31 (0 = all).
     */
    void Pixmob::storeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem, uint8_t group){
        uint8_t location =  STORE_COLOR | (mem & 0xf);
        batchWrite( green >> 2, red >> 2, blue >> 2, location, SUBMODE_EEWRITE, group);
    }

    /*!
      @brief  silent store Color to eeprom (while FX is playing)
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  mem
              memory bank 0 - 15.
      @param  group
              group value 0 - 31 (0 = all).
     */
    void Pixmob::storeColorSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem){
        uint8_t location =  STORE_COLOR_SILENT | (mem & 0xf);
        batchWrite( green >> 2, red >> 2, blue >> 2, location, SUBMODE_EEWRITE, 0);
    }
    /*!
      @brief  silent store Color to eeprom (while FX is playing)
      @param  red
              red value 0 - 255.
      @param  green
              green value 0 - 255.
      @param  blue
              blue value 0 - 255.
      @param  mem
              memory bank 0 - 15.
      @param  group
              group value 0 - 31 (0 = all).
     */
    void Pixmob::storeColorSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem, uint8_t group){
        uint8_t location =  STORE_COLOR_SILENT | (mem & 0xf);
        batchWrite( green >> 2, red >> 2, blue >> 2, location, SUBMODE_EEWRITE, group);
    }

    /*!
      @brief  store group to group register + confirm 
      @param  location
              group register 0 - 7.
      @param  value
              group value to store 0 - 31.
      @param  group
              group 0 - 31 (0 = all).
     */
    void Pixmob::storeGroup(uint8_t location, uint8_t value, uint8_t group){
        uint8_t b1 = (confirmGreen >> 4) | (((confirmRed >> 4) & 0x3) << 4);         /// rrGGGG
        uint8_t b2 = ((confirmBlue >> 4) << 2) | (confirmRed >> 6) ;                 /// BBBBRR
        batchWrite( b1, b2, (location & 0x7), (value & 0x1f), SUBMODE_GR, group);
    }

    /*!
      @brief  set master group register + confirm 
      @param  masterGroup
              master group register 0 - 7.
      @param  group
              group 0 - 31 (0 = all).
     */
    void Pixmob::setMasterGroup(uint8_t masterGroup, uint8_t group){
        uint8_t b1 = (confirmGreen >> 4) | (((confirmRed >> 4) & 0x3) << 4);         /// rrGGGG
        uint8_t b2 = ((confirmBlue >> 4) << 2) | (confirmRed >> 6) ;                 /// BBBBRR
        batchWrite( b1, b2, (masterGroup & 0x7), 0 , SUBMODE_MGR, group);
    }

    /*!
    #define SUBMODE_MHOLD       0x07
    #define SUBMODE_MMEM        0x08
    #define SUBMODE_MMRESET     0x0F
    */

/// memory play
    /*! *************************************************************************/
    void Pixmob::playSend(uint8_t mode, uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group){
        uint8_t message[7];
        message[0] = mode;                     
        message[1] = (from & 0xf) | ((to & 0x3) << 4);                              /// end 1..0, start 3..0
        message[2] = (globalAttack << 3) | (mRandom & 0x4) | ((to & 0xc) >> 2);     /// attack 2..0, mRandom 2, end 3..2
        message[3] = ((globalRelease) << 3) + (globalHold);                         /// release 2..0, hold 2..0
        message[4] = (globalRandom);                                                /// ???, random 2..0
        message[5] = 0;                                                             /// ??????
        message[6] = (group & 0x1f) | ((mRandom & 0x1) << 5);                       /// mRandom 0, group 5..0
        generateTXbuffer(message);
        refresh();
    }

    /*!
      @brief  play from memory 
      @param  from
              start bank 0 - 15.
      @param  to
              end bank 0 - 15.
      @param  mRandom
              dice the next bank 0 - 7.
     */
    void Pixmob::playMem(uint8_t from, uint8_t to, uint8_t mRandom){
      uint8_t mode = (1 << MODE_MEM);
      playSend( mode , from, to, mRandom, 0);
    }
    /*!
      @brief  play from memory 
      @param  from
              start bank 0 - 15.
      @param  to
              end bank 0 - 15.
      @param  mRandom
              dice the next bank 0 - 7.
      @param  group
              group 0 - 31 (0 = all).
     */
    void Pixmob::playMem(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group){
      uint8_t mode = (1 << MODE_MEM);
      playSend( mode , from, to, mRandom, group);
    }

    /*!
      @brief  onshot play from memory 
      @param  from
              start bank 0 - 15.
      @param  to
              end bank 0 - 15.
      @param  mRandom
              dice the next bank 0 - 7.
     */
    void Pixmob::playMemOnce(uint8_t from, uint8_t to, uint8_t mRandom){
      uint8_t mode = (1 << MODE_MEM) | (1 << MODE_ONESHOT);
      playSend( mode , from, to, mRandom, 0);
    }
    /*!
      @brief  onshot play from memory 
      @param  from
              start bank 0 - 15.
      @param  to
              end bank 0 - 15.
      @param  mRandom
              dice the next bank 0 - 7.
      @param  group
              group 0 - 31 (0 = all).
     */
    void Pixmob::playMemOnce(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group){
      uint8_t mode = (1 << MODE_MEM) | (1 << MODE_ONESHOT);
      playSend( mode , from, to, mRandom, group);
    }

    /*!
      @brief  play from memory and loop forever 
      @param  from
              start bank 0 - 15.
      @param  to
              end bank 0 - 15.
      @param  mRandom
              dice the next bank 0 - 7.
     */
    void Pixmob::playMemForever(uint8_t from, uint8_t to, uint8_t mRandom){
      uint8_t mode = (1 << MODE_MEM) | (1 << MODE_ONESHOT) | (1 << MODE_FOREVER);
      playSend( mode , from, to, mRandom, 0);
    }
    /*!
      @brief  play from memory and loop forever 
      @param  from
              start bank 0 - 15.
      @param  to
              end bank 0 - 15.
      @param  mRandom
              dice the next bank 0 - 7.
      @param  group
              group 0 - 31 (0 = all).
     */
    void Pixmob::playMemForever(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group){
      uint8_t mode = (1 << MODE_MEM) | (1 << MODE_ONESHOT) | (1 << MODE_FOREVER);
      playSend( mode , from, to, mRandom, group);
    }

/// dual flash
    /*! *************************************************************************/
    void Pixmob::dualSend(uint8_t mode, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2){
        uint8_t message[7];
        message[0] = mode;                     
        message[1] = g1 >> 2 ;
        message[2] = r1 >> 2 ;
        message[3] = b1 >> 2 ;
        message[4] = g2 >> 2 ;
        message[5] = r2 >> 2 ; 
        message[6] = b2 >> 2 ;
        generateTXbuffer(message);
        refresh();
    }

    /*!
      @brief  dual flash in sync with transmition 
      @param  red1
              red while sending.
      @param  green1
              green while sending.
      @param  blue1
              blue while sending .
      @param  red2
              red while  not sending .
      @param  green2
              green while not sending.
      @param  blue2
              blue while not sending.
     */
    void Pixmob::flashDual(uint8_t red1, uint8_t green1, uint8_t blue1, uint8_t red2, uint8_t green2, uint8_t blue2){
        uint8_t mode = MODE_RX | ( 1 << MODE_DUAL);
        dualSend(mode, red1, green1, blue1, red2, green2, blue2);
        }

    /*!
      @brief  dual flash in sync with transmition (long) 
      @param  red1
              red while sending.
      @param  green1
              green while sending.
      @param  blue1
              blue while sending .
      @param  red2
              red while  not sending .
      @param  green2
              green while not sending.
      @param  blue2
              blue while not sending.
     */
    void Pixmob::flashDualLong(uint8_t red1, uint8_t green1, uint8_t blue1, uint8_t red2, uint8_t green2, uint8_t blue2){
        uint8_t mode = MODE_RX | ( 1 << MODE_DUAL) | ( 1 << MODE_ONESHOT);
        dualSend(mode, red1, green1, blue1, red2, green2, blue2);
        }


