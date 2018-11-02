/*
    ------ Waspmote Pro Code Example --------

    Explanation: This is the basic Code for Waspmote Pro

    Copyright (C) 2016 Libelium Comunicaciones Distribuidas S.L.
    http://www.libelium.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



////////////////////////////////////////////////////////
// include ephem library to control light
// include onewire library for DS18b20 thermistors and soil moisture


////////////////////////////////////////////////////////
// Declare variables

const int focusPin = DIGITAL3;
const int shutterPin = DIGITAL2;
const int onoffPin = DIGITAL4;
const int camPowerPin = DIGITAL1;



////////////////////////////////////////////////////////
void setup()
{
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
  delay(10000);
    

  camON();
  delay(120000);
  camOFF();
  
  // Set time
  
  // check if log file exists, if not create one
  
  // check if DATA folder exists, if not create one
  
  // Find out OneWire device addresses and write them down in a file




}

////////////////////////////////////////////////////////
void loop()
{
 USB.println(F("Mote to sleep ..."));

  // every hour
  PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);

  if (intFlag & RTC_INT){
    intFlag &= ~(RTC_INT); // Clear flag

    takePicture();
    // RTC captured

  }





}




////////////////////////////////////////////////////////



// Function to trigger light
void lightON(){}


// Function to write data to SD
void data2SD(){}


// Function to log activities


// Function to read Onewire sensors (thermistors and soil moisture)


// Function to turn Cam ON/OFF
void camON(){
  // function to turn camera ON
  digitalWrite(onoffPin, HIGH);
  delay(600);
  digitalWrite(onoffPin, LOW);
  delay(8000);
}

void camOFF(){
  // function to turn camera OFF
  digitalWrite(onoffPin, HIGH);
  delay(60);
  digitalWrite(onoffPin, LOW);
}

// Function to trigger Cam shutter
void camTrigger(){
  // function  trigger the camera shutter
  digitalWrite(shutterPin, HIGH); 
  delay(150);
  digitalWrite(shutterPin, LOW);  
  delay(1000);
}


void takePicture(){
    camON();
    camTrigger();
    camOFF();
}





// Function to decide alarm mode based on sensor readings
  // alarm 1: once a day at midday
  // alarm 2: once a hour if (Ttop <= 1 degC) or (Tbot <= 1 degC)
  // if sensor reading ERROR, go to alarm 2



