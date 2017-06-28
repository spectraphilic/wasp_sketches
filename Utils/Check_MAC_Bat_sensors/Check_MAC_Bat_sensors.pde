/*  
 SCRIPT to read MAC address, check battery level, and read sensors

 Simon Filhol, April 2017
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
int batteryLevel;

// define the sampling period in minute MUST BE between 0 and 29 minute
int SamplingPeriod = 1;

String targetUnsentFile;
String archiveFile;

char macHigh[10];
char macLow[10];

void setup()
{
  USB.ON();
  USB.println("Wasp started");
  // Flags to turn USB print, OTA programming ON or OFF
  UIO.USB_output = 1;   // turn print to USB ON/OFF
  targetUnsentFile = UIO.unsent_fileA;
  delay(100);

  // Turn on the sensor board
  SensorAgrv20.ON();
  USB.println("Agri board on");

  RTC.ON();
  xbeeDM.ON();
  delay(50);
  
  // Function to initialize SD card
  UIO.initSD();
  USB.println("SD initialized");

  // Function to initialize
  UIO.initNet(NETWORK_FINSE);
  USB.println("Network intialized");
  delay(100);

  xbeeDM.OFF();
  USB.OFF();
  RTC.OFF();
}


void loop()
{

  // set whole agri board and waspmote to sleep. Wake up every minute to
  SensorAgrv20.sleepAgr("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_OFF);

  // extract RTC time
  RTC.ON();
  RTC.getTime();
  minutes = RTC.minute;
  hours = RTC.hour;
  RTC.OFF();

  //Check RTC interruption
  if(intFlag & RTC_INT)
  {
    UIO.start_RTC_SD_USB();
    UIO.logActivity("+ RTC interruption +");
    Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    // Sample every sampling period minutes
    if ((minutes%SamplingPeriod)==0)
    { 
      sprintf(message, "+ %d min Sampling +", SamplingPeriod);

      /////////////////////////////////
      //1. Print Mac address
      /////////////////////////////////

      USB.println("==================");
      USB.println("Read own Mac address...");
      xbeeDM.ON();
      xbeeDM.getOwnMac();
      // convert mac address from array to string
      Utils.hex2str(xbeeDM.sourceMacHigh, macHigh);
      Utils.hex2str(xbeeDM.sourceMacLow, macLow);

      USB.print("My mac address is:");
      USB.print(macHigh);
      USB.println(macLow);
      xbeeDM.OFF();
      USB.println("==================");
      delay(2000);

      /////////////////////////////////
      //2. Print Battery level and voltage
      /////////////////////////////////


      // Show the remaining battery level
      USB.print(F("Battery Level: "));
      USB.print(PWR.getBatteryLevel(),DEC);
      USB.print(F(" %"));
      
      // Show the battery Volts
      USB.print(F(" | Battery (Volts): "));
      USB.print(PWR.getBatteryVolts());
      USB.println(F(" V"));
      USB.println("==================");
      
      delay(2000);

      /////////////////////////////////
      //3. Test sensors
      /////////////////////////////////

      // Measure sensors
      RTC.ON();
      SD.ON();
      USB.ON();
      UIO.measureAgriSensorsBasicSet();
      USB.println("sensor read");
      USB.println("==================");
           
    }
 
  // Clear flag
  intFlag &= ~(RTC_INT); 
  UIO.stop_RTC_SD_USB();

  clearIntFlag(); 
  PWR.clearInterruptionPin();
  }
}


