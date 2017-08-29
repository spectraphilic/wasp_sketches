/******************************************************************************
I2C_ReadAllData.ino
BME280 Arduino and Teensy example
Marshall Taylor @ SparkFun Electronics
May 20, 2015
https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
This sketch configures the BME280 to read all measurements.  The sketch also
displays the BME280's physical memory and what the driver perceives the
calibration words to be.
Resources:
Uses Wire.h for I2C operation
Uses SPI.h for SPI operation
Development environment specifics:
Arduino IDE 1.6.4
Teensy loader 1.23
This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

#include <stdint.h>
#include "SparkFunBME280.h"
//Library allows either I2C or SPI, so include both.
#include "Wire.h"

//Global sensor object
BME280 mySensor;

void setup()
{
	//***Driver settings********************************//
	//commInterface can be I2C_MODE or SPI_MODE
	//specify chipSelectPin using arduino pin names
	//specify I2C address.  Can be 0x77(default) or 0x76
	
	//For I2C, enable the following and disable the SPI section
	mySensor.settings.commInterface = I2C_MODE;
	mySensor.settings.I2CAddress = 0x77;
	

	//***Operation settings*****************************//
	
	//renMode can be:
	//  0, Sleep mode
	//  1 or 2, Forced mode
	//  3, Normal mode
	mySensor.settings.runMode = 3; //Normal mode
	
	//tStandby can be:
	//  0, 0.5ms
	//  1, 62.5ms
	//  2, 125ms
	//  3, 250ms
	//  4, 500ms
	//  5, 1000ms
	//  6, 10ms
	//  7, 20ms
	mySensor.settings.tStandby = 0;
	
	//filter can be off or number of FIR coefficients to use:
	//  0, filter off
	//  1, coefficients = 2
	//  2, coefficients = 4
	//  3, coefficients = 8
	//  4, coefficients = 16
	mySensor.settings.filter = 0;
	
	//tempOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
	mySensor.settings.tempOverSample = 1;

	//pressOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    	mySensor.settings.pressOverSample = 1;
	
	//humidOverSample can be:
	//  0, skipped
	//  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
	mySensor.settings.humidOverSample = 1;
	
	USB.ON();
	USB.print("Program Started\n");
	USB.print("Starting BME280... result of .begin(): 0x");
	
	//Calling .begin() causes the settings to be loaded
	delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.
	USB.println(mySensor.begin(), HEX);

	USB.print("Displaying ID, reset and ctrl regs\n");
	
	USB.print("ID(0xD0): 0x");
	USB.println(mySensor.readRegister(BME280_CHIP_ID_REG), HEX);
	USB.print("Reset register(0xE0): 0x");
	USB.println(mySensor.readRegister(BME280_RST_REG), HEX);
	USB.print("ctrl_meas(0xF4): 0x");
	USB.println(mySensor.readRegister(BME280_CTRL_MEAS_REG), HEX);
	USB.print("ctrl_hum(0xF2): 0x");
	USB.println(mySensor.readRegister(BME280_CTRL_HUMIDITY_REG), HEX);

	USB.print("\n\n");

	USB.print("Displaying all regs\n");
	uint8_t memCounter = 0x80;
	uint8_t tempReadData;
	for(int rowi = 8; rowi < 16; rowi++ )
	{
		USB.print("0x");
		USB.print(rowi, HEX);
		USB.print("0:");
		for(int coli = 0; coli < 16; coli++ )
		{
			tempReadData = mySensor.readRegister(memCounter);
			USB.print((tempReadData >> 4) & 0x0F, HEX);//Print first hex nibble
			USB.print(tempReadData & 0x0F, HEX);//Print second hex nibble
			USB.print(" ");
			memCounter++;
		}
		USB.print("\n");
	}
	
	
	USB.print("\n\n");
	
	USB.print("Displaying concatenated calibration words\n");
	USB.print("dig_T1, uint16: ");
	USB.println(mySensor.calibration.dig_T1);
	USB.print("dig_T2, int16: ");
	USB.println(mySensor.calibration.dig_T2);
	USB.print("dig_T3, int16: ");
	USB.println(mySensor.calibration.dig_T3);
	
	USB.print("dig_P1, uint16: ");
	USB.println(mySensor.calibration.dig_P1);
	USB.print("dig_P2, int16: ");
	USB.println(mySensor.calibration.dig_P2);
	USB.print("dig_P3, int16: ");
	USB.println(mySensor.calibration.dig_P3);
	USB.print("dig_P4, int16: ");
	USB.println(mySensor.calibration.dig_P4);
	USB.print("dig_P5, int16: ");
	USB.println(mySensor.calibration.dig_P5);
	USB.print("dig_P6, int16: ");
	USB.println(mySensor.calibration.dig_P6);
	USB.print("dig_P7, int16: ");
	USB.println(mySensor.calibration.dig_P7);
	USB.print("dig_P8, int16: ");
	USB.println(mySensor.calibration.dig_P8);
	USB.print("dig_P9, int16: ");
	USB.println(mySensor.calibration.dig_P9);
	
	USB.print("dig_H1, uint8: ");
	USB.println(mySensor.calibration.dig_H1);
	USB.print("dig_H2, int16: ");
	USB.println(mySensor.calibration.dig_H2);
	USB.print("dig_H3, uint8: ");
	USB.println(mySensor.calibration.dig_H3);
	USB.print("dig_H4, int16: ");
	USB.println(mySensor.calibration.dig_H4);
	USB.print("dig_H5, int16: ");
	USB.println(mySensor.calibration.dig_H5);
	USB.print("dig_H6, uint8: ");
	USB.println(mySensor.calibration.dig_H6);
		
	USB.println();
}

void loop()
{
	//Each loop, take a reading.
	//Start with temperature, as that data is needed for accurate compensation.
	//Reading the temperature updates the compensators of the other functions
	//in the background.

	USB.print("Temperature: ");
	USB.print(mySensor.readTempC(), 2);
	USB.println(" degrees C");

	USB.print("Temperature: ");
	USB.print(mySensor.readTempF(), 2);
	USB.println(" degrees F");

	USB.print("Pressure: ");
	USB.print(mySensor.readFloatPressure(), 2);
	USB.println(" Pa");

	USB.print("Altitude: ");
	USB.print(mySensor.readFloatAltitudeMeters(), 2);
	USB.println("m");

	USB.print("Altitude: ");
	USB.print(mySensor.readFloatAltitudeFeet(), 2);
	USB.println("ft");	

	USB.print("%RH: ");
	USB.print(mySensor.readFloatHumidity(), 2);
	USB.println(" %");
	
	USB.println();
	
	delay(1000);

}