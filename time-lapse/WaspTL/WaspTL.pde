
/*  
 SCRIPT to trigger a Time-lapse camera using the UiO Tmie-Lapse v1 shield

Simon Filhol, January 2016, Oslo, Norway

Designed for Time-Lapse shield v1 attached to a Sony QX1

USE:
	set the time step in minutes with variable timeStep. Default is 10 min 


Further Development:
	- Include waspnmtoe to wireless network, send frame for confirming Picture taken
	- Sync time with Network GPS 

	- include sunrise sunset time switch with the library:
	https://github.com/chaeplin/Sunrise
 */


//================================================================
// Put your libraries here (#include ...)
#include <WaspXBeeDM.h>
#include <WaspUIO.h>

//================================================================
// Define pin
const int focusPin = DIGITAL3;
const int shutterPin = DIGITAL2;
const int onoffPin = DIGITAL4;
const int camPowerPin = DIGITAL1;


//================================================================
//========== SET time step !! ====================================
//================================================================

int timeStep = 10; // time step in minute 

//================================================================
//================================================================

char message2log[100];

//================================================================
void setup(){

	UIO.start_RTC_SD_USB();

	pinMode(focusPin, OUTPUT);
	digitalWrite(focusPin, LOW);

	pinMode(shutterPin, OUTPUT);
	digitalWrite(shutterPin, LOW);

	pinMode(onoffPin, OUTPUT);
	digitalWrite(onoffPin, LOW);

	pinMode(camPowerPin, OUTPUT);
	digitalWrite(camPowerPin, LOW);

	UIO.initSD();
	UIO.logActivity("=== Waspmote starting ===");

	RTC.setAlarm2(0,0,timeStep, RTC_OFFSET, RTC_ALM2_MODE4);

	sprintf(message2log, "Alarm set to %d min", timeStep);
	UIO.logActivity(message2log);
}

//================================================================
void loop(){
	USB.println(F("Mote to sleep ..."));
	UIO.logActivity("Mote to sleep ...");
	PWR.sleep(ALL_OFF);

	UIO.start_RTC_SD_USB();
	USB.println(F("Mote awaking!!"));
	UIO.logActivity("Mote awaking");
	USB.print(F("Time: "));
	USB.println(RTC.getTime());


	if (intFlag & RTC_INT){
		intFlag &= ~(RTC_INT); // Clear flag

		camPowerON();
	    camON();
	    camTrigger();
	    camOFF();
	    camPowerOFF();
		// RTC captured

		if(RTC.alarmTriggered == 2){
			RTC.setAlarm2(0,0,timeStep, RTC_OFFSET, RTC_ALM2_MODE4);
		}
	}
}

//================================================================
// =========== Local functions
//================================================================

void camPowerON(){
	// function to switch power to camera
	digitalWrite(camPowerPin, HIGH);
	delay(50);
	USB.println("Camera powered");
	UIO.logActivity("Cam powered");
}

void camPowerOFF(){
	// function to switch power off from camera
	digitalWrite(camPowerPin, LOW);
	USB.println("Camera unpowered");
	UIO.logActivity("Cam unpowered");
}

void camON(){
	// function to turn camera ON
	digitalWrite(onoffPin, HIGH);
	delay(20);
	digitalWrite(onoffPin, LOW);
	USB.println("Camera turned ON");
	UIO.logActivity("Cam ON");
}

void camOFF(){
	// function to turn camera OFF
	digitalWrite(onoffPin, HIGH);
	delay(20);
	digitalWrite(onoffPin, LOW);
	USB.println("Camera turned OFF");
	UIO.logActivity("Cam OFF");
}

void camTrigger(){
	// function to focus, and trigger the camera shutter
	digitalWrite(focusPin, HIGH);	
	delay(50);
	digitalWrite(shutterPin, HIGH);	
	delay(50);
	digitalWrite(shutterPin, LOW);	
	digitalWrite(focusPin, LOW);	
	delay(100);
	USB.println("Photo captured");
	UIO.logActivity("Photo captured");
}