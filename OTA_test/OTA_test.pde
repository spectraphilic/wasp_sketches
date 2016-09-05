/*  
 *  ------------  WaspMote Agriculture and OTAP testing -------------- 
 *  
 */

#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>
#include <WaspGPS.h>
#include <WaspXBeeDM.h>
//#include <WaspUIO.h>

 char waspmote_ID[]="1";

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


/*** GPS variables ***/
// define GPS timeout when connecting to satellites
// this time is defined in seconds (240sec = 4minutes)
#define TIMEOUT 60
#define key_access "LIBELIUM"

// define status variable for GPS connection
 bool status = false;
 char GPSmsg[] = "GPS not connected";

/*** XBee variables ***/
char RX_ADDRESS[] = "0013a20040779085"; // "0013a20040779085" Meshlium_Finse

unsigned long epoch;

void setup() 
{
  // Claer all interuptions (just in case...)  
  //  clearIntFlag(); 
  //  PWR.clearInterruptionPin();

  USB.ON();
  USB.println(F("Finse WSN"));

  // Set the Waspmote ID
  frame.setID(waspmote_ID); 

  // Setup
  // Write Authentication Key to EEPROM memory
  Utils.setAuthKey(key_access);
  // Initialize XBee module
  xbeeDM.ON();
  // CheckNewProgram is mandatory in every OTA program
  xbeeDM.checkNewProgram();  
  USB.println("Setup: Check for new program...");

  // Synconize time with Meshlium
  RTC.ON();
  USB.println("Setup: Starting RTC");
  delay(1000);
  xbeeDM.setRTCfromMeshlium(RX_ADDRESS);
  USB.println("Setup: Set time from Meslium");
  delay(1000);
  USB.print("Setup: time updated: ");
  USB.println(RTC.getTime());
  //xbeeDM.OFF();

  // Turn on the sensor board
  //SensorAgrv20.ON();

  delay(2000);
}  

void loop()
{
  /////////////////////////////////////////////
  // 1. Enter sleep mode
  /////////////////////////////////////////////


    // Sample every 2 minutes
  //RTC.ON();
  check_for_OTA_new_program();

  // if ((RTC.minute%2)==0)
  // { 
  //     // Measure sensors
  //   xbeeDM.ON();
  //   // USB.println(F("+++ Sampling +++"));
  //   // measureSensors();
  //   // USB.println(F("+++ Sampled +++"));

  //   USB.println(F("+++ Check for new OTA program +++"));
  //   check_for_OTA_new_program();
  //   USB.println(F("+++ Check done +++"));
  //   xbeeDM.OFF();

  //     // Update time every day at noon
  //   if (RTC.hour == 12 && RTC.minute == 0)
  //   {
  //       // Synconize time with Meshlium
  //     xbeeDM.ON();
  //     USB.println(F("+++ Synchromize time with Meshlium +++"));
  //     synchronize_time_meshlium();
  //     USB.println(F("+++ Time synchronized +++"));
  //     xbeeDM.OFF();
  //   }
  // }

}


//===============================================
//===============================================
//              FUNCTIONS
//===============================================
//===============================================


///////////////////////////////////////////
// Function: Symvhromize time with Meshlium
/////////////////////////////////////////// 

void synchronize_time_meshlium()
{
  delay(1000);
  xbeeDM.setRTCfromMeshlium(RX_ADDRESS);
  delay(1000);
  USB.print(F("Time Meshlium: "));
  USB.println(RTC.getTime());
}

///////////////////////////////////////////
// Function: Check if any program are uvailable to download via OTA
/////////////////////////////////////////// 

void check_for_OTA_new_program()
{
  // Check if new data is available
  if( xbeeDM.available() )
  {
    xbeeDM.treatData();
    // Keep inside this loop while a new program is being received
    while( xbeeDM.programming_ON  && !xbeeDM.checkOtapTimeout() )
    {
      if( xbeeDM.available() )
      {
        xbeeDM.treatData();
      }
    }
  }
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

  // ultrasound = UIO.readMaxbotixSerial();

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

  //  USB.print(F("Power on GPS..."));
  //  // Power on GPS
  //  GPS.ON();
  //  status = GPS.waitForSignal(TIMEOUT);
  //
  //  // if GPS is connected then get position
  //  if(status == true)
  //  {
  //    // getPosition function gets all basic data 
  //    GPS.getPosition();
  //    // set time in RTC from GPS time (GMT time)
  //    GPS.setTimeFromGPS();
  //  }
  //  GPS.OFF();


  ////////////////////////////////////////////////
  // 5. Send data
  ////////////////////////////////////////////////

  USB.println(F("Send data..."));

  xbeeDM.ON();
  delay(2000);  

  // 5. Create ASCII frame, basic sensors
  frame.createFrame(ASCII);
  // set frame fields (Battery sensor - uint8_t)
  frame.addSensor(SENSOR_BAT, PWR.getBatteryLevel());
  // set frame fields (Temperature in Celsius sensor - float)
  frame.addSensor(SENSOR_IN_TEMP, RTC.getTemperature());
  // set frame fields (XYZ)
  ACC.ON();
  delay(100);
  frame.addSensor(SENSOR_ACC, (int) ACC.getX(), (int) ACC.getY(), (int) ACC.getZ());
  ACC.OFF();

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
  // // Add ultrasound
  // frame.addSensor(SENSOR_US, ultrasound);
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

  //  // if GPS is connected then get position
  //  if( status == true )
  //  {
  //    // 5. Create ASCII frame
  //    frame.createFrame(ASCII);
  //    // add GPS values
  //    frame.addSensor(SENSOR_GPS, 
  //    GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator),
  //    GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator) );
  //    // Show the frame
  //    frame.showFrame();
  //    //  Send XBee packet
  //    xbeeDM.send(RX_ADDRESS, frame.buffer, frame.length);
  //  }
  delay(5000);

  // 5.4 Communication module to OFF
  xbeeDM.OFF();
  delay(1000);

    ////////////////////////////////////////////////
  // 6. Save data locally (SD card)
  ////////////////////////////////////////////////





}















