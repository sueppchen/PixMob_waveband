/*       testscript for PixMob waveband - (RF enabled crowd pixel)
 2024 sueppchen 
 processing frontend for bruteforcing with 2 pixmobs
 data organisation must be done in Host
 serial protocol 8n1:9600
 input A = '0' - '3' (/ B = '4' - '7')
  '0'= off, '1'=black, '2' = red, '3' + 000000gg ggggrrrr rrbbbbbb
 
 output = C: xx 00 gg rr bb 08 12 yy\r\n
  
 hardware
  Arduino nano / mini Pro ATmega328
  GND, R, G, B, SCL, SDA, TX, (VCC5) 2, 3, 4, 5, 6, 7
  GND, R, G, B, SCL, SDA, TX, (VCC5) 8, 9,10,11,12,14

*/

// global definitions
  #define LED 13
  #define CLI_SPEED  9600                                           //ATmega328 = nano, Mini Pro 8MHz --> save time 
  #define START_DELAY    1                  // wait until hf is present ... us

// special definitions
  #define PINREDA    2        
  #define PINGREENA  3
  #define PINBLUEA   4
  #define PINSCLA    5
  #define PINSDAA    6
  #define TXA        7                           // data output

  #define PINREDB    8        
  #define PINGREENB  9
  #define PINBLUEB  10
  #define PINSCLB   11
  #define PINSDAB   12
  #define TXB       14                           // data output
  
// TX definitions
  #define BIT_TIME       500                  // inter bit delay ... us
  #define PREAMBLE      0x55                  // preamble ... send twice
  #define SYNC1            0                  // 
  #define SYNC2            1                  // 

// global variables
  bool ledValue;
  uint8_t byteArray[4][9] = {
    // 0     1     2     3     4     5     6     7     8       
    // 0b1101010, 0b100001, 0b100001, 0b100001, 0b100001, 0b1000110, 0b1101100, 0b100001, 0b10010100
    // 6a
    {0x1B, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0F, 0x00, 0x15}, //1, black as background, active for 1 minute
    {0x37, 0x00, 0x00, 0x3f, 0x00, 0x38, 0x3E, 0x00, 0x20}, //2, red, smoooooooth fade
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //3, test-string A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //4, test-string B
    };

// special variables
  bool crcTestA, rxReadyA, activeA, pinA, sclinA = 1, sclAltA=1; 
  bool crcTestB, rxReadyB, activeB, pinB, sclinB = 1, sclAltB=1;
  uint8_t indexA, indexB;
  uint32_t superValueA, superValueB;

//

// ------------------ TX ----------------------                     tx related stuff
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

  uint8_t lineCode(uint8_t inByte){                                 // convert plain values into 6b8b line code
    inByte &= 0x7f;
    return dictTable[inByte];
    }
  
  void transmitBit(bool txBit, bool output){                        // transmit single bit
    output ? digitalWrite(TXB,txBit) : digitalWrite(TXA,txBit);
    delayMicroseconds(BIT_TIME);
    }

  void transmitByte(uint8_t txByte, bool out){                      // transmit byte
    for(int bitCounter=0; bitCounter<8; bitCounter++)  {
        bool bit=(((txByte) >> (bitCounter)) & 0x01);
        transmitBit(bit, out);
      }
    }


  void sendMessage(uint8_t* buffer,bool pin){                       // transmit one frame
    delayMicroseconds(START_DELAY);                                 //wait until hf is present
    //preamble
    transmitByte(PREAMBLE, pin);
    transmitByte(PREAMBLE, pin);
    //resync
    transmitBit(SYNC1, pin);
    transmitBit(SYNC2, pin);
    //payload: crc1, mode, green, red, blue, rise, fall, group, crc2
    for(uint8_t byteCounter=0; byteCounter < 9; byteCounter++ )
      transmitByte( lineCode( buffer[byteCounter] ), pin );
    pin ? digitalWrite(TXB,0) : digitalWrite(TXA,0);
    }
//

