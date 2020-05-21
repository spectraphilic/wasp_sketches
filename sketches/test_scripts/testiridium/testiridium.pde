#include <WaspUIO.h>
#include <time.h>

// Define WITH_IRIDIUM TRUE in libraries/WaspUIO/config.h for this sketch to work


void setup()
{
  int status;
  char version[10];

  cr_printf("setup()\n");

  status = UIO.iridium_start();

  if (status != ISBD_SUCCESS)
  {
    cr_printf("begin() error=%d\n", status);
  }
  else
  {
    status = iridium.getFirmwareVersion(version, sizeof version);
    if (status != ISBD_SUCCESS)
    {
      cr_printf("getFirmwareVersion() error=%d\n", status);
    }
    else
    {
      cr_printf("firmware=%s\n", version);
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
    cr_printf("getSignalQuality() error=%d\n", status);
  }
  else
  {
    cr_printf("quality=%d\n", quality);
  }

  status = iridium.getSystemTime(t);
  if (status != ISBD_SUCCESS)
  {
    cr_printf("getSystemTime error=%d\n", status);
  }
  else
  {
    cr_printf("time=%d-%d-%d %d:%d:%d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
  }

  status = iridium.sendSBDText("Hello");
  if (status != ISBD_SUCCESS)
  {
    cr_printf("sendSBDText(..) error=%d\n", status);
  }
  else
  {
    cr_printf("sendSBDText(..) Success!!\n");
  }

  delay(1000);
}
