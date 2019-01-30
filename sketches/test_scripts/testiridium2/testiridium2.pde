#include <WaspClasses.h>


void setup()
{
  USB.ON();

  Utils.setMuxAux2();
  beginSerial(19200, UART1);

  // Switch on
  pinMode(DIGITAL4, OUTPUT);
  digitalWrite(DIGITAL4, HIGH);
  delay(500);
}


void loop()
{
  USB.println("loop");
  char buffer[100];

  printString("AT+CGSN\r", UART1);
  delay(50);

  uint8_t i = 0;
  while (serialAvailable(UART1))
  {
    int x = serialRead(UART1); // -1: error
    if (x == -1)
    {
      USB.println("serialRead ERROR");
      break;
    }
    buffer[i++] = (char)x;
  }
  buffer[i] = '\0';

  USB.println(buffer);
  delay(2000);

//closeSerial(UART1); // close UART
//Utils.setMux(LOW, LOW); // Disable UART1's multiplexer
//Utils.muxOFF1();
}



