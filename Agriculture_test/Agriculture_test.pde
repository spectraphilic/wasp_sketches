/*  
 *  ------------  WaspMote Agriculture Test  -------------- 
 *  
 */

#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>
#include <WaspGPS.h>
#include <WaspXBeeDM.h>
#include <WaspUIO.h>

char node_ID[] = "WS_test1";

/*** variables to store sensors readings ***/
float sensirionTemperature;
float sensirionHumidity;
float pressure;
uint8_t wetness;
float DS18B20Temperature;
float anemometer;
uint8_t vane;
float pluviometer1;
float pluviometer2;
float pluviometer3;

uint8_t ultrasound;


/*** variable to store the number of pending pulses ***/
int pendingPulses;

/*** GPS variables ***/
// define GPS timeout when connecting to satellites
// this time is defined in seconds (240sec = 4minutes)
#define TIMEOUT 240

// define status variable for GPS connection
bool status = false;
char GPSmsg[] = "GPS not connected";

/*** XBee variables ***/
char RX_ADDRESS[] = "0013a20040779085"; // "0013a20040779085" Meshlium_Finse


void setup() 
{
  USB.ON();
  USB.println(F("Weather station test"));

  RTC.ON();

  // Set the Waspmote ID
  frame.setID(node_ID); 


  // Set GPS ON  
  GPS.ON();

  // 1. wait for GPS signal for specific time
  status = GPS.waitForSignal(TIMEOUT);

  // 2. if GPS is connected then set Time and Date to RTC
  if(status == true)
  {    
    // set time in RTC from GPS time (GMT time)
    GPS.setTimeFromGPS();
    USB.print(F("Time GPS: "));
  }
  else
  {
    xbeeDM.ON();
    xbeeDM.setRTCfromMeshlium(RX_ADDRESS);
    USB.print(F("Time Meshlium: "));
  }
  USB.println(RTC.getTime());

  xbeeDM.OFF();
  GPS.OFF();
  RTC.OFF();
  USB.OFF();

  // Turn on the sensor board
  SensorAgrv20.ON();

  delay(2000);
}  

