/*
Test of serial comunication
 */



char dummy[6] = "hello";
char rxBuffer[50];
uint8_t i = 0;

void setup()
{
  memset(rxBuffer, 0x00, sizeof(rxBuffer));
  Utils.setMuxAux2();
  beginSerial(115200,1);
  serialFlush(1);
  delay(100);
  printString(dummy,  1);
  delay(100);
}
void loop()
{
  i = 0;

  while(serialAvailable(1) > 0)
  {
    rxBuffer[i] = serialRead(1);

    i++;

    if (i > 50)
    {

      break;

    }

  }

  // print data

  USB.print("Sent data: ");

  USB.println(dummy);

  USB.print("Received data: ");

  USB.println(rxBuffer);

  if (strcmp(dummy,rxBuffer) == 0)
  {

    USB.println("Test OK");

  }

  else
  {

    USB.print("Test failed");

  }

  // stop here

  while(1);

}

