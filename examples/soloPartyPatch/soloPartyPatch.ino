/* turn the PixMob waveband - (RF enabled crowd pixel)
 into a colorful partypatch whitch does not need a transmittter to work

 library - edition

 2024 sueppchen
 
 TX:
  arduino pro micro /ATmega32u4
  levelshifter
  cc1101 radio module

  store 16 colors
  play the stored colors with smooth fade
  in forever-mode
  all colors in 50% to save battery life
*/

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
    if(!batch.begin(TX)){
      Serial.println("CC1101 init fail");
      while(true);                                  //loop forever
    }
    else {
      Serial.println("CC1101 init done");
      delay(1000);
    }
  batch.storeColorSilent(128, 0, 0, 0);       // red
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(96, 32, 0, 1);       // orange
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(64, 64, 0, 2);       // yellow
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(32, 96, 0, 3);       // light green
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(0, 128, 0, 4);       // green
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(0, 96, 32, 5);       // cold green
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(0, 64, 64, 6);       // turkis
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(0, 32, 96, 7);       // aqua
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(0, 0, 128, 8);       // blue
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(32, 0, 96, 9);       // warm blue
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(64, 0, 64, 10);      // violett
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(96, 0, 32, 11);      // purple
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(96, 64, 10, 12);     // warm white
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(40, 40, 40, 13);     // cold white
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(0, 0, 0, 14);        // black
  activeDelay(80);                            //  repeat sending 2 times 
  batch.storeColorSilent(255, 255, 255, 15);  // full on
  activeDelay(80);                            //  repeat sending 2 times 
  

  batch.setFXtiming(7, 6, 0, 0);              // slowest attack, longest hold, be background for fading, no random 
  batch.playMemForever(0,11,0);               // fade color 0 to color 11
  activeDelay(80);                            //  repeat sending 2 times 
  }
//


// ------------------ MAIN --------------------
void loop() {
  }
