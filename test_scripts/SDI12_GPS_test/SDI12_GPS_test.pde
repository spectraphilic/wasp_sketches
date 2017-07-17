#include <WaspGPS.h>
#include <SDI12.h>

// GPS
#define TIMEOUT 240

float Lat;
float Lon;

// SDI-12
#define DATAPIN 6         // change to the proper pin (JH) 6 = DIGITAL 6 on Waspmote
SDI12 mySDI12(DATAPIN);


void setup()
{
  // open USB port
  USB.ON();
  USB.println(F("Starting"));

  syncRTCtoGPS();
  //USB.println(getLatitudeGPS());
  //USB.println(getLongitudeGPS());
  
  PWR.setSensorPower(SENS_5V, SENS_ON); 
  delay(500); // Sensor exitation delay

  mySDI12.begin();
  // -'-'-'-'-'-'-INFO COMMAND-'-'-'-'-'-'-
  mySDI12.sendCommand("0I!");
  SDIdata();
  USB.println(F("----------------------"));
}

void loop()
{
  // Goes to sleep... and wake up.
  PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_OFF);

  USB.ON();
  RTC.ON();
  // Clear alarms (interuptions)
  if (intFlag & RTC_INT)
  {
    intFlag &= ~(RTC_INT); // Clear flag
  }

  
  // Sample data from SDI-12 sensor
  PWR.setSensorPower(SENS_5V, SENS_ON); // (JH)
  delay(500); // Sensor exitation delay

  mySDI12.begin();
  // -'-'-'-'-MEASUREMENT COMMAND-'-'-'-'-
  mySDI12.sendCommand("0M!");
  SDIdata();

  // -'-'-'-'-'-'-DATA COMMAND-'-'-'-'-'-'-
  mySDI12.sendCommand("0D0!");
  SDIdata();

}



// Function to rad data from SDI-12 sensor
char* SDIdata(void)
{
  char sdiResponse[30];
  int i;

  i = 0;
  while (mySDI12.available())  // write the response to the screen
  {
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r'))
    {
      sdiResponse[i] = c;
      i++;
    }
    delay(5);
  }
  delay(1000); // Needed for CTD10 pressure sensor
  USB.println(sdiResponse);
  mySDI12.flush();
  memset (sdiResponse, 0, sizeof(sdiResponse));

  return sdiResponse;
}


float getLatitudeGPS(void){
  GPS.ON();
  bool status = GPS.waitForSignal(TIMEOUT);
  if ( status == true ){
    GPS.getLatitude();
    float lat = GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator);
    return lat;
  }
  GPS.OFF();
}

float getLongitudeGPS(void){
  GPS.ON();
  bool status = GPS.waitForSignal(TIMEOUT);
  if ( status == true ){

    GPS.getLongitude();
    float lat = GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator);
    return lat;
  }
  GPS.OFF();
}


void syncRTCtoGPS(void){
GPS.ON();
USB.println("GPS ON");
RTC.ON();
bool status = GPS.waitForSignal(TIMEOUT);
if ( status == true ){
  USB.println("GPS fixed");
  GPS.setTimeFromGPS();
  USB.println("RTC updated");
}else{USB.println("No GPS signal");}
GPS.OFF();
RTC.OFF();
}




