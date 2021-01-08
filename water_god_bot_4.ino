#include <Metro.h>               // Include the Metro library header
#include <Tsunami.h>             // Include the Tsunami library header
#include <AltSoftSerial.h>       // Optional for using AltSoftSerial
#include <Wire.h>                // i2c library
#include <SoftwareWire.h> 
#include <Adafruit_MPR121.h>     // capacitive touch board communication library (uses Wire library)
#include <Adafruit_MPR121_SW2.h> // capacitive touch board communication library (uses SoftwareWire library)
#include <TimerOne.h>            // TimerOne Interrupt Library (Arduino Mega Pins: 11,12,13)

#define LED 13                   // our LED

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

Tsunami tsunami;                // Our Tsunami object

Metro gLedMetro(500);           // LED blink interval timer
Metro gSeqMetro(6000);          // Sequencer state machine interval timer

byte gLedState = 0;             // LED State
int  gSeqState = 0;             // Main program sequencer state
int  gRateOffset = 0;           // Tsunami sample-rate offset
int  gNumTracks;                // Number of tracks on SD card

char gTsunamiVersion[VERSION_STRING_LEN];    // Tsunami version string

// Wire i2c bus (20,21)
Adafruit_MPR121 cap = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();
Adafruit_MPR121 cap3 = Adafruit_MPR121();
Adafruit_MPR121 cap4 = Adafruit_MPR121();
// SoftwareWire i2c bus (18,19)
Adafruit_MPR121_SW2 cap5 = Adafruit_MPR121_SW2();
Adafruit_MPR121_SW2 cap6 = Adafruit_MPR121_SW2();
Adafruit_MPR121_SW2 cap7 = Adafruit_MPR121_SW2();
Adafruit_MPR121_SW2 cap8 = Adafruit_MPR121_SW2();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

uint16_t lasttouched2 = 0;
uint16_t currtouched2 = 0;

uint16_t lasttouched3 = 0;
uint16_t currtouched3 = 0;

uint16_t lasttouched4 = 0;
uint16_t currtouched4 = 0;

uint16_t lasttouched5 = 0;
uint16_t currtouched5 = 0;

uint16_t lasttouched6 = 0;
uint16_t currtouched6 = 0;

uint16_t lasttouched7 = 0;
uint16_t currtouched7 = 0;

uint16_t lasttouched8 = 0;
uint16_t currtouched8 = 0;

int delay_amount = 100;
int master_vol_out = -6;  // range -70 to +4
int master_gain    = -6;  // range -70 to +4
int note_vol       = 0;   // range -70 to +4
int sample_vol     = -6; // range -70 to +4

int sampleNumStart = 74;
int sampleNumEnd   = 120;
int nonActiveTrack = 121;

// Timers
volatile unsigned long inactiveTime = 0;
volatile unsigned long activeTime = 0;
volatile unsigned long timeSinceLastTouch = 0;
int resetActivityStateAmt = 10; // Seconds of no activity before resetting activity state to false
int playNonActiveSampleAmt = 24;
boolean activity = false;

void incrementTimer1(void) {
  if (activity) {
    activeTime = activeTime + 1;
    inactiveTime = 0;
  } else {
    inactiveTime = inactiveTime + 1;
    activeTime = 0;
  }
  timeSinceLastTouch = timeSinceLastTouch + 1;
  // Log Timers
  Serial.println((String)"InActive Time: "+ inactiveTime + " seconds");
  Serial.println((String)"Active Time: "+ activeTime + " seconds");
  Serial.println((String)"Time Since Last Touch: "+ timeSinceLastTouch + " seconds");
}

void setup() {
  // Serial monitor
  Serial.begin(9600);

  // Initialize Timer
  Timer1.initialize(1000000); // Timer period is 1000000 microseconds = 1 second
  // Attach Interrupt
  Timer1.attachInterrupt(incrementTimer1);

  // Initialize the LED pin
  pinMode(LED,OUTPUT);
  digitalWrite(LED,gLedState);

  // We should wait for the Tsunami to finish reset before trying to send
  // commands.
  delay(1000);

  // Tsunami startup at 57600
  tsunami.start();
  delay(10);
  
  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the Tsunami was already playing.
  tsunami.stopAllTracks();
  tsunami.samplerateOffset(0, 0);
  
  // Enable track reporting from the Tsunami
  tsunami.setReporting(true);
  
  // Allow time for the Tsunami to respond with the version string and
  //  number of tracks.
  delay(100); 

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
    
  }
  
  if (cap.begin(0x5A)) {
    Serial.println("A yes");
  } else {
    Serial.println("A no");
  }

  if (cap2.begin(0x5B)) {
    Serial.println("B yes");
  } else {
    Serial.println("B no");
  }

  if (cap3.begin(0x5C)) {
    Serial.println("C yes");
  } else {
    Serial.println("C no");
  }

  if (cap4.begin(0x5D)) {
    Serial.println("D yes");
  } else {
    Serial.println("D no");
  }

  if (cap5.begin(0x5A)) {
    Serial.println("E yes");
  } else {
    Serial.println("E no");
  }

  if (cap6.begin(0x5B)) {
    Serial.println("F yes");
  } else {
    Serial.println("F no");
  }

  if (cap7.begin(0x5C)) {
    Serial.println("G yes");
  } else {
    Serial.println("G no");
  }

  if (cap8.begin(0x5D)) {
    Serial.println("H yes");
  } else {
    Serial.println("H no");
  }

  // Stop mode to adjust settings
  //cap.writeRegister(MPR121_ECR, 0x00);
  // Adjust sensitivity
  //cap.setThresholds(0x0D, 0x05); // Defaults are 0x12, 0x06 
  // Back in run mode with new settings
  //cap.writeRegister(MPR121_ECR, 0x8F);

  tsunami.masterGain(master_vol_out, master_gain); // (out, gain) range -70 to +4
  tsunami.trackGain(nonActiveTrack, note_vol);
  
}

