#include <WaspUIO.h>
#include "IridiumSBD.h"
#include "SoftwareSerial.h"
#include <time.h>


bool ISBDCallback()
{
  return true; // return false to cancel
}

void ISBDConsoleCallback(IridiumSBD *device, char c)
{
  USB.print(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c)
{
  USB.print(c);
}

SoftwareSerial ssIridium(18, 19); // RockBLOCK serial port on RX=18 TX=19
IridiumSBD modem(ssIridium); // SLEEP and RING pins not connected

void setup()
{
  int status;
  char version[10];

  UIO.pwr_5v(1);
  delay(200);

  modem.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);

  status = modem.begin(); // Wake up the 9602 and prepare it for communications.

  cr.println(F("begin status=%d"), status);
  status = modem.getFirmwareVersion(version, sizeof version);
  cr.println(F("firmware status=%d"), status);
  if (status == ISBD_SUCCESS) {
    cr.println(F("firmware=%s"), version);
  }
}


void loop()
{
  int status;
  int quality;
  struct tm t;

  status = modem.getSignalQuality(quality); // quality 0..5
  cr.println(F("quality status=%d"), status);
  if (status == ISBD_SUCCESS) {
    cr.println(F("quality=%d"), quality);
  }

  status = modem.getSystemTime(t);
  cr.println(F("time status=%d"), status);
  if (status == ISBD_SUCCESS) {
    cr.println(F("time=%d-%d-%d %d:%d:%d"), t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  }

  status = modem.sendSBDText("Hello");
  cr.println(F("send status=%d"), status);
  delay(1000);
}
