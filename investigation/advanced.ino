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
  bool silent, debug;

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

uint8_t index, indexAlt, repeatSet=1;
bool crcTest, rxReady, match, ledValue;
uint8_t testByte, repeat;
uint8_t h, exec;
bool sclin = 1, sclAlt=1;// , pinB = 1;



uint32_t superValue;

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

  // some values are missing in the next two tables, replaced by _NONE_ (0xFFFF)
  // cannot be used to calculate a valid CRC
  #define _NONE_ 0xFFFF
  uint16_t table_M[] = {
      0x0000, _NONE_, 0x3D30, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_,  
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_,  
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_}; 

  uint16_t table_G[] = { 
      0x0000, 0x0f0f, 0x1129, 0x1b23, 0x2800, 0x050d, 0x0b32, 0x0a12, // 00 - 07
      0x2d25, 0x3c14, 0x223a, 0x330b, 0x271f, 0x2808, 0x3901, 0x3c24, // 08 - 0F
      0x153c, 0x1019, 0x1f16, 0x0128, 0x0138, 0x041d, 0x1a33, 0x0b02, // 10 - 17
      0x331b, 0x363e, 0x0008, 0x2d05, 0x0e27, 0x143c, 0x1b33, 0x0a02, // 18 - 1F
      0x1131, 0x0b2a, 0x1031, 0x1514, 0x0e0f, 0x1534, 0x0100, 0x0405, // 20 - 27
      0x0425, 0x1009, 0x0e17, 0x152c, 0x1a1b, 0x0120, 0x1a3b, 0x1f1e, // 28 - 2F
      0x0e07, 0x1f36, 0x3911, 0x2820, 0x3313, 0x2222, 0x222a, 0x3919, // 30 - 37
      0x1e3e, 0x0505, 0x3636, 0x1b3b, 0x2d2d, 0x3616, 0x2d0d, 0x3333}; // 38 - 3f

  uint16_t table_R[] = {
      0x0000, 0x3a1b, 0x3f02, 0x0e2d, 0x0e2f, 0x020f, 0x0005, 0x0d25, 
      0x083f, 0x0b37, 0x0715, 0x041d, 0x0105, 0x0714, 0x3532, 0x3322,
      0x0103, 0x0713, 0x3d08, 0x041b, 0x312a, 0x373a, 0x3b18, 0x3810, 
      0x312c, 0x373c, 0x093b, 0x051b, 0x3e00, 0x0107, 0x3b1c, 0x3814,
      0x0308, 0x3c0f, 0x030c, 0x051c, 0x3a1f, 0x0838, 0x0004, 0x0b30, 
      0x0614, 0x3222, 0x0615, 0x3432, 0x3f07, 0x0d20, 0x3223, 0x3433,
      0x3324, 0x302c, 0x0003, 0x030b, 0x3817, 0x3b1f, 0x3224, 0x0938, 
      0x3913, 0x0b34, 0x3e07, 0x3227, 0x0104, 0x3323, 0x0c20, 0x3533};

  uint16_t table_B[]  = {
      0x0000, 0x3108, 0x1108, 0x0f07, 0x373b, 0x3a2f, 0x0b20, 0x0630, 
      0x0212, 0x0b25, 0x3e0d, 0x373a, 0x0b24, 0x2213, 0x2935, 0x1c1c, 
      0x2426, 0x110f, 0x2007, 0x1838, 0x152f, 0x2006, 0x152e, 0x1c19, 
      0x3a2d, 0x0f04, 0x1528, 0x183c, 0x2930, 0x3318, 0x0210, 0x0b27, 
      0x0937, 0x131f, 0x1e09, 0x2b20, 0x2636, 0x310e, 0x173e, 0x3839, 
      0x2217, 0x1c18, 0x3e09, 0x2931, 0x1a28, 0x0d10, 0x0006, 0x352f, 
      0x331e, 0x3a29, 0x2422, 0x2d15, 0x2f05, 0x2632, 0x331a, 0x310a, 
      0x383f, 0x2f07, 0x1a2c, 0x1738, 0x173a, 0x0002, 0x0d14, 0x352b};

  uint16_t table_AR[] = {
      0x0000, 0x2037, 0x2e38, 0x3320, 0x3c18, _NONE_, _NONE_, _NONE_, 
      0x0d05, 0x0628, 0x2b2b, 0x2006, 0x1a36, _NONE_, _NONE_, _NONE_, 
      0x2c39, 0x1d24, 0x3d13, 0x1608, 0x1010, _NONE_, _NONE_, _NONE_, 
      0x261f, 0x1702, 0x230c, 0x0137, 0x363e, _NONE_, _NONE_, _NONE_,  
      0x0b2d, 0x281d, 0x321a, 0x0307, _NONE_, _NONE_, _NONE_, _NONE_, 
      0x082a, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_,  
      0x3a0c, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      0x2b1a, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_};  

  uint16_t table_RH[] = {
      0x0000, 0x0506, 0x3f19, 0x2420, 0x1532, 0x0e39, 0x090f, 0x2b33, 
      0x0b14, 0x0407, 0x2e1e, 0x210d, 0x3038, 0x0534, 0x3a34, 0x2412,  
      0x230f, 0x3d29, 0x382f, 0x323a, 0x1236, 0x0c10, 0x2609, 0x291a, 
      0x0101, 0x1f27, 0x1006, 0x0b0d, 0x373c, 0x012a, 0x042c, 0x0b3f,  
      0x0f13, 0x3905, 0x2d36, 0x3310, 0x2723, 0x3309, 0x2225, 0x3c1a, 
      0x3c03, 0x1d25, 0x1729, 0x0303, 0x3616, 0x223c, 0x360f, 0x2829,  
      0x3725, 0x3836, 0x1a38, 0x152b, 0x1107, 0x1e14, 0x0e12, 0x0a3e, 
      0x0a15, 0x1e3f, 0x0f21, 0x142a, 0x1b12, 0x0f38, 0x1b0b, 0x111e};

  uint16_t table_GP[] = {
      0x0000, 0x2F16, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_,  
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_,  
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, 
      _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_, _NONE_};  

  uint16_t setCRC(uint8_t *message) {          // set calculated CRC in the given message
    // return CRC1 and CRC2 values as a 16-bit word : 00aaaaaa 00bbbbbb, a is the CRC1 value, b is the CRC2 value
    uint8_t mod = message[1] & 0x3f;
    uint8_t g = message[2] & 0x3f;
    uint8_t r = message[3] & 0x3f;
    uint8_t b = message[4] & 0x3f;
    uint8_t ar = message[5] & 0x3f;
    uint8_t rh = message[6] & 0x3f;
    uint8_t gp = message[7] & 0x3f;
    uint16_t crc = INIT ^ table_M[mod] ^ table_G[g] ^ table_R[r] ^ table_B[b] ^ table_AR[ar] ^ table_RH[rh] ^ table_GP[gp];
    
    // add a warning if one of the values if 0xffff
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
  
  uint8_t reverseLineCode(uint8_t inByte){                          //convert 6b8b line code into plain values
    for(uint8_t i = 0; i<64; i++)
      if(inByte == dictTable[i]) 
        return i;
    return 0xff;
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

  void showLineCode(){                                              // show 6b8b line code as table
      Serial.print("\r\n   ");
      for(byte i=0;i<0x10;i++) {Serial.print(i,HEX);Serial.print("   ");}
      Serial.print("\r\n0:");
      for(byte i=0;i<0x10;i++) {Serial.print(dictTable[i],HEX);Serial.print("  ");}
      Serial.print("\r\n1:");
      for(byte i=0x10;i<0x20;i++) {Serial.print(dictTable[i],HEX);Serial.print("  ");}
      Serial.print("\r\n2:");
      for(byte i=0x20;i<0x30;i++) {Serial.print(dictTable[i],HEX);Serial.print("  ");}
      Serial.print("\r\n3:");
      for(byte i=0x30;i<0x40;i++) {Serial.print(dictTable[i],HEX);Serial.print("  ");}
      Serial.println();
    }
  void showBuffer(uint8_t* buffer){                                 // print buffer (plain + encoded)
    Serial.print("LC: ");
    for(uint8_t i = 0; i < 9; i++ ) {
      Serial.print(lineCode(buffer[i]),HEX);
      Serial.print(" ");
      }
    Serial.print("P: ");
    for(uint8_t i = 0; i < 9; i++ ) {
      if( buffer[i] < 0x10) Serial.print("0");
      Serial.print(buffer[i],HEX);
      Serial.print(" ");
      }
    Serial.print("\r\n");
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
        showLineCode();
        index=0x0;
        crcTest=1;
        Serial.print("1 mode   (0-3f)");
         byteArray[2][1] = ((uint8_t) readCliHex() ) & 0x3f;
        Serial.print("2 green  (0-3f)");
         byteArray[2][2] = ((uint8_t) readCliHex() ) & 0x3f;
        Serial.print("3 red    (0-3f)");
         byteArray[2][3] = ((uint8_t) readCliHex() ) & 0x3f; 
        Serial.print("4 blue   (0-3f)");
         byteArray[2][4] = ((uint8_t) readCliHex() ) & 0x3f; 
        Serial.print("5 attack (0-7)");
         byteArray[2][5] = (((uint8_t) readCliHex() ) & 0x7) << 3; 
        Serial.print("5 random (0-7)");
         byteArray[2][5] += ((uint8_t) readCliHex() ) & 0x7; 
        Serial.print("6 release(0-7)");
         byteArray[2][6] = (((uint8_t) readCliHex() ) & 0x7) << 3; 
        Serial.print("6 hold   (0-7)");
         byteArray[2][6] += ((uint8_t) readCliHex() ) & 0x7; 
        Serial.print("7 Group  (0-3f)");
         byteArray[2][7] = ((uint8_t) readCliHex() ) & 0x3f; 
         repeatSet = (byteArray[2][5] & 0x7) + (byteArray[2][5] >> 4);  //high random and slow attack --> slow testing

   }
  void plainInput(){
        Serial.print("input mode   (0-3f)"); byteArray[2][1] = (uint8_t) readCliHex();
        Serial.print("input green  (0-3f)"); byteArray[2][2] = (uint8_t) readCliHex();
        Serial.print("input red    (0-3f)"); byteArray[2][3] = (uint8_t) readCliHex();
        Serial.print("input blue   (0-3f)"); byteArray[2][4] = (uint8_t) readCliHex();
        Serial.print("input rise   (0-3f)"); byteArray[2][5] = (uint8_t) readCliHex();
        Serial.print("input fall   (0-3f)"); byteArray[2][6] = (uint8_t) readCliHex();
        Serial.print("input group  (0-3f)"); byteArray[2][7] = (uint8_t) readCliHex();
    }
  void commandLineInterface() {                                     // minimalistic command line interface
    char selection = readCliByte();
    switch (selection){
      case '0':  // transmitter off, (put badge to sleep) 
        index=0x00;
        crcTest=0;
        break;
      case '1':  // transmitter on, black
        index=0x01;
        break;
      case '2':  // transmitter on, smoth red fade - test device response
        index=0x02;
        break;
      case 's':  // toggle silence
        silent = ! silent;
        Serial.println( silent? "silent..." : "verbose...");
        break;
      case 'v':  // toggle debug
        debug = ! debug;
        Serial.println( debug? "debug on..." : "debug off...");
        break;
      case 'a':  // custom transfer raw 6b8b string
        Serial.println(" send raw 6b8b data ");
        showLineCode();
        Serial.print("input crc1   (hex8)"); 
         byteArray[2][0] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("input mode   (hex8)"); 
         byteArray[2][1] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("input green  (hex8)");
         byteArray[2][2] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("input red    (hex8)");
         byteArray[2][3] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("input blue   (hex8)");
         byteArray[2][4] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("input rise   (hex8)");
         byteArray[2][5] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("input fall   (hex8)");
         byteArray[2][6] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("input group  (hex8)");
         byteArray[2][7] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("input crc2   (hex8)");
         byteArray[2][8] = reverseLineCode((uint8_t) readCliHex());
        Serial.print("\r\nsending string:\r\n");
        showBuffer(byteArray[2]);
        index=0x03;
        break;
      case 'b':  // custom transfer plain string
        Serial.println(" send data with known crc");
        showLineCode();
        Serial.print("input crc1   (0-3f)"); byteArray[2][0] = (uint8_t) readCliHex();
        plainInput();
        Serial.print("input crc2   (0-3f)"); byteArray[2][8] = (uint8_t) readCliHex();
        Serial.print("\r\nsending string:\r\n");
        showBuffer(byteArray[2]);
        exec = 'b';
        index=0x03;
        break;
      case 'c':  // transfer string and brute CRC
        Serial.println(" transfer & brute CRC ");
        valueInput();                              //get plain values
        Serial.print("start crcH (0-3f) = (hex8):"); byteArray[2][0]=( (uint8_t) readCliHex() ) & 0x3f;
        Serial.print("start crcL (0-3f) = (hex8):"); byteArray[2][8]=( (uint8_t) readCliHex() ) & 0x3f; 
        testByte = 2;
        Serial.print(" .. testing .. \r\n");
        showBuffer(byteArray[2]);
        exec = 'c';
        break;
      case 'd':  // transfer string and brute CRC
        Serial.println(" transfer & brute CRC ");
        plainInput();                              //get plain values
        setCRC(byteArray[2]);
        showBuffer(byteArray[2]);
        exec = 'd';
        index=0x03;
        break;
      case 'w':  // custom transfer plain string
        Serial.print("9 ");
        // 00 22 33 44 55 66 88
        superInput = readCliHex();
        byteArray[2][0] = (uint8_t)((superInput >> 48) & 0x3f);;
        byteArray[2][1] = 0;
        byteArray[2][2] = (uint8_t)((superInput >> 40) & 0x3f);;
        byteArray[2][3] = (uint8_t)((superInput >> 32) & 0x3f);;
        byteArray[2][4] = (uint8_t)((superInput >> 24) & 0x3f);;
        byteArray[2][5] = (uint8_t)((superInput >> 16) & 0x3f);;
        byteArray[2][6] = (uint8_t)((superInput >> 8) & 0x3f);;
        byteArray[2][7] = 0;
        byteArray[2][8] = (uint8_t)(superInput & 0x3f);;
        exec = 'q';
        crcTest = true;
        index=0x0;
        break;
      case '?':  // help index
        Serial.println(SEPARATOR);
        Serial.println("|                   PixMob                 |");
        Serial.println(SEPARATOR);
        Serial.println(" 0 - transmitter off, device sleep");
        Serial.println(" 1 - black, device wakeup");
        Serial.println(" 2 - smoth red fade");
        Serial.println(" s - toggle silent mode");
        Serial.println(" v - toggle debug mode");
        Serial.println(" a - send raw 6b8b frame");
        Serial.println(" b - send raw plain frame");
        Serial.println(" c - send values + brute the CRC");
        Serial.println(" D - send values + CRC from table");
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
      //printPrompt();

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
  void done(){
    if(exec != 'q') Serial.print("--- DONE ---");
    index=1;
    crcTest=false;
    }
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
  void next(bool end){
    ledValue = 1- ledValue;            //blink

    //  29..24    23..18    17..12    11..6      5..0
    // array2.4, array2.3, testIndex, crcIndexH, crcIndexL

    superValue =  (uint32_t)byteArray[2][4] << 24;
    superValue += (uint32_t)byteArray[2][3] << 18;
    superValue += (uint32_t)byteArray[2][testByte] << 12;
    superValue += (byteArray[2][0] << 6);
    superValue += (byteArray[2][8]);


    if((exec == 'c') & ((superValue & 0xfff) == 0xfff)) done();
    if(((exec == 'd') | (exec == 'e')) & ((superValue & 0xfff) == 0xfff)){
      Serial.print("CRC not Found\r\n");
      }
    if((exec == 'd') & ((superValue & 0x3ffff) == 0x3ffff)) done();
    if((exec == 'e') & ((superValue & 0xffffff) == 0xffffff)) done();

    if(end)  superValue |= 0xfff;                                             //crc = end --> next
    
    superValue ++;
    

    if(exec == 'e'){
      h = (superValue >> 24) & 0x3f;
      byteArray[2][4] = h;
      if(debug){
        Serial.print(" ");
        if(h<0x10) Serial.print("0");
        Serial.print(h,HEX);
        }
      h = (superValue >> 18) & 0x3f;
      byteArray[2][3] = h;
      if(debug){
        Serial.print(" ");
        if(h<0x10) Serial.print("0");
        Serial.print(h,HEX);
        }  
      }

    if((exec == 'e') | (exec == 'd')){
      h = (superValue >> 12) & 0x3f;
      byteArray[2][testByte] = h;
      if(debug){
        Serial.print(":");
        if(h<0x10) Serial.print("0");
        Serial.print(h,HEX);
        }
      }

    if((exec == 'e') | (exec == 'd') | (exec == 'c')){
      h = (superValue >> 6) & 0x3f;
      byteArray[2][0] = h;
      if(debug){
        Serial.print("-");
        if(h<0x10) Serial.print("0");
        Serial.print(h,HEX);
        }
      h = superValue & 0x3f;
      byteArray[2][8] =  h;
      if(debug){
        Serial.print("");
        if(h<0x10) Serial.print("0");
        Serial.print(h,HEX);
        }  
      }

    if((debug) & ((superValue % 0x8) == 0)) Serial.println();
    if((superValue % 0x80) == 0){
      stayAlive();
      if((!silent) & (!debug)){
        h = (superValue >> 6) & 0x3f;
        Serial.print(",");
        if(h<0x10) Serial.print("0");
        Serial.print(h,HEX);
        if((superValue % 0x100) == 0) Serial.println();
        
      } 
    } 

    }
//  
// ------------------ MAIN --------------------
void loop() {
  commandLineInterface();
  if(index > 0) sendMessage(byteArray[index-1]);
  else if (crcTest){
    
    //wait for receiver to be ready - SCL goes low
    getStatus();    
        
    if(rxReady){
      if(debug & (repeat==0)){
        //Serial.print(char(exec));
        Serial.print(", ");
        }  
      
      sendMessage(byteArray[2]);       //transmit new values
      //poll pins
       //poll: did something happen?
        if (testPins() == 0){                                       // match
          if(!silent){
            Serial.print(byteArray[2][testByte],HEX);
            Serial.print(" C=");
            Serial.print(byteArray[2][0],HEX);
            Serial.print(byteArray[2][8],HEX);
            Serial.print("\r\nfound\r\n");
            }
          if(exec != 'q') showBuffer(byteArray[2]);
          else Serial.println("!!");
          if((exec == 'd') | (exec == 'e')){
            next(1);
            }
          else done();                                                   // back to idle1
          stayAlive();                                              // transmit stay Alive
          delay(2000);
          #ifndef TRANSMITTER
            getPortD = 0;
          #endif            
          } 
        else if(exec != 'q'){
          if(repeat < repeatSet) repeat++;
          //if(false) repeat++;
          else{
            repeat=0;
            //go ahead!
            next(0);
            }
          }                                                            // no match, try next crc      
      }
    }
  
  #ifndef TRANSMITTER
    digitalWrite(LED,ledValue);
  #endif
  delay(40);
  indexAlt=index;
  }