void loop()
{
  /////////////////////////////////////////////
  // 1. Enter sleep mode
  /////////////////////////////////////////////

  // "00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5" -> Measures every minute

  SensorAgrv20.sleepAgr("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, SOCKET0_OFF, SENS_AGR_PLUVIOMETER);


  /////////////////////////////////////////////
  // 2 Check interruptions
  /////////////////////////////////////////////

  SensorAgrv20.detachPluvioInt(); // Is this needed???

  //Check pluviometer interruption
  if( intFlag & PLV_INT)
  {
    USB.println(F("+++ PLV interruption +++"));

    pendingPulses = intArray[PLV_POS];

    USB.print(F("Number of pending pulses:"));
    USB.println( pendingPulses );

    for(int i=0 ; i<pendingPulses; i++)
    {
      // Enter pulse information inside class structure
      SensorAgrv20.storePulse();

      // decrease number of pulses
      intArray[PLV_POS]--;
    }

    // Clear flag
    intFlag &= ~(PLV_INT); 
  }

  //Check RTC interruption
  if(intFlag & RTC_INT)
  {
    USB.println(F("+++ RTC interruption +++"));


    // Gets time for Alarm1
    USB.println(RTC.getAlarm1());


    //if (RTC.minute_alarm1 == 0)

      // switch on sensor board
      //SensorAgrv20.ON();

      RTC.ON();
    USB.print(F("Time:"));
    USB.println(RTC.getTime());        

    // measure sensors
    measureSensors();

    // Clear flag
    intFlag &= ~(RTC_INT); 
  }
  clearIntFlag(); 
  PWR.clearInterruptionPin();
}


///////////////////////////////////////////
// Function: measureSensors
/////////////////////////////////////////// 

void measureSensors()
{  
  // 1. Turn on the sensors
  USB.print(F("Turn on the sensors..."));

  // Power on Sensirion
  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_SENSIRION);
  // Power on the pressure sensor
  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_PRESSURE);
  // Power on the leaf wetness sensor
  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_LEAF_WETNESS);
  // Power on the weather station sensor
  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_ANEMOMETER);
  // Power on the temperature sensor
  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_TEMP_DS18B20);
  delay(10000);

  // 2. Read sensors

  USB.print(F("Read sensors..."));
  // Read the digital temperature sensor 
  sensirionTemperature = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_TEMP);
  // Read the digital humidity sensor 
  sensirionHumidity = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_HUM);
  // Read the pressure sensor
  pressure = SensorAgrv20.readValue(SENS_AGR_PRESSURE);
  // Read the leaf wetness sensor 
  wetness = SensorAgrv20.readValue(SENS_AGR_LEAF_WETNESS);
  //Sensor temperature reading
  DS18B20Temperature = SensorAgrv20.readValue(SENS_AGR_TEMP_DS18B20);
  // Read the anemometer sensor 
  anemometer = SensorAgrv20.readValue(SENS_AGR_ANEMOMETER);
  // Read the vane sensor 
  vane = SensorAgrv20.readValue(SENS_AGR_VANE);
  // Read the pluviometer sensor 
  pluviometer1 = SensorAgrv20.readPluviometerCurrent();
  pluviometer2 = SensorAgrv20.readPluviometerHour();
  pluviometer3 = SensorAgrv20.readPluviometerDay();

  ultrasound = UIO.readMaxbotixSerial();

  // 3. Turn off the sensors

  USB.print(F("Turn off the sensors..."));
  // Power off Sensirion
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_SENSIRION);
  // Power off the pressure sensor
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_PRESSURE);
  // Power off the leaf wetness sensor
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_LEAF_WETNESS);
  // Power off the temperature sensor
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_TEMP_DS18B20);
  // Power off the weather station sensor
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_ANEMOMETER);

  // 4. Read GPS

  USB.print(F("Power on GPS..."));
  // Power on GPS
  GPS.ON();
  status = GPS.waitForSignal(TIMEOUT);

  // if GPS is connected then get position
  if(status == true)
  {
    // getPosition function gets all basic data 
    GPS.getPosition();
    // set time in RTC from GPS time (GMT time)
    GPS.setTimeFromGPS();
  }
  GPS.OFF();


  ////////////////////////////////////////////////
  // 5. Send data
  ////////////////////////////////////////////////

  USB.println(F("Send data..."));

  xbeeDM.ON();
  delay(2000);  

  // 5. Create ASCII frame
  frame.createFrame(ASCII);
  // set frame fields (Battery sensor - uint8_t)
  frame.addSensor(SENSOR_BAT, PWR.getBatteryLevel());
  // set frame fields (Temperature in Celsius sensor - float)
  frame.addSensor(SENSOR_IN_TEMP, RTC.getTemperature());
  // set frame fields (XYZ)
  frame.addSensor(SENSOR_ACC, (int) ACC.getX(), (int) ACC.getY(), (int) ACC.getZ());


  // Show the frame
  frame.showFrame();
  //  Send XBee packet
  xbeeDM.send(RX_ADDRESS, frame.buffer, frame.length); 
  delay(1000);

  // 5. Create ASCII frame
  frame.createFrame(ASCII);
  // Add digital temperature
  frame.addSensor(SENSOR_TCB, sensirionTemperature);
  // Add digital humidity
  frame.addSensor(SENSOR_HUMB, sensirionHumidity);
  // Add pressure
  frame.addSensor(SENSOR_PA, pressure);
  // Show the frame
  frame.showFrame();
  //  Send XBee packet
  xbeeDM.send(RX_ADDRESS, frame.buffer, frame.length); 
  delay(1000);

  // 5. Create ASCII frame
  frame.createFrame(ASCII);
  // Add wetness
  frame.addSensor(SENSOR_LW, wetness);
  // Add DS18B20 temperature
  frame.addSensor(SENSOR_TCC, DS18B20Temperature);
  // Show the frame
  frame.showFrame();
  //  Send XBee packet
  xbeeDM.send(RX_ADDRESS, frame.buffer, frame.length); 
  delay(1000);

  // 5. Create ASCII frame
  frame.createFrame(ASCII);
  // Add anemometer value
  frame.addSensor( SENSOR_ANE, anemometer );
  // Add wind vane value
  frame.addSensor( SENSOR_WV, vane );
  // Add ultrasound
  frame.addSensor(SENSOR_US, ultrasound);
  // Show the frame
  frame.showFrame();
  //  Send XBee packet
  xbeeDM.send(RX_ADDRESS, frame.buffer, frame.length); 
  delay(1000);

  // 5. Create ASCII frame
  frame.createFrame(ASCII);
  // Add pluviometer value
  frame.addSensor( SENSOR_PLV1, pluviometer1 );
  // Add pluviometer value
  frame.addSensor( SENSOR_PLV2, pluviometer2 );
  // Add pluviometer value
  frame.addSensor( SENSOR_PLV3, pluviometer3 );
  // Show the frame
  frame.showFrame();
  //  Send XBee packet
  xbeeDM.send(RX_ADDRESS, frame.buffer, frame.length); 
  delay(1000);

  // if GPS is connected then get position
  if( status == true )
  {
    // 5. Create ASCII frame
    frame.createFrame(ASCII);
    // add GPS values
    frame.addSensor(SENSOR_GPS, 
    GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator),
    GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator) );
    // Show the frame
    frame.showFrame();
    //  Send XBee packet
    xbeeDM.send(RX_ADDRESS, frame.buffer, frame.length);
  }
  delay(1000);

  // 5.4 Communication module to OFF
  xbeeDM.OFF();
  delay(1000);
}





































