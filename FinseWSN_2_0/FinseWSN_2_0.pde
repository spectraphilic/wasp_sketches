/*  
 */

// Put your libraries here (#include ...)
#include <WaspXBeeDM.h>
#include <WaspUIO.h>
#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>



#define key_access "LIBELIUM"   // in use for OTA programing
char node_ID[10];
String message = "";



void setup() {

	USB.ON();
	SD.ON();
	xbeeDM.ON();
	xbeeDM.checkNewProgram(); // CheckNewProgram is mandatory in every OTA program


  	// Function to initialize SD card
	UIO.initSD();

  	// Function to initialize
	UIO.initNet('Finse');

	UIO.logActivity('SD and XbeeDM initialized');

	// Turn on the sensor board
	SensorAgrv20.ON();
	SensorAgrv20.attachPluvioInt();
	delay(2000);


	USB.OFF();	
	SD.OFF();
	xbeeDM.OFF();
	UIO.logActivity('Waspmote all set and ready');
}


void loop() {

	SensorAgrv20.sleepAgr("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, SOCKET0_OFF, SENS_AGR_PLUVIOMETER);
	SensorAgrv20.detachPluvioInt(); 


	//If statement to record pluviometer interruptions
	if( intFlag & PLV_INT)
	{	
		USB.ON();
		SD.ON();
		message = "+++ PLV interruption +++";
		UIO.logActivity(message);

		pendingPulses = intArray[PLV_POS];

		message = "Number of pending pulses:" + String(pendingPulses);
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
		USB.OFF();
		SD.OFF();
	}



  //Check RTC interruption
	if(intFlag & RTC_INT)
	{
		RTC.ON();
		SD.ON();
		USB.ON();

		message = "+++ RTC interruption +++";
		UIO.logActivity(message);

		epoch = RTC.getEpochTime();


    Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    // Sample every 10 minutes
    if ((RTC.minute%10)==0)
    { 
      // Update time every day at noon
    	if (RTC.hour == 12 && RTC.minute == 0)
    	{
        // Synconize time with GPS
    	}

      // Measure sensors
    	message = "+++ Sampling +++";
    	UIO.logActivity(message);
    	measureSensors();
    }

    // Clear flag
    intFlag &= ~(RTC_INT); 
}
clearIntFlag(); 
PWR.clearInterruptionPin();
}

  // loop to record sensors, add data to tmp file
if()
{
	SD.ON();
	if(USB.available()){

		USB.ON();}

		// [INCLUDE CODE HERE] to read sensor and store values into tmp file

		SD.OFF();
		USB.OFF();
	}


  // loop to start xbeeDM and exchange data through network
	if()
	{
		xbeeDM.ON();
		SD.ON();
		if(USB.available()){USB.ON();}

		// [INCLUDE CODE HERE] Synchronize time


		// Transfer frames from tmp_file to data file, unsent_file if no connectoin
		UIO.Frame2Meshlium( tmp_file, data_file, unsent_file,  RX_ADDRESS,  log_file );

		

  		// [INCLUDE CODE HERE]

		xbeeDM.OFF();
		SD.OFF();
		USB.OFF();
	}
}


///////////////////////////////////////////
// Function: measureSensors
/////////////////////////////////////////// 

void measureSensorsBasicSet()
{  
	USB.ON();
	SD.ON();
  	// 1. Turn on the sensors
	UIO.logActivity("Turn on the sensors...");

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
	delay(10000); // 10s delay for sensor warm up. CHECK IF possible

  	// 2. Read sensors

	UIO.logActivity("Read sensors...");
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

	UIO.logActivity("Turn off the sensors...");
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


  	////////////////////////////////////////////////
  	// 5. Save data to SD in frame format
  	////////////////////////////////////////////////

	UIO.logActivity("Save data to tmp_file.txt...");

  	// 5. Create ASCII frame, basic sensors
	frame.createFrame(ASCII);
  	// set frame fields (Battery sensor - uint8_t)
	frame.addSensor(SENSOR_BAT, PWR.getBatteryLevel());
  	// set frame fields (Temperature in Celsius sensor - float)
	frame.addSensor(SENSOR_IN_TEMP, RTC.getTemperature());
  	// set frame fields (XYZ)
	ACC.ON();
	delay(50);
	frame.addSensor(SENSOR_ACC, (int) ACC.getX(), (int) ACC.getY(), (int) ACC.getZ());
	ACC.OFF();

  	// Show the frame
	frame.showFrame();
	UIO.frameToSd(UIO.tmp_file, frame.buffer, frame.length); 

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
	UIO.frameToSd(UIO.tmp_file, frame.buffer, frame.length); 

  	// 5. Create ASCII frame
	frame.createFrame(ASCII);
  	
	frame.addSensor(SENSOR_LW, wetness); // Add wetness
	frame.addSensor(SENSOR_TCC, DS18B20Temperature); // Add DS18B20 temperature
	frame.showFrame(); // Show the frame
  	//  Send XBee packet
	UIO.frameToSd(UIO.tmp_file, frame.buffer, frame.length); 

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
	UIO.frameToSd(UIO.tmp_file, frame.buffer, frame.length); 

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
	UIO.frameToSd(UIO.tmp_file, frame.buffer, frame.length); 

	UIO.logActivity("Data saved to " + String(UIO.tmp_file));
	SD.OFF();
	USB.ON();
}
