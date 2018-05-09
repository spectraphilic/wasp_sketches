
/*  
 SCRIPT to trigger a Time-lapse camera using the UiO Tmie-Lapse v2 shield

Simon Filhol, May 2017, Oslo, Norway

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

//String imageFiles = "IMAGES.TXT";

//================================================================
//================================================================

char message2log[100];
char* logfile = "LOG.TXT";
//================================================================
void setup(){

	SD.ON();
	RTC.ON();
	USB.ON();

	// Create log file on SD card:
	SD.create(logfile);
	USB.print(logfile);
	USB.println(" created");

	pinMode(focusPin, OUTPUT);
	digitalWrite(focusPin, LOW);

	pinMode(shutterPin, OUTPUT);
	digitalWrite(shutterPin, LOW);

	pinMode(onoffPin, OUTPUT);
	digitalWrite(onoffPin, LOW);

	pinMode(camPowerPin, OUTPUT);
	digitalWrite(camPowerPin, HIGH); // Provide power to camera
	delay(10000);

	RTC.setTime("00:01:01:01:00:00:00");

	camON();
}

//================================================================
void loop(){

}

//================================================================
// =========== Local functions
//================================================================


void camON(){
	// function to turn camera ON
	digitalWrite(onoffPin, HIGH);
	delay(20);
	digitalWrite(onoffPin, LOW);
	USB.println("Camera turned ON");
}


