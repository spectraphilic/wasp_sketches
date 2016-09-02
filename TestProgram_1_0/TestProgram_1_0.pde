

/*  
 *  ------ First test program to understand and learn how to use WaspMote -------- 
 *  ------ By: John Hulth (john.hulth@geo.uio.no
 */


//////////////////////////////////////////
// Included Waspmote Libraries
//////////////////////////////////////////
#include <WaspXBeeDM.h>
#include <WaspFrame.h>
#include <WaspGPS.h>
#include <WaspSensorCities.h>
#include <WaspStackEEPROM.h>

//////////////////////////////////////////
// User selectable variables
//////////////////////////////////////////

// Define the Waspmote ID ("Mote_1")
//char WASPMOTE_ID[] = "Mote_1";

// Define base station or gateway MAC adress ("0013A20040db6048")
char RX_ADDRESS[] = "0013A20040db6048";

// define GPS timeout when connecting to satellites, this time is defined in seconds (240sec = 4minutes)
#define GPS_TIMEOUT 240



//////////////////////////////////////////
// Global variables declaration
//////////////////////////////////////////

// define variable
uint8_t tx_error; // Xbee send error
unsigned long id; // WaspMote serial ID
bool gps_status; // define status variable for GPS connection



//////////////////////////////////////////
// Module initialization after start or reset
//////////////////////////////////////////
void setup()
{
  // Blink red led fast 3 times at startup  
  Utils.blinkRedLED(200, 3);

  // init USB, SD, RTC, GPS, xbeeDM etc...
  USB.ON();
  SD.ON();
  RTC.ON();
  GPS.ON(); 
  xbeeDM.ON();

  // Reading the serial number
  id = Utils.readSerialID();
  USB.print(F("Waspmote serial ID: "));
  USB.println(id);

  // Reading the xbee Mac adress if present
  xbeeDM.getOwnMacHigh(); // Get 32 upper bits of MAC Address  
  xbeeDM.getOwnMacLow(); // Get 32 lower bits of MAC Address
  if (xbeeDM.error_AT == 0)
  {
    USB.print(F("WaspMote MAC:       "));
    USB.printHex(xbeeDM.sourceMacHigh,4);
    USB.printHexln(xbeeDM.sourceMacLow,4);
  }
  else
  {
    USB.println(F("xbeeDM error!!!"));
  }

  // Reading the base station MAC adress
  USB.print(F("Base station MAC:   "));
  USB.println((RX_ADDRESS));

  // Reading the PAN ID 
  xbeeDM.getPAN();
  USB.print(F("panid:              "));
  USB.printHexln(xbeeDM.PAN_ID,2); 

  // Check SD if present
  if( SD.flag == 0)
  {
    USB.print(F("SD size:            "));
    USB.println(SD.getDiskSize());
  }
  else
  {
    USB.println(F("SD error!!!"));
  }

  // setup the GPS module
  gps_status = GPS.waitForSignal(GPS_TIMEOUT);

  if( gps_status == true )
  {
    USB.println(F("GPS Connected"));

    // set time in RTC from GPS time (GMT time)
    GPS.setTimeFromGPS();
    USB.print(F("Time set by GPS to: "));
    USB.println(RTC.getTime());

    if( GPS.saveEphems() == 1 )
    {
      USB.println(F("Ephemeris stored successfully"));
    }
    else
    {
      USB.println(F("Ephemeris storing failed"));
    }
  }
  else
  {
    USB.println(F("!!!GPS NOT connected GPS TIMEOUT!!!"));
    USB.print(F("Time NOT updated:    "));
  }
  GPS.OFF(); 

  delay(1000); 

  // scan network
  xbeeDM.scanNetwork(); 
  USB.print(F("WaspMotes in the network:"));
  USB.println(xbeeDM.totalScannedBrothers,DEC); 
  printScanInfo(); // Defined function, se the end of program
}

//////////////////////////////////////////
// Start program
//////////////////////////////////////////
void loop()
{
  // create new ASCII frame
  frame.createFrame(ASCII,"");  

  // add frame field Timestamp
  frame.addSensor(SENSOR_TST, RTC.getEpochTime());

  // add frame field TEXT
  // frame.addSensor(SENSOR_STR, "new_sensor_frame");

  // add frame field Battery
  frame.addSensor(SENSOR_BAT, PWR.getBatteryLevel()); 

  // add frame field GPS
  GPS.ON();
  SD.ON();
  if( (GPS.loadEphems(),DEC) == 1 ) 
  {
    USB.println(F("Ephemeris loaded successfully")); 
  }
  else 
  {
    USB.println(F("Ephemeris load failed"));
  }        
  gps_status = GPS.waitForSignal(GPS_TIMEOUT);

  if( gps_status == true )
  {
    frame.addSensor(SENSOR_GPS,
    GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator),
    GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));

    if( GPS.saveEphems() == 1 )
    {
      USB.println(F("Ephemeris stored successfully"));
    }
    else
    {
      USB.println(F("Ephemeris storing failed"));
    }
  }
  else
  {
    frame.addSensor(SENSOR_STR,"NaN NaN");
  }
  GPS.OFF(); 

  //  // add sensors connected to the sensor board
  //  SensorCities.ON(); //Turn on board
  //  SensorCities.setSensorMode(SENS_ON, SENS_CITIES_ULTRASOUND_5V); //Turn on sensor
  //  SensorCities.setSensorMode(SENS_ON, SENS_CITIES_TEMPERATURE); //Turn on sensor
  //  delay(3000);
  //
  //  frame.addSensor(SENSOR_TCA, SensorCities.readValue(SENS_CITIES_TEMPERATURE)); 
  //  frame.addSensor(SENSOR_US,  SensorCities.readValue(SENS_CITIES_ULTRASOUND_5V, SENS_US_WRA1)); 
  //
  //  SensorCities.OFF(); //Turn off board

  ///////////////////////////////////////////
  // 2. Send packet
  ///////////////////////////////////////////  

  // send XBee packet
  tx_error = xbeeDM.send( RX_ADDRESS, frame.buffer, frame.length );   

  // check TX flag
  if( tx_error == 0 )
  {
    frame.showFrame();
  }
  else 
  {
    USB.println(F("send error"));
  }

  // wait for five seconds...
  delay(5000);
}























//////////////////////////////////////////
// Functions
//////////////////////////////////////////




/*
 *  printScanInfo
 *
 *  This function prints all info related to the scan 
 *  process given by the XBee module
 */
void printScanInfo()
{  
  USB.println(F("----------------------------"));

  for(int i=0; i<xbeeDM.totalScannedBrothers; i++)
  {  
    USB.print(F("MAC:"));
    USB.printHex(xbeeDM.scannedBrothers[i].SH,4);	
    USB.printHex(xbeeDM.scannedBrothers[i].SL,4);	

    USB.print(F("\nNI:"));
    USB.print(xbeeDM.scannedBrothers[i].NI);		

    USB.print(F("\nDevice Type:"));
    switch(xbeeDM.scannedBrothers[i].DT)
    {
    case 0: 
      USB.print(F("End Device"));
      break;
    case 1: 
      USB.print(F("Router"));
      break;
    case 2: 
      USB.print(F("Coordinator"));
      break;
    }

    USB.print(F("\nPMY:"));
    USB.printHex(xbeeDM.scannedBrothers[i].PMY,2);	

    USB.print(F("\nPID:"));
    USB.printHex(xbeeDM.scannedBrothers[i].PID,2);	

    USB.print(F("\nMID:"));
    USB.printHex(xbeeDM.scannedBrothers[i].MID,2);	

    USB.println(F("\n----------------------------"));

  }
}































