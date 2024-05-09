/*!
 *  @file pixmob_cement.h
 *
 * 	driver for the PixMob "cement V1.1" crowd-pixel
 *  
 * 	BSD license (see license.txt)
 */

#ifndef _PIXMOB_CEMENT_H
#define _PIXMOB_CEMENT_H

#include "Arduino.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>

#define START_DELAY    800                       ///< wait until HF is present ... value in us
#define BIT_TIME       500                       ///< inter bit delay ... us
#define PREAMBLE      0x55                       ///< preamble ... send twice
#define SYNC1            0                       ///< resync after preamble 
#define SYNC2            1                       ///< resync after preamble
#define TX_FREQ     868.49                       ///< transmit frequency for PixMob device in EU

#define POLYR 0x8f3                              ///< reversed CRC12 polynom from 0xCF1     
#define INITR 0xc69                              ///< reversed CRC12 initial value from 0x963

class Pixmob{
  public:
    Pixmob();
    bool begin(int pin);
    void reSend();
    void generateTXbuffer(uint8_t * message);
    uint8_t * basicColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t attack, uint8_t hold, uint8_t release, uint8_t random, uint8_t group);
  private:
    const uint8_t dictTable[64] = {
        0x21, 0x35, 0x2c, 0x34, 0x66, 0x26, 0xac, 0x24, 0x46, 0x56, 0x44, 0x54, 0x64, 0x6d, 0x4c, 0x6c,     ///< 6b-8b conversion table for 0x00 - 0x0f
        0x92, 0xb2, 0xa6, 0xa2, 0xb4, 0x94, 0x86, 0x96, 0x42, 0x62, 0x2a, 0x6a, 0xb6, 0x36, 0x22, 0x32,     ///< 6b-8b conversion table for 0x10 - 0x1f
        0x31, 0xB1, 0x95, 0xB5, 0x91, 0x99, 0x85, 0x89, 0xa5, 0xa4, 0x8c, 0x84, 0xa1, 0xa9, 0x8d, 0xad,     ///< 6b-8b conversion table for 0x20 - 0x2f
        0x9a, 0x8a, 0x5a, 0x4a, 0x49, 0x59, 0x52, 0x51, 0x25, 0x2d, 0x69, 0x29, 0x4D, 0x45, 0x61, 0x65      ///< 6b-8b conversion table for 0x30 - 0x3f
        };
    int _pin;
    int _chipSelect;
    uint8_t TXbuffer[9];
    uint8_t lineCode(uint8_t inByte);
    void transmitBit(bool txBit);
    void transmitByte(uint8_t txByte);
    void setCRC();
  };


#endif
