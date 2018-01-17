#include <WaspXBeeDM.h>

void setup()
{
  xbeeDM.ON();
  xbeeDM.writeValues();
  USB.println("writeValues DONE");
  xbeeDM.OFF();
  Utils.blinkRedLED(500, 3);
}

void loop()
{
}