void loop() {

  // Check if non activity has exceeded reset amount
  if (timeSinceLastTouch > resetActivityStateAmt) {
    activity = false;
  }

  if (timeSinceLastTouch > playNonActiveSampleAmt && !(tsunami.isTrackPlaying(nonActiveTrack))) {
     tsunami.trackLoop(nonActiveTrack, true);
     tsunami.trackPlayPoly(nonActiveTrack, 0, true);
     Serial.println((String)"Playing Inactive Sample...");
     delay(delay_amount);
   }
  
  // Get the currently touched pads
  currtouched  = cap.touched();
  currtouched2 = cap2.touched();
  currtouched3 = cap3.touched();
  currtouched4 = cap4.touched();
  currtouched5 = cap5.touched();
  currtouched6 = cap6.touched();
  currtouched7 = cap7.touched();
  currtouched8 = cap8.touched();

  for (uint8_t i=0; i<12; i++) {
    /////////////////////////// A ///////////////////////////
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      int t = random(sampleNumStart, sampleNumEnd);
      tsunami.trackLoop(nonActiveTrack, false);
      tsunami.trackStop(nonActiveTrack);
      tsunami.trackGain(i+1, note_vol);
      tsunami.trackGain(t, sample_vol);
      tsunami.trackPlayPoly(i + 1, 0, true);
      tsunami.trackPlayPoly(t, 0, true); 
      // Update activity state & reset time since last touched
      activity = true;
      timeSinceLastTouch = 0;
      Serial.println((String)"A: "+(i+1));
      delay(delay_amount); 
    }
    // if it *was* touched and now *isnt*, alert!
//    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
//      //Serial.print(i + 1); Serial.println(" A released");
//    }
    /////////////////////////// B ///////////////////////////
    if ((currtouched2 & _BV(i)) && !(lasttouched2 & _BV(i)) ) {
      int t = random(sampleNumStart, sampleNumEnd);
      tsunami.trackLoop(nonActiveTrack, false);
      tsunami.trackStop(nonActiveTrack);
      tsunami.trackGain(i+13, note_vol);
      tsunami.trackGain(t, sample_vol);
      tsunami.trackPlayPoly(i + 13, 0, true);
      tsunami.trackPlayPoly(t, 0, true);
      // Update activity state & reset time since last touched
      activity = true;
      timeSinceLastTouch = 0;
      Serial.println((String)"B: "+(i+1)); 
      delay(delay_amount);  
    }
    // if it *was* touched and now *isnt*, alert!
//    if (!(currtouched2 & _BV(i)) && (lasttouched2 & _BV(i)) ) {
//      //Serial.print(i + 13); Serial.println(" B released"); 
//    }
    /////////////////////////// C ///////////////////////////
    if ((currtouched3 & _BV(i)) && !(lasttouched3 & _BV(i)) ) {
      int t = random(sampleNumStart, sampleNumEnd);
      tsunami.trackLoop(nonActiveTrack, false);
      tsunami.trackStop(nonActiveTrack);
      tsunami.trackGain(i+26, note_vol);
      tsunami.trackGain(t, sample_vol);
      tsunami.trackPlayPoly(i + 26, 0, true);
      tsunami.trackPlayPoly(t, 0, true);
      // Update activity state & reset time since last touched
      activity = true;
      timeSinceLastTouch = 0;
      Serial.println((String)"C: "+(i+1)); 
      delay(delay_amount);
    }
    // if it *was* touched and now *isnt*, alert!
//    if (!(currtouched3 & _BV(i)) && (lasttouched3 & _BV(i)) ) {
//      //Serial.print(i + 26); Serial.println(" C released");
//    }
    /////////////////////////// D ///////////////////////////
    if ((currtouched4 & _BV(i)) && !(lasttouched4 & _BV(i)) ) {
      int t = random(sampleNumStart, sampleNumEnd);
      tsunami.trackLoop(nonActiveTrack, false);
      tsunami.trackStop(nonActiveTrack);
      tsunami.trackGain(i+37, note_vol);
      tsunami.trackGain(t, sample_vol);
      tsunami.trackPlayPoly(i + 37, 0, true);
      tsunami.trackPlayPoly(t, 0, true);
      // Update activity state & reset time since last touched
      activity = true;
      timeSinceLastTouch = 0;
      Serial.println((String)"D: "+(i+1)); 
      delay(delay_amount); 
    }
    // if it *was* touched and now *isnt*, alert!
//    if (!(currtouched4 & _BV(i)) && (lasttouched4 & _BV(i)) ) {
//      //Serial.print(i + 37); Serial.println(" D released");
//    }
    /////////////////////////// E ///////////////////////////
    if ((currtouched5 & _BV(i)) && !(lasttouched5 & _BV(i)) ) {
      int t = random(sampleNumStart, sampleNumEnd);
      tsunami.trackLoop(nonActiveTrack, false);
      tsunami.trackStop(nonActiveTrack);
      tsunami.trackGain(i+49, note_vol);
      tsunami.trackGain(t, sample_vol);
      tsunami.trackPlayPoly(i + 49, 0, true);
      tsunami.trackPlayPoly(t, 0, true);
      // Update activity state & reset time since last touched
      activity = true;
      timeSinceLastTouch = 0;
      Serial.println((String)"E: "+(i+1)); 
      delay(delay_amount); 
    }
    // if it *was* touched and now *isnt*, alert!
//    if (!(currtouched5 & _BV(i)) && (lasttouched5 & _BV(i)) ) {
//      //Serial.print(i + 49); Serial.println(" E released");
//    }
    /////////////////////////// F ///////////////////////////
    if ((currtouched6 & _BV(i)) && !(lasttouched6 & _BV(i)) ) {
      int t = random(sampleNumStart, sampleNumEnd);
      tsunami.trackLoop(nonActiveTrack, false);
      tsunami.trackStop(nonActiveTrack);
      tsunami.trackGain(i+61, note_vol);
      tsunami.trackGain(t, sample_vol);
      tsunami.trackPlayPoly(i + 61, 0, true);
      tsunami.trackPlayPoly(t, 0, true);
      // Update activity state & reset time since last touched
      activity = true;
      timeSinceLastTouch = 0;
      Serial.println((String)"F: "+(i+1)); 
      delay(delay_amount); 
    }
    // if it *was* touched and now *isnt*, alert!
//    if (!(currtouched6 & _BV(i)) && (lasttouched6 & _BV(i)) ) {
//      //Serial.print(i + 61); Serial.println(" F released");
//    }
    /////////////////////////// G ///////////////////////////
    if ((currtouched7 & _BV(i)) && !(lasttouched7 & _BV(i)) ) {
      int t = random(sampleNumStart, sampleNumEnd);
      tsunami.trackLoop(nonActiveTrack, false);
      tsunami.trackStop(nonActiveTrack);
      tsunami.trackGain(i+73, note_vol);
      tsunami.trackGain(t, sample_vol);
      tsunami.trackPlayPoly(i + 73, 0, true);
      tsunami.trackPlayPoly(t, 0, true);
      // Update activity state & reset time since last touched
      activity = true;
      timeSinceLastTouch = 0;
      Serial.println((String)"G: "+(i+1)); 
      delay(delay_amount); 
    }
    // if it *was* touched and now *isnt*, alert!
//    if (!(currtouched7 & _BV(i)) && (lasttouched7 & _BV(i)) ) {
//      //Serial.print(i + 73); Serial.println(" G released");
//    }
    /////////////////////////// H ///////////////////////////
    if ((currtouched8 & _BV(i)) && !(lasttouched8 & _BV(i)) ) {
      int t = random(sampleNumStart, sampleNumEnd);
      tsunami.trackLoop(nonActiveTrack, false);
      tsunami.trackStop(nonActiveTrack);
      tsunami.trackGain(i+85, note_vol);
      tsunami.trackGain(t, sample_vol);
      tsunami.trackPlayPoly(i + 85, 0, true);
      tsunami.trackPlayPoly(t, 0, true);
      // Update activity state & reset time since last touched
      activity = true;
      timeSinceLastTouch = 0;
      Serial.println((String)"H: "+(i+1));  
      delay(delay_amount);
    }
    // if it *was* touched and now *isnt*, alert!
//    if (!(currtouched8 & _BV(i)) && (lasttouched8 & _BV(i)) ) {
//      //Serial.print(i + 85); Serial.println(" H released");
//    }
  }

  // reset our state
  lasttouched = currtouched;
  lasttouched2 = currtouched2;
  lasttouched3 = currtouched3;
  lasttouched4 = currtouched4;
  lasttouched5 = currtouched5;
  lasttouched6 = currtouched6;
  lasttouched7 = currtouched7;
  lasttouched8 = currtouched8;

  // comment out this line for detailed data from the sensor!
  return;
  
  // debugging info, what
    Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
    Serial.print("Filt: ");
    for (uint8_t i=0; i<12; i++) {
      Serial.print(cap.filteredData(i)); Serial.print("\t");
    }
    Serial.println();
    Serial.print("Base: ");
    for (uint8_t i=0; i<12; i++) {
      Serial.print(cap.baselineData(i)); Serial.print("\t");
    }
    Serial.println();
    
    // put a delay so it isn't overwhelming
    delay(100);

}
