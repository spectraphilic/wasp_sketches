
void setup()
{
}

void loop()
{
  uint8_t serial[8];
  char buffer[99];

  for (uint8_t i=0; i < 8; i++) { serial[i] = _serial_id[i]; }
  Utils.hex2str(serial, buffer, 8);
  USB.println(buffer);
  delay(200);
}
