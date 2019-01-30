#include <WaspUIO.h>
#include <time.h>

// Define WITH_IRIDIUM TRUE in libraries/WaspUIO/config.h for this sketch to work


void setup()
{
  int status;
  char version[10];

  cr.println(F("setup()"));

  status = UIO.iridium_start();

  if (status != ISBD_SUCCESS)
  {
    cr.println(F("begin() error=%d"), status);
  }
  else
  {
    status = iridium.getFirmwareVersion(version, sizeof version);
    if (status != ISBD_SUCCESS)
    {
      cr.println(F("getFirmwareVersion() error=%d"), status);
    }
    else
    {
      cr.println(F("firmware=%s"), version);
    }
  }
}


void loop()
{
  int status;
  int quality;
  struct tm t;

  status = iridium.getSignalQuality(quality); // quality 0..5
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("getSignalQuality() error=%d"), status);
  }
  else
  {
    cr.println(F("quality=%d"), quality);
  }

  status = iridium.getSystemTime(t);
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("getSystemTime error=%d"), status);
  }
  else
  {
    cr.println(F("time=%d-%d-%d %d:%d:%d"), t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  }

  status = iridium.sendSBDText("Hello");
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("sendSBDText(..) error=%d"), status);
  }
  else
  {
    cr.println(F("sendSBDText(..) Success!!"));
  }

  delay(1000);
}
