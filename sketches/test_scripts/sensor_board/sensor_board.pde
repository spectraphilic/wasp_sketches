#include <SDI12.h>

/*
 * The call to deepSleep first switches off sensor power, then uses the RTC.
 * But if there're I2C devices attached the RTC won't work, it will print the
 * error "[PWR] deepSleep RTC error".
 * To make it work we need to switch on 3V3, this is done in TwoWire::secureBegin,
 * for this to happen there's the include line below.
 *
 * So the workflow within deepSleep will be:
 *
 * - Switch Off 3V3
 * - Switch On 3V3
 * - Use RTC
 * - Switch Off 3V3
 *
 * This is suboptimal. To fix it we should override TwoWire::secureBegin
 */
#include <WaspSensorAmbient.h> // This is used to enable 3V3


#define SENSOR_BOARD 0
#define POWER_BOARD 1


void setup()
{
#if POWER_BOARD
  pinMode(14, OUTPUT); // 3V3
  pinMode(15, OUTPUT); // 5V
  pinMode(16, OUTPUT); // 12V
  pinMode(17, OUTPUT); // Lead-Acid power
  pinMode(ANALOG5, INPUT); // Lead-Acid data
#endif

#if SENSOR_BOARD
  pinMode(DIGITAL1 , OUTPUT); // Maxbotix power
  pinMode(DIGITAL2 , OUTPUT); // I2C power
  pinMode(DIGITAL5 , OUTPUT); // Onewire power
  pinMode(DIGITAL7 , OUTPUT); // SDI12 power
  pinMode(DIGITAL6, INPUT); // Onewire data
  pinMode(DIGITAL8, INPUT); // SDI12 data
#endif
}


void loop()
{
  SDI12 mySDI12(DIGITAL8);
  uint8_t id;

  // Power On
#if POWER_BOARD
  digitalWrite(16, HIGH); // 12V
  digitalWrite(15, HIGH); // 5V
#else
  PWR.setSensorPower(SENS_5V, SENS_ON);
#endif
#if (SENSOR_BOARD || POWER_BOARD)
  digitalWrite(DIGITAL7, HIGH);
#endif
  delay(1000);

  // Read
  USB.println("Identifying SDI-12 devices...");
  mySDI12.begin();
  if (! mySDI12.identification(0))
  {
    USB.printf((char*)"Id(0) = %s\n", mySDI12.buffer);
  }
  if (! mySDI12.identification(1))
  {
    USB.printf((char*)"Id(1) = %s\n", mySDI12.buffer);
  }
  mySDI12.end();
  USB.println();

  // OFF
#if (SENSOR_BOARD || POWER_BOARD)
  digitalWrite(DIGITAL7, LOW); // SDI12 power
#endif
#if POWER_BOARD
  digitalWrite(16, LOW); // 12V
  digitalWrite(15, LOW); // 5V
#endif

  // Sleep
  PWR.deepSleep("00:00:00:10", RTC_OFFSET, RTC_ALM1_MODE2, ALL_OFF); // 10s
  if( intFlag & RTC_INT ) { intFlag &= ~(RTC_INT); }
}
