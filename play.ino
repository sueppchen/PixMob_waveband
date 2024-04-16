/*       testscript for PixMob waveband - (RF enabled crowd pixel)
 2024 sueppchen, Serge_45
 
 TX:
  arduino pro micro /ATmega32u4
  levelshifter
  cc1101 radio module
  5 test-wires, power, ground
  bk GND, rd VCC5, ye 3 SCL, bn 2 SDA, bl 0 TX, gn 1 RX, or 8
 
 NO TX:
  Arduino nano / mini Pro ATmega328
  GND, R, G, B, SCL, SDA, TX, (VCC5)
 
*/
// ------------ global ------------
#if defined(__AVR_ATmega32U4__)      //automatic select between boards
  #define TRANSMITTER
#endif

// ------------ I/O ------------

  #ifdef TRANSMITTER                              // ATmega32u4 = micro Pro
    #define CLI_SPEED 115200                                           //ATmega328 = nano, Mini Pro
    #define PINRED   0                            //normalInt
    #define PINGREEN 1
    #define PINSDA   2
    #define PINSCL   3
    #define PINBLUE  8
    #define TX 7                           // data output
    #define RADIO_GD2 6
    #define START_DELAY    800                  // wait until hf is present ... us
  #else
    #define CLI_SPEED 9600                                           //ATmega328 = nano, Mini Pro 8MHz --> save time 
    #define PINRED   2        
    #define PINGREEN 3
    #define PINBLUE  4
    #define PINSCL   5
    #define PINSDA   6
    #define TX 7                           // data output
    
    #define LED 13
    #define START_DELAY    1                  // wait until hf is present ... us
  #endif

// ------------ CLI ---------
  #define SEPARATOR    "+-------------------------------------------+"
  #define TABLE_HEADER "| PixMob CL-interface: type ? for help      |"
  #define PROMPT "> "

// ------------ TX ----------
  #define BIT_TIME       500                  // inter bit delay ... us
  #define PREAMBLE      0x55                  // preamble ... send twice
  #define SYNC1            0                  // 
  #define SYNC2            1                  // 
  
  #ifdef TRANSMITTER
    #include <ELECHOUSE_CC1101_SRC_DRV.h>       // for radio module
    #define TX_FREQ     868.49                  // transmit frequency
  #else
    uint8_t getPortD;
  #endif

uint8_t index, indexAlt;
bool rxReady, match, ledValue; 
uint8_t h; 
bool sclin = 1, sclAlt=1;

uint64_t superInput,h64;

uint8_t byteArray[4][9] = {
  // 0     1     2     3     4     5     6     7     8       
  // 0b1101010, 0b100001, 0b100001, 0b100001, 0b100001, 0b1000110, 0b1101100, 0b100001, 0b10010100
  // 6a
  {0x1B, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0F, 0x00, 0x15}, //1, black as background, active for 1 minute
  {0x37, 0x00, 0x00, 0x3f, 0x00, 0x38, 0x3E, 0x00, 0x20}, //2, red, smoooooooth fade
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //3, test-string
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //4, test-string2
  };

