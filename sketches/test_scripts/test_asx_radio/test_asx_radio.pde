
// Put your libraries here (#include ...)


#include <SparkFun_AS7265X.h> //Click here to get the library: http://librarymanager/All#SparkFun_AS7265X
AS7265X sensor;

void setup() {
  PWR.setSensorPower(SENS_3V3, SENS_ON);
  delay(100);
  pinMode(DIGITAL2, OUTPUT);
  digitalWrite(DIGITAL2, HIGH);
  delay(100);
  USB.println((long)I2C.scan(0x49));
  
  delay(100);
  USB.ON();
  USB.println("AS7265x Spectral Triad Example");
  
  
  if(sensor.isConnected() == false)
  {
    USB.println("Sensor not connected...");
  }

  if(sensor.begin() == false)
  {
    USB.println("Sensor does not appear to be connected. Please check wiring. Freezing...");
  }
  
  USB.println("A,B,C,D,E,F,G,H,I,J,K,L,R,S,T,U,V,W");
  

}

void loop() {
  sensor.takeMeasurements(); //This is a hard wait while all 18 channels are measured

  USB.print(sensor.getCalibratedA());
  USB.print(",");
  USB.print(sensor.getCalibratedB());
  USB.print(",");
  USB.print(sensor.getCalibratedC());
  USB.print(",");
  USB.print(sensor.getCalibratedD());
  USB.print(",");
  USB.print(sensor.getCalibratedE());
  USB.print(",");
  USB.print(sensor.getCalibratedF());
  USB.print(",");

  USB.print(sensor.getCalibratedG());
  USB.print(",");
  USB.print(sensor.getCalibratedH());
  USB.print(",");
  USB.print(sensor.getCalibratedI());
  USB.print(",");
  USB.print(sensor.getCalibratedJ());
  USB.print(",");
  USB.print(sensor.getCalibratedK());
  USB.print(",");
  USB.print(sensor.getCalibratedL());
  USB.print(",");

  USB.print(sensor.getCalibratedR());
  USB.print(",");
  USB.print(sensor.getCalibratedS());
  USB.print(",");
  USB.print(sensor.getCalibratedT());
  USB.print(",");
  USB.print(sensor.getCalibratedU());
  USB.print(",");
  USB.print(sensor.getCalibratedV());
  USB.print(",");
  USB.print(sensor.getCalibratedW());
  USB.print(",");

  USB.println();
}

