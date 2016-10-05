/*  
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 14 September 2016, Simon Filhol, John Hulth

Script description:
  - sample sensors every 10 minutes
	- OTA once a week (Tuesday 12pm UTC = 2pm Oslo) if battery and connection to Meshlium exist. OTA accessible for 4 min
	- send data to Meshlium every hour if battery > 60% otherwise only every 6hours
	- Try sendind nusent frames only once a day. If not able to send data for the last 20000 frames, move frames to permanent storage on SD
  - Update time from GPS unit once a day
  - If battery power is less than 30%, waspmote only sample from sensors, save data to SD and nothing else.

 */

// Put your libraries here (#include ...)
#include <WaspXBeeDM.h>
#include <WaspUIO.h>
#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>

// Define local variables:
char message[40];
int pendingPulses;
int minutes;
int hours;
int randomNumber;
uint8_t USB_output = 0;   // pass to 1 for printing info to serial


void setup()
{
  USB.ON();
  RTC.ON();
  xbeeDM.ON();
  xbeeDM.checkNewProgram(); // CheckNewProgram is mandatory in every OTA program

  // Function to initialize SD card
  UIO.initSD();
  UIO.logActivity("Waspmote starting");

  // Function to initialize
  UIO.initNet('Finse');
  UIO.logActivity("SD,XbeeDM initialized");

  // Attempt to initialize timestamp from GPS mote
  UIO.receiveGPSsyncTime();
  
  // Turn on the sensor board
  SensorAgrv20.ON();
  SensorAgrv20.attachPluvioInt();
  delay(2000);

  // set random seed
  srandom(42);

  UIO.logActivity("Waspmote set and ready");
  UIO.readBatteryLevel();
  
  xbeeDM.OFF();
  USB.OFF();
  RTC.OFF();
}


void loop()
{

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
    UIO.start_RTC_SD_USB(USB_output);
    UIO.logActivity("+ PLV interruption +");
    pendingPulses = intArray[PLV_POS];

    for(int i=0 ; i<pendingPulses; i++)
    {
      // Enter pulse information inside class structure
      SensorAgrv20.storePulse();

      // decrease number of pulses
      intArray[PLV_POS]--;
    }

    // Clear flag
    intFlag &= ~(PLV_INT); 
    UIO.stop_RTC_SD_USB(USB_output);
  }

  //Check RTC interruption
  if(intFlag & RTC_INT)
  {
    UIO.start_RTC_SD_USB(USB_output);
    UIO.logActivity("+ RTC interruption +");
    Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    // Sample every 10 minutes
    if ((minutes%10)==0)
    { 
      // Measure sensors
      UIO.logActivity("+ Sampling +");
      UIO.measureSensorsBasicSet(USB_output);
      
      if(PWR.getBatteryLevel()>60)
      {
        if(minutes == 0) 
        {
          // send frame to Meshlium every hours
          xbeeDM.ON();
          delay(50);
          
          // delay sending of frame by a random time within 0 to 100 ms to avoid jaming the network
          randomNumber = rand()%500;
          delay(randomNumber);

          UIO.Frame2Meshlium(USB_output);
          UIO.logActivity("Frame2Meshlium passed");

          if(hours == 12){

            // send once a day the unsent frame (if unsent_file is less than 20000 lines otherwise save frames to SD permanently)
            UIO.manage_unsent_data();
            UIO.Frame2Meshlium(USB_output);

            UIO.receiveGPSsyncTime();
            UIO.logActivity("Sync GPS time passed");
            delay(3*60000);  // daily delay of 3min for passing frame from station with large anount of frame
          	
          	// Allow for OTA connection on tuesdays at about 12:03 after GPS time synchronization and sending data out
          	if(RTC.day == 3)
          	{
          		UIO.OTA_communication(4); // function to open OTA for a 4minute weekly window on tuesday
              UIO.delLogActivity();
          	}
          }
          delay(30000); // leave xbee on for 30 second, making sure it synchronizes with other motes
          xbeeDM.OFF();
        }
      }
      else
      {
        if(PWR.getBatteryLevel()>30)
        {
          if(hours%6 == 0) 
          {
            // send frame to Meshlium every 6 hours
            xbeeDM.ON();
            delay(50);

            // delay sending of frame by a random time within 0 to 100 ms to avoid jaming the network
            randomNumber = rand()%100;
            delay(randomNumber);

            UIO.Frame2Meshlium(USB_output);
            UIO.logActivity("Frame2Meshlium passed");

            if(hours == 12)
            {
              UIO.receiveGPSsyncTime();
              UIO.logActivity("Sync GPS time passed");
              delay(1*60000);  // daily delay of 1min for passing frame from station with large anount of frame
            }
              delay(10000); // leave xbee on for 10 second, making sure it synchronizes with other motes
              xbeeDM.OFF();
            }
          }
          else
          {
          // Battery level less than 30% so do not send data over network
          // Do not synchronize time from GPS
            UIO.logActivity("+ Low Battery +");
          }
        }
      }
    // Clear flag
      intFlag &= ~(RTC_INT); 
      UIO.stop_RTC_SD_USB(USB_output);

      clearIntFlag(); 
      PWR.clearInterruptionPin();
    }
    UIO.stop_RTC_SD_USB(USB_output);
  }