// ------------------ TX ---------------------- tx related stuff
  #define INIT 0x3202  // reference CRC = CRC of message "32 00 00 00 00 00 00 00 02"
  uint16_t table[7][64] = {
    { 0x0000, 0x0c2d, 0x3d30, 0x283b, 0x1e2d, 0x1b26, 0x3726, 0x130b, 
      0x0f0b, 0x343b, 0x0726, 0x3c16, 0x1600, 0x1c2d, 0x291d, 0x383b,    
      0x0c3b, 0x1d1d, 0x1130, 0x262d, 0x222d, 0x330b, 0x0016, 0x3b26, 
      0x3816, 0x2930, 0x0200, 0x070b, 0x2a00, 0x2016, 0x2c3b, 0x170b,    
      0x3b30, 0x3126, 0x171d, 0x063b, 0x2000, 0x0e3b, 0x2c2d, 0x350b, 
      0x3d0b, 0x191d, 0x2600, 0x083b, 0x0a16, 0x242d, 0x0216, 0x1330,    
      0x2200, 0x1930, 0x2d1d, 0x162d, 0x3a16, 0x0126, 0x0326, 0x2f1d, 
      0x371d, 0x1926, 0x2b30, 0x2e3b, 0x0d0b, 0x2330, 0x050b, 0x3216},   
    //mode0: green, mode2: mem(end<<4 | start ), mode4:green1,
    { 0x0000, 0x0f0f, 0x1129, 0x1b23, 0x2800, 0x050d, 0x0b32, 0x0a12,  // 00 - 07
      0x2d25, 0x3c14, 0x223a, 0x330b, 0x271f, 0x2808, 0x3901, 0x3c24,  // 08 - 0F
      0x153c, 0x1019, 0x1f16, 0x0128, 0x0138, 0x041d, 0x1a33, 0x0b02,  // 10 - 17
      0x331b, 0x363e, 0x0008, 0x2d05, 0x0e27, 0x143c, 0x1b33, 0x0a02,  // 18 - 1F
      0x1131, 0x0b2a, 0x1031, 0x1514, 0x0e0f, 0x1534, 0x0100, 0x0405,  // 20 - 27
      0x0425, 0x1009, 0x0e17, 0x152c, 0x1a1b, 0x0120, 0x1a3b, 0x1f1e,  // 28 - 2F
      0x0e07, 0x1f36, 0x3911, 0x2820, 0x3313, 0x2222, 0x222a, 0x3919,  // 30 - 37
      0x1e3e, 0x0505, 0x3636, 0x1b3b, 0x2d2d, 0x3616, 0x2d0d, 0x3333}, // 38 - 3f
    // mode0: red, mode2: attack+random, mode4: red1
    { 0x0000, 0x3a1b, 0x3f02, 0x0e2d, 0x0e2f, 0x020f, 0x0005, 0x0d25, 
      0x083f, 0x0b37, 0x0715, 0x041d, 0x0105, 0x0714, 0x3532, 0x3322,
      0x0103, 0x0713, 0x3d08, 0x041b, 0x312a, 0x373a, 0x3b18, 0x3810, 
      0x312c, 0x373c, 0x093b, 0x051b, 0x3e00, 0x0107, 0x3b1c, 0x3814,
      0x0308, 0x3c0f, 0x030c, 0x051c, 0x3a1f, 0x0838, 0x0004, 0x0b30, 
      0x0614, 0x3222, 0x0615, 0x3432, 0x3f07, 0x0d20, 0x3223, 0x3433,
      0x3324, 0x302c, 0x0003, 0x030b, 0x3817, 0x3b1f, 0x3224, 0x0938, 
      0x3913, 0x0b34, 0x3e07, 0x3227, 0x0104, 0x3323, 0x0c20, 0x3533},
    // mode0: blue, mode2: release+hold, mode4: blue1
    { 0x0000, 0x3108, 0x1108, 0x0f07, 0x373b, 0x3a2f, 0x0b20, 0x0630, 
      0x0212, 0x0b25, 0x3e0d, 0x373a, 0x0b24, 0x2213, 0x2935, 0x1c1c, 
      0x2426, 0x110f, 0x2007, 0x1838, 0x152f, 0x2006, 0x152e, 0x1c19, 
      0x3a2d, 0x0f04, 0x1528, 0x183c, 0x2930, 0x3318, 0x0210, 0x0b27, 
      0x0937, 0x131f, 0x1e09, 0x2b20, 0x2636, 0x310e, 0x173e, 0x3839, 
      0x2217, 0x1c18, 0x3e09, 0x2931, 0x1a28, 0x0d10, 0x0006, 0x352f, 
      0x331e, 0x3a29, 0x2422, 0x2d15, 0x2f05, 0x2632, 0x331a, 0x310a, 
      0x383f, 0x2f07, 0x1a2c, 0x1738, 0x173a, 0x0002, 0x0d14, 0x352b},
    // mode0: attack+random, mode2: none, mode4: green2
    { 0x0000,	0x2037,	0x2E38,	0x3320,	0x3C18,	0x1E23,	0x0D08,	0x380D,	/* Byte 5 (Attack/Random), 00-07 */
      0x0D05,	0x0628,	0x2B2B,	0x2006,	0x1A36,	0x1F14,	0x3D1E,	0x0C03,	/* Byte 5 (Attack/Random), 08-0F */
      0x2C39,	0x1D24,	0x3D13,	0x1609,	0x1010,	0x210D,	0x0C0E,	0x0723,	/* Byte 5 (Attack/Random), 10-17 */
      0x261F,	0x1702,	0x230C,	0x0137,	0x363E,	0x150E,	0x3539,	0x3E14,	/* Byte 5 (Attack/Random), 18-1F */
      0x0B2D,	0x281D,	0x321A,	0x0307,	0x1900,	0x0F35,	0x3937,	0x0418,	/* Byte 5 (Attack/Random), 20-27 */
      0x082A,	0x1B3D,	0x3C15,	0x2A20,	0x2330,	0x3505,	0x2F02,	0x1E1F,	/* Byte 5 (Attack/Random), 28-2F */
      0x3A0C,	0x3121,	0x3B07,	0x302A,	0x0513,	0x0E3E,	0x2D32,	0x180B,	/* Byte 5 (Attack/Random), 30-37 */
      0x2B1A,	0x3D2F,	0x340E,	0x1635,	0x2E09,	0x383C,	0x223B,	0x0921},	/* Byte 5 (Attack/Random), 38-3F */
    // mode0: release+hold, mode2: none, mode4: red2
    { 0x0000, 0x0506, 0x3f19, 0x2420, 0x1532, 0x0e39, 0x090f, 0x2b33, 
      0x0b14, 0x0407, 0x2e1e, 0x210d, 0x3038, 0x0534, 0x3a34, 0x2412,  
      0x230f, 0x3d29, 0x382f, 0x323a, 0x1236, 0x0c10, 0x2609, 0x291a, 
      0x0101, 0x1f27, 0x1006, 0x0b0d, 0x373c, 0x012a, 0x042c, 0x0b3f,  
      0x0f13, 0x3905, 0x2d36, 0x3310, 0x2723, 0x3309, 0x2225, 0x3c1a, 
      0x3c03, 0x1d25, 0x1729, 0x0303, 0x3616, 0x223c, 0x360f, 0x2829,  
      0x3725, 0x3836, 0x1a38, 0x152b, 0x1107, 0x1e14, 0x0e12, 0x0a3e, 
      0x0a15, 0x1e3f, 0x0f21, 0x142a, 0x1b12, 0x0f38, 0x1b0b, 0x111e},
    // mode0: group, mode2: group??, mode4: blue2
    { 0x0000,	0x2F16,	0x0920,	0x033B,	0x273A,	0x2D08,	0x3A03,	0x1214,	 /* Byte 7 (Group), 00-07 */
      0x2223,	0x330C,	0x1D3F,	0x0C10,	0x1826,	0x2F3F,	0x060B,	0x0312,	 /* Byte 7 (Group), 08-0F */
      0x3424,	0x313D,	0x1E2B,	0x2012,	0x3018,	0x3501,	0x1B32,	0x0A1D,	 /* Byte 7 (Group), 10-17 */
      0x1C1A,	0x1903,	0x0805,	0x0237,	0x0F04,	0x3C27,	0x1331,	0x021E,	 /* Byte 7 (Group), 18-1F */
      0x112F,	0x220C,	0x192C,	0x1C35,	0x2715,	0x3C21,	0x0803,	0x2D0E,	 /* Byte 7 (Group), 20-27 */
      0x0D1A,	0x2137,	0x3F1A,	0x242E,	0x3323,	0x2817,	0x1337,	0x162E,	 /* Byte 7 (Group), 28-2F */
      0x2F10,	0x3E3F,	0x1601,	0x072E,	0x141F,	0x0530,	0x0D35,	0x1E04,	 /* Byte 7 (Group), 30-37 */
      0x3E39,	0x250D,	0x1106,	0x1B34,	0x2A26,	0x3112,	0x0A32,	0x340B}, /* Byte 7 (Group), 38-3F */
      };  

  uint16_t setCRC(uint8_t *message) {          // set calculated CRC in the given message
    // return CRC1 and CRC2 values as a 16-bit word : 00aaaaaa 00bbbbbb, a is the CRC1 value, b is the CRC2 value
    uint16_t crc = INIT;
    for(byte i=0;i<7;i++){
      uint8_t value = (message[(i+1)]) & 0x3f;
      crc ^= table[i][value];
    }
    message[0] = (crc>>8) & 0x3f;
    message[8] = crc & 0x3f;
    return crc;
    }
  uint8_t dictTable[66] = {
    0x21,0x35,0x2c,0x34,0x66,0x26,0xac,0x24,     // 00-07
    0x46,0x56,0x44,0x54,0x64,0x6d,0x4c,0x6c,     // 08-0f
    0x92,0xb2,0xa6,0xa2,0xb4,0x94,0x86,0x96,     // 10-17
    0x42,0x62,0x2a,0x6a,0xb6,0x36,0x22,0x32,     // 18-1f
    0x31,0xB1,0x95,0xB5,0x91,0x99,0x85,0x89,     // 20-27
    0xa5,0xa4,0x8c,0x84,0xa1,0xa9,0x8d,0xad,     // 28-2f
    0x9a,0x8a,0x5a,0x4a,0x49,0x59,0x52,0x51,     // 30-37
    0x25,0x2d,0x69,0x29,0x4D,0x45,0x61,0x65,      // 38-3f
    0xaa,0x55};

  void initTX(){                                                    // init transmiter 
    #ifdef TRANSMITTER
      Serial.print("init TX:");
      ELECHOUSE_cc1101.Init();                        // must be set to initialize the cc1101!
      Serial.println(" done");
      ELECHOUSE_cc1101.SetTx();
      ELECHOUSE_cc1101.setGDO(TX,RADIO_GD2);   // set lib internal gdo pin (gdo0). Gdo2 not use for this example.
      ELECHOUSE_cc1101.setModulation(2);              // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
      ELECHOUSE_cc1101.setCCMode(0);                  // set 1 config for internal FiFo mode. 0 for Assync external Mode
      ELECHOUSE_cc1101.setMHZ(TX_FREQ);                // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
      ELECHOUSE_cc1101.setChannel(0);                 // Set the Channelnumber from 0 to 255. Default is cahnnel 0.
      Serial.println("Tx ready");
      pinMode(TX, OUTPUT);
    #else
      Serial.println("no TX");
    #endif
    }
  void transmitBit(bool txBit){                                     // transmit single bit
    digitalWrite(TX,txBit);
    delayMicroseconds(BIT_TIME);
    }

  void transmitByte(uint8_t txByte){                                // transmit byte
    for(int bitCounter=0; bitCounter<8; bitCounter++)  {
        bool bit=(((txByte) >> (bitCounter)) & 0x01);
        transmitBit(bit);
      }
    }

  uint8_t lineCode(uint8_t inByte){                                 // convert plain values into 6b8b line code
    inByte &= 0x7f;
    return dictTable[inByte];
    }
  
  void sendMessage(uint8_t* buffer){                                // transmit one frame
    #ifdef TRANSMITTER
      ELECHOUSE_cc1101.SetTx();                                     //enable transmitter
    #endif
    delayMicroseconds(START_DELAY);                                 //wait until hf is present
    //preamble
    transmitByte(PREAMBLE);
    transmitByte(PREAMBLE);
    //resync
    transmitBit(SYNC1);
    transmitBit(SYNC2);
    //payload: crc1, mode, green, red, blue, rise, fall, group, crc2
    for(uint8_t byteCounter=0; byteCounter < 9; byteCounter++ )
      transmitByte( lineCode( buffer[byteCounter] ) );
    #ifdef TRANSMITTER
      ELECHOUSE_cc1101.SetRx() ;                                    // disable transmitter
    #endif
    digitalWrite(TX,0);
    }
