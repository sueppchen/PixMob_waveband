// testscript of for PixMob waveband - RF enabled crowd pixel
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#define RADIO_GD0 7
#define RADIO_GD2 6

#define LED 13

// ------------ CLI ---------
  #define SEPARATOR    "+-------------------------------------------+"
  #define TABLE_HEADER "| PixMob CLI-interface: type ? for help     |"
  #define PROMPT "> "

byte index, indexAlt;

void initTX(){                                      // init transmiter 
    Serial.print("init TX:");
    ELECHOUSE_cc1101.Init();                        // must be set to initialize the cc1101!
    Serial.println(" done");
    ELECHOUSE_cc1101.SetTx();
    ELECHOUSE_cc1101.setGDO(RADIO_GD0,RADIO_GD2);   // set lib internal gdo pin (gdo0). Gdo2 not use for this example.
    ELECHOUSE_cc1101.setModulation(2);              // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setCCMode(0);                  // set 1 config for internal FiFo mode. 0 for Assync external Mode
    ELECHOUSE_cc1101.setMHZ(868.49);                // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
    ELECHOUSE_cc1101.setChannel(0);                 // Set the Channelnumber from 0 to 255. Default is cahnnel 0.
    Serial.println("Tx ready");
    pinMode(RADIO_GD0, OUTPUT);
  }

void transmitBit(bool txBit){
  digitalWrite(RADIO_GD0,txBit);
  delayMicroseconds(500);
  }

void transmitByte(byte txByte){                                //
  for(int bitCounter=7; bitCounter>=0; bitCounter--)  {
      bool bit=(((txByte) >> (bitCounter)) & 0x01);
      transmitBit(bit);
    }
  }

void transmitArray(byte* buffer, byte size){                   //
  ELECHOUSE_cc1101.SetTx();                                    //enable transmitter
  delayMicroseconds(800);                                      //wait until hf is present
  //preamble
  transmitByte(0xAA);
  transmitByte(0xAA);
  //resync
  transmitBit(0);
  transmitBit(1);
  //payload
    //byte 0:   unknown - frc?
    //byte 1:   0x84 = mode?
    //byte 2-4: green, red, blue
    //byte 5:   rising speed + randomness
    //byte 6:   falling speed + hold-time
    //byte 7:   0x84 = group?
    //byte 8:   unknown - crc?
  for(byte byteCounter=0;byteCounter<size;byteCounter++)
    transmitByte(buffer[byteCounter]);
  ELECHOUSE_cc1101.SetRx() ;                             // disable transmitter
  }


//  ----------------- SETUP ---------------------------
void setup() {                                    //SETUP
  pinMode(LED, OUTPUT);

  Serial.begin(115200);                     // debug
  delay(3000);
  initTX();
  Serial.println("done");

  printTableHeader();
  printPrompt();
  }
byte ByteArray[17][9] = {
  // 0     1     2     3     4     5     6     7     8       
  { 0x56, 0x84, 0x84, 0x84, 0x84, 0x62, 0x36, 0x84, 0x29}, //1 nothing_Black
  { 0x94, 0x84, 0x91, 0xb5, 0x84, 0x8c, 0x45, 0x84, 0xad}, //2 gold_fadein_slow
  { 0x6d, 0x84, 0x91, 0xb5, 0x84, 0x49, 0x45, 0x84, 0x8a}, //3 gold_fastin_fade
  { 0x42, 0x84, 0x91, 0xb5, 0x84, 0x64, 0x6a, 0x84, 0x49}, //4 rand_gold_blink
  { 0x4a, 0x84, 0x91, 0xb5, 0x84, 0x89, 0xa9, 0x84, 0x34}, //5 rand_gold_fade
  { 0x56, 0x84, 0x91, 0xb5, 0x84, 0x29, 0x65, 0x84, 0x61}, //6 rand_gold_fastfade
  { 0xa4, 0x84, 0x84, 0xb5, 0x84, 0x89, 0x89, 0x84, 0x65}, //7 rand_red_fade
  { 0x4c, 0x84, 0x84, 0xb5, 0x84, 0x29, 0x65, 0x84, 0x45}, //8 rand_red_fast_fade
  { 0x6d, 0x84, 0x84, 0xb5, 0x84, 0x64, 0x6a, 0x84, 0x29}, //9 rand_red_fast_blink
  { 0xa6, 0x84, 0x84, 0xb5, 0x84, 0x8c, 0x45, 0x84, 0xa1}, //a wine_fade_in
  { 0x36, 0x84, 0xb5, 0x84, 0xb1, 0xb6, 0x4d, 0x84, 0x91}, //b rand_turq_blink
  { 0x84, 0x84, 0x32, 0x84, 0xb5, 0x89, 0x89, 0x84, 0x36}, //c rand_blue_fade
  { 0x4a, 0x84, 0xb5, 0xb5, 0xb5, 0x64, 0x6a, 0x84, 0x89}, //d rand_white_blink
  { 0x65, 0x84, 0xb5, 0xb5, 0xb5, 0x89, 0x89, 0x84, 0xad}, //e rand_white_fade
  { 0x9a, 0x84, 0xb5, 0xb5, 0xb5, 0x29, 0x65, 0x84, 0xa9}, //f rand_white_fast_fade
  { 0x5a, 0x84, 0xb5, 0xb5, 0xb5, 0x49, 0x45, 0x84, 0x2c}, //g white_fast_fade
  { 0x5a, 0x84, 0xb5, 0xb5, 0xb5, 0x49, 0x45, 0x84, 0x2c}  //white_fast_fade -- TEST
  };