// ------------------ SI ---------------------                      serial interface
  void showBuffer(bool client, uint8_t* buffer){                    // send data to host 
    Serial.print(client);
    Serial.print(":");
    for(uint8_t i = 0; i < 9; i++ ) {
      Serial.print(" ");
      if( buffer[i] < 0x10) Serial.print("0");
      Serial.print(buffer[i],HEX);
      }
    Serial.print("\r\n");
    }

  void clearSerial(){                                               // read and discard data on serial port 
    while (Serial.available() > 0){
      Serial.read();
      }
    }

  uint8_t readCliByte(){                                            // read si single byte 
    uint8_t input=0;
    if (Serial.available() > 0){
      input = Serial.read();
      Serial.write((input >= 0x20 && input <= 0x7e) ? input : 0x20);
      Serial.println();
      clearSerial();
      }
    return input;
    }


  uint32_t readCliHex(){                                            // read si hex value 
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
          return strtoul(buffer, NULL, 16);
          }
        }
      delay(100);
      }
    }

  void valueInput(bool client){                                     // get values from host 
    //gggggg, rrrrrr, bbbbbb,
    //0b 00 00000000 00000000 
    // 0x3ffff
    uint32_t si = (uint32_t) readCliHex();
    
    if(client){
      byteArray[2][0] = 0x00;
      byteArray[2][1] = 0x00;
      byteArray[2][2] = ((uint8_t) (si >> 12) ) & 0x3f;
      byteArray[2][3] = ((uint8_t) (si >>  6) ) & 0x3f; 
      byteArray[2][4] = ((uint8_t) (si      ) ) & 0x3f; 
      byteArray[2][5] = 0x08; 
      byteArray[2][6] = 0x12; 
      byteArray[2][7] = 0x00;
      byteArray[2][8] = 0x00; 
      activeA = true;
      }
    else{
      byteArray[3][0] = 0x00;
      byteArray[3][1] = 0x00;
      byteArray[3][2] = ((uint8_t) (si >> 12) ) & 0x3f;
      byteArray[3][3] = ((uint8_t) (si >>  6) ) & 0x3f; 
      byteArray[3][4] = ((uint8_t) (si      ) ) & 0x3f; 
      byteArray[3][5] = 0x08; 
      byteArray[3][6] = 0x12; 
      byteArray[3][7] = 0x00;
      byteArray[3][8] = 0x00; 
      activeB = true;
      } 
    }

  void serialInterface() {                                          // serial interface 
    char selection = (readCliByte() & 0x7);                         //0,1,2,3   4,5,6,7
    bool client = (bool)((selection) >> 2);                         //  0=A       1=B
    switch (selection){
      case 0:  // transmitter off, (put badge A to sleep) 
        indexA = 0;
        crcTestA = 0;
        break;
      case 1:  // transmitter A on, black
        indexA = 1;
        break;
      case 2:  // transmitter A on, smoth red fade - test device response
        indexA = 2;
        break;
      case 3:  // transfer string and brute CRC
        valueInput(0);                              //get plain values
        indexA = 3;
        break;
      case 4:  // transmitter off, (put badge B to sleep) 
        indexB = 0;
        crcTestB = 0;
        break;
      case 5:  // transmitter on, black
        indexB = 1;
        break;
      case 6:  // transmitter on, smoth red fade - test device response
        indexB = 2;
        break;
      case 7:  // transfer string and brute CRC
        valueInput(1);                              //get plain values
        indexB = 4;
        break;
      default:
        break;
      }
    }
//

// ------------------ SETUP -------------------                     arduino setup
  void setup() {                                                    //SETUP
    pinMode(LED,      OUTPUT);
    
    pinMode(TXA,      OUTPUT);
    pinMode(PINSCLA,   INPUT);
    pinMode(PINSDAA,   INPUT);
    pinMode(PINREDA,   INPUT);
    pinMode(PINGREENA, INPUT);
    pinMode(PINBLUEA,  INPUT);

    pinMode(TXB,      OUTPUT);
    pinMode(PINSCLB,   INPUT);
    pinMode(PINSDAB,   INPUT);
    pinMode(PINREDB,   INPUT);
    pinMode(PINGREENB, INPUT);
    pinMode(PINBLUEB,  INPUT);

    Serial.begin(CLI_SPEED);                     // debug
    delay(100);
    }
//

