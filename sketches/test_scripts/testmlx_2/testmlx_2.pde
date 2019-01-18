#include <SparkFunMLX90614.h>

IRTherm therm;
char buffer[99];

void setup()
{
  uint8_t serial[8];

  for (uint8_t i=0; i < 8; i++) { serial[i] = _serial_id[i]; }
  Utils.hex2str(serial, buffer, 8);

  USB.ON();
  USB.print("Version : ");
  USB.println((int)_boot_version);
  USB.print("Serial  : ");
  USB.println(buffer);

  RTC.ON();
  PWR.setSensorPower(SENS_3V3,SENS_ON);
  delay(500);

  therm.begin(); // Initialize thermal IR sensor
  therm.setUnit(TEMP_C);
  if (therm.readID())
  {
    USB.print("MLX ID: 0x");
    USB.print(therm.getIDH());
    USB.println(therm.getIDL());
  }
}

void loop()
{
  float object;
  float ambient;

  USB.print("Time       : ");
  USB.println(RTC.getTime());

  if (therm.read())
  {
    object = therm.object();
    ambient = therm.ambient();

    USB.print("MLX object : ");
    Utils.float2String(object, buffer, 2);
    USB.println(buffer);

    USB.print("MLX ambient: ");
    Utils.float2String(ambient, buffer, 2);
    USB.println(buffer);
  }

  delay(1000);
}
