
void setup()
{
  uint8_t serial[8];
  char buffer[99];

  for (uint8_t i=0; i < 8; i++) { serial[i] = _serial_id[i]; }
  Utils.hex2str(serial, buffer, 8);

  USB.ON();
  USB.print("Version : ");
  USB.println((int)_boot_version);
  USB.print("Serial  : ");
  USB.println(buffer);

  RTC.ON();
  PWR.setSensorPower(SENS_3V3,SENS_ON);
}

void loop()
{
  USB.print("Time    : ");
  USB.println(RTC.getTime());
  delay(1000);
}