// ------------------ SUBS --------------------                     functions
  void done(bool client){                                           // finished - back to idle 
    if(client){
      indexB=1;
      crcTestB=false;
      activeB=false;
      }
    else{
      indexA=1;
      crcTestA=false;
      activeA=false;
      }
    }

  bool testPins(bool client){                                       // check pixmob output 
    if(client){
      for(byte i=0;i<50;i++){
        pinB   &= digitalRead(PINSDAB) & digitalRead(PINREDB) & digitalRead(PINGREENB) & digitalRead(PINBLUEB);
        }
      return pinB;
      }
    else{
      for(byte i=0;i<50;i++){
        pinA   &= digitalRead(PINSDAA) & digitalRead(PINREDA) & digitalRead(PINGREENA) & digitalRead(PINBLUEA);
        }
      return pinA;
      }    
    }

  void stayAlive(bool client){                                      // send 60 sec alive frame 
    delay(40);
    sendMessage(byteArray[0], client);                    //transmit wakeup
    }

  void getStatus(){                                                 // get pixmob status 
    sclinA = digitalRead(PINSCLA);
      if( sclinA and  sclAltA) rxReadyA = false;                    // H = sleeping
      if(!sclinA and  sclAltA) stayAlive(0);                        // H -> L = wake up: transmit stay alive
      if(!sclinA and !sclAltA) rxReadyA = true;                     // L = active
      if( sclinA and !sclAltA) rxReadyA = false;                    // L -> H = gone sleeping
      sclAltA = sclinA;

    sclinB = digitalRead(PINSCLB);
      if( sclinB and  sclAltB) delay(2);                             // H = sleeping 
      if(!sclinB and  sclAltB) stayAlive(1);                         // H -> L = wake up: transmit stay alive
      if(!sclinB and !sclAltB) rxReadyB = true;                      // L = active 
      if( sclinB and !sclAltB) rxReadyB = false;                     // L -> H = gone sleeping
      sclAltB = sclinB;

    }


  void next(bool client){                                           // next crc
    ledValue != ledValue;            //blink
    // 11..6      5..0
    // crcIndexH, crcIndexL

    if(client){
      superValueB += (byteArray[3][0] << 6);
      superValueB += (byteArray[3][8]);
      if(superValueB == 0xfff){
        done(1);
        Serial.print("1:XX");
        return;
        }
      superValueB ++;
      byteArray[3][0] = (superValueB >> 6) & 0x3f;
      byteArray[3][8] = superValueB & 0x3f;

      if((superValueB % 0x80) == 0) stayAlive(1);
      }
    else{
      superValueA += (byteArray[2][0] << 6);
      superValueA += (byteArray[2][8]);
      if(superValueA == 0xfff){
        done(0);
        Serial.print("0:XX");
        return;
        }
      superValueA ++;
      byteArray[2][0] = (superValueA >> 6) & 0x3f;
      byteArray[2][8] = superValueA & 0x3f;

      if((superValueA % 0x80) == 0) stayAlive(0);
      }

    }
//  

// ------------------ MAIN --------------------                     main loop
  void loop() {
    serialInterface();                                                // new input ?
    getStatus();                                                      // get pixmob status
    
    if(indexA > 0) sendMessage(byteArray[indexA-1],0);                // 0 off, 1-3
    else if (crcTestA){                                               
      //wait for receiver to be ready - SCL goes low
      if(rxReadyA){
        sendMessage(byteArray[2],0);                                 // transmit new values
        //poll: did something happen?
        if (testPins(0) == 0){                                       // match
          showBuffer(0,byteArray[2]);
          done(0);                                                   // back to idle1
          stayAlive(0);                                              // transmit stay Alive
          delay(500);
          } 
        else next(0);                                                // no match, try next crc
        }
      }

    if(indexB > 0) sendMessage(byteArray[indexB-1],1);
    else if (crcTestB){                                               
      //wait for receiver to be ready - SCL goes low
      if(rxReadyB){
        sendMessage(byteArray[3],1);       //transmit new values
        //poll: did something happen?
        if (testPins(1) == 0){                      //match
          showBuffer(1,byteArray[3]);
          done(1);                                  //end
          stayAlive(1);                             //transmit stay Alive
          delay(500);
          } 
        else next(1);                               //no match, try next crc -- client, end
        }
      }

    // global stuff
    digitalWrite(LED,ledValue);
    delay(40);
    }
