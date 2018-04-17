#include <SDI12.h>


void setup()
{
  // Power board
  pinMode(14, OUTPUT); // 3V3
  pinMode(15, OUTPUT); // 5V
  pinMode(16, OUTPUT); // 12V
  pinMode(17, OUTPUT); // Lead-Acid power
  pinMode(ANALOG5, INPUT); // Lead-Acid data

  // Sensor board
  pinMode(DIGITAL1 , OUTPUT); // Maxbotix power
  pinMode(DIGITAL2 , OUTPUT); // I2C power
  pinMode(DIGITAL5 , OUTPUT); // Onewire power
  pinMode(DIGITAL7 , OUTPUT); // SDI12 power
  pinMode(DIGITAL6, INPUT); // Onewire data
  pinMode(DIGITAL8, INPUT); // SDI12 data
}


void loop()
{
  SDI12 mySDI12(DIGITAL7);
  uint8_t id;

  // ON
  USB.ON();
  USB.println("ON");
  USB.println();
  //digitalWrite(16, HIGH); // Lead acid (power board)
  //digitalWrite(15, HIGH); // Lead acid (power board)
  PWR.setSensorPower(SENS_5V, SENS_ON); // Lithium
  digitalWrite(DIGITAL7, HIGH); // Sensor board
  delay(1000);

  // Read
  mySDI12.begin();
  id = mySDI12.identification(0);
  USB.print("Id (0): "); USB.print(id); USB.println();
  id = mySDI12.identification(1);
  USB.print("Id (1): "); USB.print(id); USB.println();
  mySDI12.end();

  // OFF
  USB.println("OFF");
  digitalWrite(DIGITAL7, LOW);
  digitalWrite(15, LOW);

  // Sleep
  PWR.deepSleep("00:00:00:10", RTC_OFFSET, RTC_ALM1_MODE1, ALL_OFF); // 10s
  if( intFlag & RTC_INT ) { intFlag &= ~(RTC_INT); }
}
