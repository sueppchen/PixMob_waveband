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

 full controll of pixmob device
 

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
bool rxReady, ledValue; 
uint8_t h, exec; 
bool sclin = 1, sclAlt=1;
uint64_t superInput,h64;

uint8_t mode;
bool forever, once, dual, txOn, mem;                                      //logic
uint8_t red, green, blue, red2, green2, blue2, bgRed, bgGreen, bgBlue;    //colors
uint8_t attack, hold, release, tRandom;                                   //timing
uint8_t memStore, memRandom, memFrom, memTo;                              //memory
uint8_t group, groupMem, groupStore;                                      //group


uint8_t byteArray[4][9] = {
  // 0     1     2     3     4     5     6     7     8       
  // 0b1101010, 0b100001, 0b100001, 0b100001, 0b100001, 0b1000110, 0b1101100, 0b100001, 0b10010100
  // 6a
  {0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}, //1, black as background, active for 1 minute
  {0x37, 0x00, 0x00, 0x3f, 0x00, 0x38, 0x3E, 0x00, 0x20}, //2, red, smoooooooth fade
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //3, FX
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //4, background
  };

// ------------------ TX ---------------------- tx related stuff
  #define INIT 0x1b05                            // initial value of XOR calculation
  uint16_t table[7][8] = {
    {0x2416, 0x082D, 0x371D, 0x2E3B, 0x3B30, 0x1126, 0x050B, 0x0A16},  // byte 1, bit 0 - 7
    {0x142C, 0x0F1F, 0x1E3E, 0x1B3B, 0x1131, 0x0525, 0x2D0D, 0x1A1B},  // byte 2, bit 0 - 7
    {0x3436, 0x0F2A, 0x3913, 0x3227, 0x0308, 0x0610, 0x0C20, 0x3F07},  // byte 3, bit 0 - 7
    {0x3E0F, 0x3C1F, 0x383F, 0x1738, 0x0937, 0x3529, 0x0D14, 0x1A28},  // byte 4, bit 0 - 7
    {0x1317, 0x262E, 0x2B1A, 0x1635, 0x0B2D, 0x311D, 0x223B, 0x2330},  // byte 5, bit 0 - 7
    {0x2126, 0x250A, 0x0A15, 0x142A, 0x0F13, 0x1E26, 0x1B0B, 0x3616},  // byte 6, bit 0 - 7
    {0x2C2D, 0x3F1C, 0x3E39, 0x1B34, 0x112F, 0x0519, 0x0A32, 0x3323}   // byte 7, bit 0 - 7
    };  
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



  uint16_t setCRC(uint8_t *message) {          // set calculated CRC in the given message
    // return CRC1 and CRC2 values as a 16-bit word : 00aaaaaa 00bbbbbb, a is the CRC1 value, b is the CRC2 value
    uint16_t crc = INIT;
    for(uint8_t i=0;i<7;i++){
      uint8_t value = (message[(i+1)]) & 0x3f;  // get value from message
      uint8_t coded = dictTable[value];         // encode value to 6b8b
      for(uint8_t bit = 0; bit < 8; bit++){
        if(coded & (1<<bit)) crc ^= table[i][bit];
        }
      }

    message[0] = (crc>>8) & 0x3f;
    message[8] = crc & 0x3f;
    return crc;
    }
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
      case 't':  // test: transmitter on, smoth red fade
        index=0x02;
        break;
      case 'f':  // transfer string
        Serial.println(" transfer custom ");
        plainInput();                              //get plain values        
        setCRC(byteArray[2]);
        showBuffer(byteArray[2]);
        once = 0;
        exec = 'f';
        index=0x03;
        break;
      case 'g':  // change working group
        Serial.println("change working group");
        Serial.print("group  = 0 - 31"); group = (((uint8_t) readCliDez()) & 0x1f);
        break;
      case 'w':  // write group to batch
        Serial.println("write group to batch");
        Serial.print("gpMEM = 0 -  7"); groupMem   = (((uint8_t) readCliDez()) & 0x07);
        Serial.print("group = 1 - 31"); groupStore = (((uint8_t) readCliDez()) & 0x1f);
        exec = 'w';
        once = 1;
        break;
      case 'b':  // set Background
        Serial.println("set background");
        Serial.print("red    = 0 - 63"); bgRed   = (((uint8_t) readCliDez()) & 0x3f);
        Serial.print("green  = 0 - 63"); bgGreen = (((uint8_t) readCliDez()) & 0x3f);
        Serial.print("blue   = 0 - 63"); bgBlue  = (((uint8_t) readCliDez()) & 0x3f);
        exec = 'b';
        break;
      case 'l':  // loop - mode
        Serial.println("loop");
        if(forever) Serial.println("disable");
        else Serial.println("enable");
        forever = 1 - forever;
        exec = 'l';
        break;
      case 's':  // store color
        Serial.println("store color");
        Serial.print("memory = 0 - 15"); memStore = (((uint8_t) readCliDez()) & 0x0f);
        Serial.print("red    = 0 - 63"); red   = (((uint8_t) readCliDez()) & 0x3f);
        Serial.print("green  = 0 - 63"); green = (((uint8_t) readCliDez()) & 0x3f);
        Serial.print("blue   = 0 - 63"); blue  = (((uint8_t) readCliDez()) & 0x3f);
        setCRC(byteArray[2]);
        once = 1;
        break;
      case 'p':  // play color 
        Serial.println("play color");
        Serial.print("from   mem  = 0 - 15"); memFrom   = (((uint8_t) readCliDez()) & 0x1f);
        Serial.print(" to    mem  = 0 - 15"); memTo     = (((uint8_t) readCliDez()) & 0x1f);
        Serial.print("random mem  = 0 -  7"); memRandom = (((uint8_t) readCliDez()) & 0x1f);
        Serial.print("attack      = 0 -  7"); attack    = (((uint8_t) readCliDez()) & 0x1f);
        Serial.print("hold        = 0 -  7"); hold      = (((uint8_t) readCliDez()) & 0x1f);
        Serial.print("release     = 0 -  7"); release   = (((uint8_t) readCliDez()) & 0x1f);
        Serial.print("random time = 0 -  7"); tRandom   = (((uint8_t) readCliDez()) & 0x1f);
        once = 0;
        break;
      case 'q':  // fast transfer plain string
        Serial.print("9");
        // 11 22 33 44 55 66 77
        superInput = readCliHex();
        byteArray[2][1] = (uint8_t)((superInput >> 48) & 0x3f);
        byteArray[2][2] = (uint8_t)((superInput >> 40) & 0x3f);
        byteArray[2][3] = (uint8_t)((superInput >> 32) & 0x3f);
        byteArray[2][4] = (uint8_t)((superInput >> 24) & 0x3f);
        byteArray[2][5] = (uint8_t)((superInput >> 16) & 0x3f);
        byteArray[2][6] = (uint8_t)((superInput >> 8) & 0x3f);
        byteArray[2][7] = (uint8_t)(superInput & 0x3f);
        //countBytes =      (uint8_t)((superInput >> 56) & 0x7) + 2;
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
        Serial.println(" t - smoth red fade");
        Serial.println(" f - send values + CRC from table");
        Serial.println(" q - response test - hex-string");
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
    delay(10);
    sendMessage(byteArray[0]);                    //transmit wakeup
    }
//  
// ------------------ MAIN --------------------
void loop() {
  commandLineInterface();  // get values
  //set message according to mode
  
  if (mem) mode |= 0x02;

  if(mode == 0x00){
    
  }

  if(index > 0) sendMessage(byteArray[index-1]);
  #ifndef TRANSMITTER
    digitalWrite(LED,ledValue);
  #endif
  delay(100);
  indexAlt=index;
  }
