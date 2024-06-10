/*       testscript for PixMob waveband - (RF enabled crowd pixel)

 library - edition

 2024 sueppchen, Serge_45
 
 TX:
  arduino pro micro /ATmega32u4
  levelshifter
  cc1101 radio module

  send custom color with attaack and release
*/

#define RED    128      // 0 - 255
#define GREEN   30      // 0 - 255
#define BLUE     0      // 0 - 255
#define ATTACK   1      // 0 -  7 (0 = fast)
#define HOLD     2      // 0 -  7 (7 = forever)
#define RELEASE  2      // 0 -  7 (0 = background color)
#define RANDOM   2      // 0 -  7 (0 = no random)
#define GROUP    0      // 0 - 31 (0 = all batches) 

// ------------ global ------------
#include <pixmob_cement.h>

// ------------ I/O ------------
#define CLI_SPEED 115200                                           
#define TX 7                           // data output pin


// ------------ TX ----------
Pixmob batch;                          //create the pixmob instance


void activeDelay(uint16_t time){
  for(uint16_t i = 0; i< (time/40); i++){
    delay(40);
    batch.refresh();
  }
}

// ------------------ SETUP ------------------- arduino setup  
  void setup() {                                    //SETUP

    Serial.begin(CLI_SPEED);                        // debug
    delay(3000);
    if(!batch.begin(TX,EU)){                        // use EU for 868 MHz and US for 915 MHz
      Serial.println("CC1101 init fail");
      while(true);                                  //loop forever
    }
    else {
      Serial.println("CC1101 init done");
      delay(1000);
    }
  }
//


// ------------------ MAIN --------------------
void loop() {
    batch.setFXtiming(0, 0, 0, 0);
    batch.sendColor(0, 0, 0, 0);
    activeDelay(5000);
    batch.setFXtiming(ATTACK, HOLD, RELEASE, RANDOM);
    batch.sendColor(RED, GREEN, BLUE, GROUP);
    activeDelay(5000);
    batch.flashDual(128, 0, 0, 0, 0, 128);
    activeDelay(2000);
  }