//
// ------------------ CLI --------------------- command line interface 
  uint64_t strtoul64(char const *str) {                             // simple strtoul() for 64bit
      uint64_t accumulator = 0;
      for (size_t i = 0 ; isxdigit((unsigned char)str[i]) ; ++i) {
          char c = str[i];
          accumulator *= 16;
          if (isdigit(c))       accumulator += c - '0';             // '0' .. '9'
          else if (isupper(c))  accumulator += c - 'A' + 10;        // 'A' .. 'F'
          else                  accumulator += c - 'a' + 10;        // 'a' .. 'f'
        }
      return accumulator;
    }
  void showBuffer(uint8_t* buffer){                                 // print buffer (plain + encoded)
    Serial.print("[True, ");
    for(uint8_t i = 0; i < 9; i++ ) {
      Serial.print("0x");
      if( buffer[i] < 0x10) Serial.print("0");
      Serial.print(buffer[i],HEX);
      if(i<8) Serial.print(", ");
      }
    Serial.print("]\r\n");
    }
  void clearSerial(){                                               // read and discard data on serial port
    while (Serial.available() > 0){
      Serial.read();
      }
    }

  uint8_t readCliByte(){                                            // read cli single byte
    uint8_t input=0;
    if (Serial.available() > 0){
      input = Serial.read();
      Serial.write((input >= 0x20 && input <= 0x7e) ? input : 0x20);
      Serial.println("");
      clearSerial();
      }
    return input;
    }


  uint64_t readCliHex(){                                            // read cli hex value
    char buffer[16];
    uint8_t idx = 0;

    while (true){
      if (Serial.available() > 0){
        uint8_t input = Serial.read();
        Serial.write((input >= 0x20 && input <= 0x7e) ? input : 0x20);
        if ((idx < sizeof(buffer) - 1) && (input != 0x0d) && (input != 0x0a)){   //return
          if (input >= 0x30 && input <= 0x7a)
            buffer[idx++] = input;
          }
        else{
          Serial.println("");
          clearSerial();
          buffer[idx] = 0x00;
          return strtoul64(buffer);
          }
        }
      delay(100);
      }
    }
  uint64_t readCliDez(){                                            // read 
    char buffer[5];
    uint8_t idx = 0;

    while (true){
      if (Serial.available() > 0){
        uint8_t input = Serial.read();
        Serial.write((input >= 0x20 && input <= 0x7e) ? input : 0x20);
        if ((idx < sizeof(buffer) - 1) && (input != 0x0d) && (input != 0x0a)){   //return
          if (input >= 0x30 && input <= 0x7a)
            buffer[idx++] = input;
          }
        else{
          Serial.println("");
          clearSerial();
          buffer[idx] = 0x00;
          return strtoul(buffer, NULL, 10);
          }
        }
      delay(100);
      }
    }

  void printPrompt(){                                               // print the prompt
    Serial.print(PROMPT);
    }

  void printCliHeader(){                                            // print cli table header
    Serial.println(SEPARATOR);
    Serial.println(TABLE_HEADER);
    Serial.println(SEPARATOR);
    }

  
  void valueInput(){                                                // get plain values
        index=0x0;
        Serial.print("1 mode   (0-3f)"); byteArray[2][1] =  ((uint8_t) readCliHex() ) & 0x3f;
        Serial.print("2 green  (0-3f)"); byteArray[2][2] =  ((uint8_t) readCliHex() ) & 0x3f;
        Serial.print("3 red    (0-3f)"); byteArray[2][3] =  ((uint8_t) readCliHex() ) & 0x3f; 
        Serial.print("4 blue   (0-3f)"); byteArray[2][4] =  ((uint8_t) readCliHex() ) & 0x3f; 
        Serial.print("5 attack (0-7)");  byteArray[2][5] = (((uint8_t) readCliHex() ) & 0x7) << 3; 
        Serial.print("5 random (0-7)");  byteArray[2][5] += ((uint8_t) readCliHex() ) & 0x7; 
        Serial.print("6 release(0-7)");  byteArray[2][6] = (((uint8_t) readCliHex() ) & 0x7) << 3; 
        Serial.print("6 hold   (0-7)");  byteArray[2][6] += ((uint8_t) readCliHex() ) & 0x7; 
        Serial.print("7 Group  (0-3f)"); byteArray[2][7] =  ((uint8_t) readCliHex() ) & 0x3f; 
   }
  void plainInput(){
        Serial.print("input mode   (0-3f)"); byteArray[2][1] = ((uint8_t) readCliHex()) & 0x3f;
        Serial.print("input green  (0-3f)"); byteArray[2][2] = ((uint8_t) readCliHex()) & 0x3f;
        Serial.print("input red    (0-3f)"); byteArray[2][3] = ((uint8_t) readCliHex()) & 0x3f;
        Serial.print("input blue   (0-3f)"); byteArray[2][4] = ((uint8_t) readCliHex()) & 0x3f;
        Serial.print("input rise   (0-3f)"); byteArray[2][5] = ((uint8_t) readCliHex()) & 0x3f;
        Serial.print("input fall   (0-3f)"); byteArray[2][6] = ((uint8_t) readCliHex()) & 0x3f;
        Serial.print("input group  (0-3f)"); byteArray[2][7] = ((uint8_t) readCliHex()) & 0x3f;
    }
  void commandLineInterface() {                                     // minimalistic command line interface
    char selection = readCliByte();
    switch (selection){
      case '0':  // transmitter off, (put badge to sleep) 
        index=0x00;
        //crcTest=0;
        break;
      case '1':  // transmitter on, black
        index=0x01;
        break;
      case '2':  // transmitter on, smoth red fade - test device response
        index=0x02;
        break;
      case 'f':  // transfer string and brute CRC
        Serial.println(" transfer & brute CRC ");
        plainInput();                              //get plain values
        setCRC(byteArray[2]);
        showBuffer(byteArray[2]);
        //exec = 'f';
        index=0x03;
        break;
      case 'w':  // custom transfer plain string
        Serial.print("9 ");
        // 11 22 33 44 55 66 77
        superInput = readCliHex();
        byteArray[2][1] = (uint8_t)((superInput >> 48) & 0x3f);
        byteArray[2][2] = (uint8_t)((superInput >> 40) & 0x3f);
        byteArray[2][3] = (uint8_t)((superInput >> 32) & 0x3f);
        byteArray[2][4] = (uint8_t)((superInput >> 24) & 0x3f);
        byteArray[2][5] = (uint8_t)((superInput >> 16) & 0x3f);
        byteArray[2][6] = (uint8_t)((superInput >> 8) & 0x3f);
        byteArray[2][7] = (uint8_t)(superInput & 0x3f);;
        setCRC(byteArray[2]);
        //exec = 'w';
        index=0x3;
        break;
      case '?':  // help index
        Serial.println(SEPARATOR);
        Serial.println("|                   PixMob                 |");
        Serial.println(SEPARATOR);
        Serial.println(" 0 - transmitter off, device sleep");
        Serial.println(" 1 - black, device wakeup");
        Serial.println(" 2 - smoth red fade");
        Serial.println(" f - send values + CRC from table");
        Serial.println(" w - response test - hex-string");
        Serial.println(" ? - this help");
        Serial.println(SEPARATOR);
        break;
      default:
        break;
      }
    if(byte(selection) > 0){
      #ifndef TRANSMITTER
        getPortD = 0;
      #endif

      }
    }
