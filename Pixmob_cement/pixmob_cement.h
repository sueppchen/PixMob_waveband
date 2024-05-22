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

/// mode
#define MODE_RX             0x00
#define MODE_FOREVER           0
#define MODE_MEM               1
#define MODE_DUAL              2
#define MODE_ONESHOT           4
#define MODE_WRITE          0x0F

#define SUBMODE_EEWRITE     0x00
#define SUBMODE_MGR         0x01
#define SUBMODE_GR          0x02
#define SUBMODE_MHOLD       0x07
#define SUBMODE_MMEM        0x08
#define SUBMODE_MMRESET     0x0F

#define SET_BG              0x10
#define SET_BG_SILENT       0x30

#define STORE_COLOR         0x00
#define STORE_COLOR_SILENT  0x20

///global values
#define CONFIRM_RED         0x80
#define CONFIRM_GREEN       0x00
#define CONFIRM_BLUE        0x00

#define DEFAULT_ATTACK         1
#define DEFAULT_HOLD           2
#define DEFAULT_RELEASE        2
#define DEFAULT_RANDOM         0

class Pixmob{
  public:
    Pixmob();
    bool begin(int pin);
    void refresh();
    void setConfirmColor(uint8_t red, uint8_t green, uint8_t blue);
    void setFXtiming(uint8_t attack, uint8_t hold, uint8_t release, uint8_t random);
    void sendColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t group);
    void sendColorOnce(uint8_t red, uint8_t green, uint8_t blue, uint8_t group);
    void sendColorForever(uint8_t red, uint8_t green, uint8_t blue, uint8_t group);
    void setBackground(uint8_t red, uint8_t green, uint8_t blue, uint8_t group);
    void setBackgroundSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t group);
    void storeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem, uint8_t group);
    void storeColorSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem, uint8_t group);
    void storeGroup(uint8_t location, uint8_t value, uint8_t group);
    void setMasterGroup(uint8_t masterGroup, uint8_t group);
    void playMem(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group);
    void playMemOnce(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group);
    void playMemForever(uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group);
    void flashDual(uint8_t red1, uint8_t green1, uint8_t blue1, uint8_t red2, uint8_t green2, uint8_t blue2);
    void flashDualLong(uint8_t red1, uint8_t green1, uint8_t blue1, uint8_t red2, uint8_t green2, uint8_t blue2);
    void sendColor(uint8_t red, uint8_t green, uint8_t blue);
    void sendColorOnce(uint8_t red, uint8_t green, uint8_t blue);
    void sendColorForever(uint8_t red, uint8_t green, uint8_t blue);
    void setBackground(uint8_t red, uint8_t green, uint8_t blue);
    void setBackgroundSilent(uint8_t red, uint8_t green, uint8_t blue);
    void storeColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem);
    void storeColorSilent(uint8_t red, uint8_t green, uint8_t blue, uint8_t mem);
    void playMem(uint8_t from, uint8_t to, uint8_t mRandom);
    void playMemOnce(uint8_t from, uint8_t to, uint8_t mRandom);
    void playMemForever(uint8_t from, uint8_t to, uint8_t mRandom);
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
    uint8_t globalAttack, globalHold, globalRelease, globalRandom;
    uint8_t confirmRed, confirmGreen, confirmBlue;
    uint8_t lineCode(uint8_t inByte);
    void transmitBit(bool txBit);
    void transmitByte(uint8_t txByte);
    void setCRC();
    void generateTXbuffer(uint8_t * message);
    void rxSend(uint8_t mode, uint8_t red, uint8_t green, uint8_t blue, uint8_t group);
    void playSend(uint8_t mode, uint8_t from, uint8_t to, uint8_t mRandom, uint8_t group);
    void batchWrite(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t submode, uint8_t group);
    void dualSend(uint8_t mode, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2);
  };


#endif
