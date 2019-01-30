#include <WaspUIO.h>
#include <time.h>

// Define WITH_IRIDIUM TRUE in libraries/WaspUIO/config.h for this sketch to work


bool ISBDCallback()
{
  return true; // return false to cancel
}

void ISBDConsoleCallback(IridiumSBD *device, char c)
{
  //USB.print(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c)
{
  //USB.print(c);
}

IridiumSBD modem(Serial1, DIGITAL4); // RING pins not connected

void setup()
{
  int status;
  char version[10];

  Utils.setMuxAux2();
  beginSerial(19200, UART1);

  modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
  status = modem.begin(); // Wake up the 9602 and prepare it for communications.
  //delay(500);

  if (status != ISBD_SUCCESS)
  {
    cr.println(F("begin() error=%d"), status);
  }
  else
  {
    status = modem.getFirmwareVersion(version, sizeof version);
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

  status = modem.getSignalQuality(quality); // quality 0..5
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("getSignalQuality() error=%d"), status);
  }
  else
  {
    cr.println(F("quality=%d"), quality);
  }

  status = modem.getSystemTime(t);
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("getSystemTime error=%d"), status);
  }
  else
  {
    cr.println(F("time=%d-%d-%d %d:%d:%d"), t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  }

  status = modem.sendSBDText("Hello");
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