//
// ------------------ SETUP ------------------- arduino setup  
  #ifndef TRANSMITTER
    ISR (PCINT2_vect){                                  // PCINT2_vect: interrupt vector for PORTD
    getPortD = PIND;
    //event = true;
    }
  #endif

  void setup() {                                    //SETUP
    #ifndef TRANSMITTER
      pinMode(LED, OUTPUT);
      PCICR  = (1<<PCIE2);    // enable PCINT[23:16] interrupts
      PCMSK2 = (1<<PCINT18) | (1<<PCINT19) | (1<<PCINT20) | (1<<PCINT22) ; // unmask PCINT 18-20,22
    #endif

    pinMode(PINSCL, INPUT);
    pinMode(PINSDA, INPUT);
    pinMode(PINRED, INPUT);
    pinMode(PINGREEN, INPUT);
    pinMode(PINBLUE, INPUT);


    Serial.begin(CLI_SPEED);                     // debug
    delay(3000);
    initTX();
    pinMode(TX, OUTPUT);
    Serial.println("done");

    printCliHeader();
    printPrompt();
    }
//
// ------------------ SUBS --------------------
  bool testPins(){
    bool pin = 1;
    #ifdef TRANSMITTER
      for(byte i=0;i<50;i++){
        pin   &= digitalRead(PINSDA) & digitalRead(PINRED) & digitalRead(PINGREEN) & digitalRead(PINBLUE);
        }
    #else
      if(getPortD){
        pin = bool(0b01011100 - (getPortD & 0b01011100));
        ledValue = 1-ledValue; 
        }
      getPortD = 0;
    #endif
    return pin;
    }
  void getStatus(){
    sclin = digitalRead(PINSCL);
      if( sclin and  sclAlt) rxReady = false;                    // H = sleeping
      if(!sclin and  sclAlt) stayAlive();                        // H -> L = wake up: transmit stay alive
      if(!sclin and !sclAlt) rxReady = true;                     // L = active
      if( sclin and !sclAlt) rxReady = false;                    // L -> H = gone sleeping
      sclAlt = sclin;
    }

  void stayAlive(){
    delay(40);
    sendMessage(byteArray[0]);                    //transmit wakeup
    }
//  
// ------------------ MAIN --------------------
void loop() {
  commandLineInterface();
  if(index > 0) sendMessage(byteArray[index-1]);
  #ifndef TRANSMITTER
    digitalWrite(LED,ledValue);
  #endif
  delay(40);
  indexAlt=index;
  }
