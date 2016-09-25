/*  
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 14 September 2016, Simon Filhol, John Hulth
 */

// Put your libraries here (#include ...)
#include <WaspXBeeDM.h>
#include <WaspUIO.h>
#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>


char message[40];
int pendingPulses;
int minutes;
int hours;
int randomNumber;

void setup(){

  USB.ON();
  RTC.ON();
  xbeeDM.ON();
  xbeeDM.checkNewProgram(); // CheckNewProgram is mandatory in every OTA program

  // Function to initialize SD card
  UIO.initSD();
  UIO.logActivity("Waspmote starting");

  // Function to initialize
  UIO.initNet('Finse');
  UIO.logActivity("SD and XbeeDM initialized");

  // Attempt to initialize timestamp from GPS mote
  UIO.receiveGPSsyncTime();
  
  // Turn on the sensor board
  SensorAgrv20.ON();
  SensorAgrv20.attachPluvioInt();
  delay(2000);

  // set random seed
  srandom(42);

  UIO.logActivity("Waspmote all set and ready");
  UIO.readBatteryLevel();
  
  xbeeDM.OFF();
  USB.OFF();
  RTC.OFF();
}


void loop(){

  SensorAgrv20.sleepAgr("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, SOCKET0_OFF, SENS_AGR_PLUVIOMETER);
  SensorAgrv20.detachPluvioInt(); 

  RTC.ON();
  RTC.getTime();
  minutes = RTC.minute;
  hours = RTC.hour;
  RTC.OFF();


  //If statement to record pluviometer interruptions
  if( intFlag & PLV_INT)
  {	

    UIO.start_RTC_SD_USB();
    UIO.logActivity("+++ PLV interruption +++");
    pendingPulses = intArray[PLV_POS];
    sprintf(message, "Number of pending pulses: %d",pendingPulses);
    UIO.logActivity(message);

    for(int i=0 ; i<pendingPulses; i++)
    {
      // Enter pulse information inside class structure
      SensorAgrv20.storePulse();

      // decrease number of pulses
      intArray[PLV_POS]--;
    }

    // Clear flag
    intFlag &= ~(PLV_INT); 
    UIO.stop_RTC_SD_USB();
  }

  //Check RTC interruption
  if(intFlag & RTC_INT)
  {
    UIO.start_RTC_SD_USB();
    UIO.logActivity("+++ RTC interruption +++");
    Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    // Sample every 10 minutes
    if ((minutes%10)==0)
    { 

      // Measure sensors
      UIO.logActivity("+++ Sampling +++");
      UIO.measureSensorsBasicSet();

      if(minutes == 0) 
      {
        // send frame to Meshlium every hours
        xbeeDM.ON();
        delay(50);
        
        // delay sending of frame by a random time within 0 to 100 ms to avoid jaming the network
        randomNumber = rand()%100;
        delay(randomNumber);

        UIO.Frame2Meshlium();
        UIO.logActivity("+++ Frame to Meshlium passed +++");

        if(hours == 12){
          UIO.receiveGPSsyncTime();
          UIO.logActivity("+++ Sync GPS time passed +++");
          delay(3*60000);  // daily delay of 3min for passing frame from station with large anount of frame
        }
        delay(30000); // leave xbee on for 30 second, making sure it synchronizes with other motes
        xbeeDM.OFF();
      }
    }

    // Clear flag
    intFlag &= ~(RTC_INT); 
    UIO.stop_RTC_SD_USB();

    clearIntFlag(); 
    PWR.clearInterruptionPin();
  }
  UIO.stop_RTC_SD_USB();
}

