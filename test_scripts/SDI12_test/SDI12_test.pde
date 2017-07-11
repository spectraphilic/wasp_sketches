



#include <WaspFrame.h>
#include <WaspGPS.h>
#include <SDI12.h>

// SD
// define folder and file to store data
char path[] = "/data";
char filename[] = "/data/log.txt";

// buffer to write into Sd File
char toWrite[200];

// define variables to read stored frames
uint8_t frameSD[MAX_FRAME + 1];
uint16_t lengthSD;
int32_t numLines;

// variables to define the file lines to be read
int startLine;
int endLine;

// define variable
uint8_t sd_answer;

// GPS
#define TIMEOUT 240

float Lat;
float Lon;

char Lat_str[20];
char Lon_str[20];


// define status variable for GPS connection
bool status;

long time = 0;

// SDI-12
#define DATAPIN 6         // change to the proper pin (JH) 6 = DIGITAL 6 on Waspmote
SDI12 mySDI12(DATAPIN);
char sdiResponse[30];
int i;

void setup()
{
  // open USB port
  USB.ON();
  USB.println(F("Waking-up"));

  // Set SD ON
  SD.ON();

  // create path
  sd_answer = SD.mkdir(path);

  if ( sd_answer == 1 )
  {
    USB.println(F("path created"));
  }
  else
  {
    USB.println(F("mkdir failed, error or dir already exist"));
  }

  // Create file for Waspmote Frames
  sd_answer = SD.create(filename);

  if ( sd_answer == 1 )
  {
    USB.println(F("/data/log.txt created"));
  }
  else
  {
    USB.println(F("/data/log.txt not created, error or file already exist"));
  }


  // set GPS ON
  GPS.ON();
  // set RTC ON
  RTC.ON();

  //////////////////////////////////////////////////////
  // 1. wait for GPS signal for specific time
  //////////////////////////////////////////////////////
  status = GPS.waitForSignal(TIMEOUT);

  //////////////////////////////////////////////////////
  // 2. if GPS is connected then set Time and Date to RTC
  //////////////////////////////////////////////////////
  if ( status == true )
  {
    // set time in RTC from GPS time (GMT time)
    GPS.setTimeFromGPS();
    USB.println(F("\n----------------------"));
    USB.println(F("RTC clock set by GPS"));
    USB.println(F("----------------------"));

    SD.appendln(filename, "\n----------------------");
    SD.appendln(filename, "RTC clock set by GPS");
    SD.appendln(filename, "----------------------");


  }
  else
  {
    USB.println(F("\n----------------------"));
    USB.println(F("GPS TIMEOUT. NOT connected"));
    USB.println(F("----------------------"));



  }

  if ( status == true)
  {
    // Getting Time
    GPS.getTime();
    USB.print(F("Time [hhmmss.sss]: "));
    USB.println(GPS.timeGPS);

    SD.append(filename, "Time [hhmmss.sss]: ");
    SD.appendln(filename, GPS.timeGPS);

    // Getting Date
    GPS.getDate();
    USB.print(F("Date [ddmmyy]: "));
    USB.println(GPS.dateGPS);

    SD.append(filename, "Date [ddmmyy]: ");
    SD.appendln(filename, GPS.dateGPS);

    // Getting Latitude
    GPS.getLatitude();
    USB.print(F("Latitude [ddmm.mmmm]: "));
    USB.println(GPS.latitude);
    USB.print(F("North/South indicator: "));
    USB.println(GPS.NS_indicator);
    USB.print("Latitude (degrees):");
    USB.println(GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator));

    Lat = GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator);
    Utils.float2String(Lat, Lat_str, 6);

    SD.append(filename, "Latitude (degrees):");
    SD.appendln(filename, Lat_str);

    // Getting Longitude
    GPS.getLongitude();
    USB.print(F("Longitude [dddmm.mmmm]: "));
    USB.println(GPS.longitude);
    USB.print(F("East/West indicator: "));
    USB.println(GPS.EW_indicator);
    USB.print("Longitude (degrees):");
    USB.println(GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));
    USB.println(F("----------------------"));

    Lon = GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator);
    Utils.float2String(Lon, Lon_str, 6);


    SD.append(filename, "Longitude (degrees):");
    SD.appendln(filename, Lon_str);
    SD.appendln(filename, "----------------------");

  }

  PWR.setSensorPower(SENS_5V, SENS_ON); // (JH)
  delay(500); // Sensor exitation delay

  mySDI12.begin();

  // -'-'-'-'-'-'-INFO COMMAND-'-'-'-'-'-'-
  mySDI12.sendCommand("0I!");
  SDIdata();
  USB.println(F("----------------------"));
  SD.appendln(filename, "----------------------");

}

void loop()
{
  // Goes to sleep... and wake up.
  PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_OFF);

  USB.ON();
  RTC.ON();
  SD.ON();

  // Clear alarms (interuptions)
  if (intFlag & RTC_INT)
  {
    intFlag &= ~(RTC_INT); // Clear flag
  }


  // Raedung timestamp from RTC
  USB.print(RTC.getTime());
  SD.append(filename, RTC.getTime());
  USB.print(F(","));
  SD.append(filename, ",");



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

  USB.println("");
  SD.appendln(filename, "");
}



// Function to rad data from SDI-12 sensor
void SDIdata(void)
{
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
  USB.print(sdiResponse); //write the response to the screen
  SD.append(filename, sdiResponse);

  USB.print(F(","));
  SD.append(filename, ",");

  delay(1000); // Needed for CTD10 pressure sensor
  mySDI12.flush();
  memset (sdiResponse, 0, sizeof(sdiResponse));
}





