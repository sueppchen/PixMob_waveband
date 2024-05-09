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
 *     v0.1 - First release
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


/* *************************************************************************/

uint8_t Pixmob::lineCode(uint8_t inByte){                                 /// convert plain values into 6b8b line code
    inByte &= 0x3f;
    return dictTable[inByte];
}

void Pixmob::transmitBit(bool txBit){                                     /// transmit single bit
    digitalWrite(_pin,txBit);
    delayMicroseconds(BIT_TIME);
}

void Pixmob::transmitByte(uint8_t txByte){                                /// transmit byte
    for(uint8_t bitCounter = 0; bitCounter < 8; bitCounter ++) {
        bool bit = (((txByte) >> (bitCounter)) & 1);
        transmitBit( bit );
      }
}

/*!
  @brief  resend the message in the buffer 
 */
void Pixmob::reSend(){                                               /// transmit one frame
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

/*!
  @brief  Sets up the hardware and initializes output
  @param  message
          mode + 6 byte as *array pointer.
 */
void Pixmob::generateTXbuffer(uint8_t * message){                           /// generate TX buffer from given message
    for( uint8_t i = 0; i < 7; i ++){
      TXbuffer[(i+1)] = lineCode( message[i] );
    }
    setCRC();
}
/*!
  @brief  create the transmit buffer from message
  @param  red
          red value 0 - 63.
  @param  green
          green value 0 - 63.
  @param  blue
          blue value 0 - 63.
  @param  attack
          attack value 0 - 63.
  @param  hold
          hold value 0 - 7 (0 = background).
  @param  release
          release value 0 - 7 (7 = forever).
  @param  random
          random value 0 - 7 (0 = strait, 7 = extrem).
  @param  group
          group value 0 - 31 (0 = all).
  @return message - array 
 */
uint8_t * Pixmob::basicColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t attack, uint8_t hold, uint8_t release, uint8_t random, uint8_t group){
    static uint8_t message[7];
    message[0] = 0x00;                     //mode
    message[1] = green & 0x3f;
    message[2] = red   & 0x3f;
    message[3] = blue  & 0x3f;
    message[4] = ((attack  & 0x7) << 3) + (random & 0x7);
    message[5] = ((release & 0x7) << 3) + (hold   & 0x7);
    message[6] = group & 0x1f;
    return message;
} 
/**************************************************************************/

