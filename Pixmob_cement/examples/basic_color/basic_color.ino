/*       testscript for PixMob waveband - (RF enabled crowd pixel)

 library - edition

 2024 sueppchen, Serge_45
 
 TX:
  arduino pro micro /ATmega32u4
  levelshifter
  cc1101 radio module

  send custom color with attaack and release
*/

#define RED     30      // 0 - 63
#define GREEN    5      // 0 - 63
#define BLUE     0      // 0 - 63
#define ATTACK   1      // 0 -  7 (0 = fast)
#define HOLD     2      // 0 -  7 (7 = forever)
#define RELEASE  2      // 0 -  7 (0 = background color)
#define RANDOM   2      // 0 -  7 (0 = no random)
#define GROUP    0      // 0 - 31 (0 = all batches) 

// ------------ global ------------
#include <pixmob_cement.h>

// ------------ I/O ------------
#define CLI_SPEED 115200                                           //ATmega328 = nano, Mini Pro
#define TX 7                           // data output


// ------------ TX ----------
Pixmob batch;


// ------------------ SETUP ------------------- arduino setup  
  void setup() {                                    //SETUP

    Serial.begin(CLI_SPEED);                     // debug
    delay(3000);
    batch.begin(TX);

    Serial.println("done");
    delay(1000);
    
    batch.setFXtiming(ATTACK, HOLD, RELEASE, RANDOM);
    batch.sendColor(RED, GREEN, BLUE, GROUP);
    }
//

// ------------------ MAIN --------------------
void loop() {
  batch.refresh();
  delay(40);
  }
