
/*  
 SCRIPT to trigger a Time-lapse camera using the UiO Tmie-Lapse v2 shield

Simon Filhol, May 2018, Oslo, Norway

Designed for Time-Lapse shield v2 attached to a Sony QX1

USE:
  - set the time step in minutes with variable timeStep. Default is 10 min 
  - download filename from SD Card to file



Further Development:
  - Include waspmoote to wireless network, send frame for confirming Picture taken
  - Sync time with Network GPS 

  - include sunrise sunset time switch with the library:
  https://github.com/chaeplin/Sunrise
    - add system to read battery voltage to make sure the battery load does not get below a threshold
 */


//================================================================
// Put your libraries here (#include ...)


//================================================================
// Define pin
const int focusPin = DIGITAL3;
const int shutterPin = DIGITAL2;
const int onoffPin = DIGITAL4;
const int camPowerPin = DIGITAL1;


//================================================================
//========== SET time step !! ====================================
//================================================================


//================================================================
//================================================================

char message2log[100];

//================================================================
void setup(){

  USB.ON();
  RTC.ON();  

  pinMode(focusPin, OUTPUT);
  digitalWrite(focusPin, LOW);

  pinMode(shutterPin, OUTPUT);
  digitalWrite(shutterPin, LOW);

  pinMode(onoffPin, OUTPUT);
  digitalWrite(onoffPin, LOW);

  pinMode(camPowerPin, OUTPUT);
  digitalWrite(camPowerPin, HIGH);
  delay(1000);
    

  camON();
  delay(120000);
  camOFF();
}

//================================================================
void loop(){
  USB.println(F("Mote to sleep ..."));

  // every hour
  PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);

  if (intFlag & RTC_INT){
    intFlag &= ~(RTC_INT); // Clear flag

    takePicture();
    // RTC captured

  }
}

//================================================================
// =========== Local functions
//================================================================

void takePicture(){
    camON();
    camTrigger();
    camOFF();
}

void camON(){
  // function to turn camera ON
  digitalWrite(onoffPin, HIGH);
  delay(100);
  digitalWrite(onoffPin, LOW);
  delay(8000);
}

void camOFF(){
  // function to turn camera OFF
  digitalWrite(onoffPin, HIGH);
  delay(60);
  digitalWrite(onoffPin, LOW);
}

void camTrigger(){
  // function  trigger the camera shutter
  digitalWrite(shutterPin, HIGH); 
  delay(150);
  digitalWrite(shutterPin, LOW);  
  delay(1000);
}