// ------------- CLI ------------------
void clearSerial(){                                // Read and discard data on serial port
  while (Serial.available() > 0){
    Serial.read();
    }
 }

byte readCliByte(){                                // Read cli byte
  byte input=0;
  if (Serial.available() > 0){
    input = Serial.read();
    Serial.write((input >= 0x20 && input <= 0x7e) ? input : 0x20);
    Serial.println("");
    clearSerial();
    }
  return input;
  }

void printPrompt(){                                 // Print the prompt
  Serial.print(PROMPT);
  }

void printTableHeader(){                            // Print result table header
  Serial.println(SEPARATOR);
  Serial.println(TABLE_HEADER);
  Serial.println(SEPARATOR);
  }


void commandLineInterface() {                        // Minimalistic command line interface
  char selection = readCliByte();
  switch (selection){
    case '0': 
      index=0x00;
      break;
    case '1': 
      index=0x01;
      break;
    case '2': 
      index=0x02;
      break;
    case '3': 
      index=0x03;
      break;
    case '4': 
      index=0x04;
      break;
    case '5': 
      index=0x05;
      break;
    case '6': 
      index=0x06;
      break;
    case '7': 
      index=0x07;
      break;
    case '8': 
      index=0x08;
      break;
    case '9': 
      index=0x09;
      break;
    case 'a': 
      index=0x0a;
      break;
    case 'b': 
      index=0x0b;
      break;
    case 'c': 
      index=0x0c;
      break;
    case 'd': 
      index=0x0d;
      break;
    case 'e': 
      index=0x0e;
      break;
    case 'f': 
      index=0x0f;
      break;
    case 'g': 
      index=0x10;
      break;
    case 'h': 
      index=0x10;
      break;
    case '?':
      Serial.println(SEPARATOR);
      Serial.println("|                   PixMob                 |");
      Serial.println(SEPARATOR);
      Serial.println(" 0 - transmitter off");
      Serial.println(" 1 - black, wakeup!");
      Serial.println(" 2 - gold, fade in, slow");
      Serial.println(" 3 - gold, fade in, fast");
      Serial.println(" 4 - gold, pulse, random slow");
      Serial.println(" 5 - gold, fade, random slow");
      Serial.println(" 6 - gold, fade, random fast");
      Serial.println(" 7 - red, fade, random slow");
      Serial.println(" 8 - red, fade, random fast");
      Serial.println(" 9 - red, pulse, random fast");
      Serial.println(" a - wine, fade in, slow");
      Serial.println(" b - turq, pulse, random slow");
      Serial.println(" c - blue, fade, random slow");
      Serial.println(" d - white, pulse, random slow");
      Serial.println(" e - white, fade in, random slow");
      Serial.println(" f - white, fade, random fast");
      Serial.println(" g - white, fade in, fast");
      Serial.println(" h - test-signal");
      Serial.println(SEPARATOR);
      break;
    default:
      break;
    }
  if(byte(selection) > 0){
    printPrompt();
    //Serial.print(byte(selection));
    }
  }

// ---------------------- MAIN -------------------
void loop() {
  commandLineInterface();
  if(index > 0) transmitArray(ByteArray[index-1], 9);
  delay(200);
  indexAlt=index;
  }
