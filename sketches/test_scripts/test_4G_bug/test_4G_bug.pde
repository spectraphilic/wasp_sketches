#include <WaspUIO.h>


void setup()
{
  USB.ON();
  UIO.boot();
  USB.println();

  // SD
  UIO.startSD();
  RTC.ON();
}


void print_values(const char* value)
{
  const char *format = "%2hhu%*c%2hhu%*c%2hhu%*c%2hhu%*c%2hhu%*c%2hhu%hhd\"";
  uint8_t year, month, day, hour, minute, second;
  int8_t timezone;

  sscanf(value, format, &year, &month, &day, &hour, &minute, &second, &timezone);
  cr_printf("20%02d-%02d-%02d %02d:%02d:%02d %02d\n", year, month, day, hour, minute, second, timezone);

  uint32_t epoch = RTC.getEpochTime(year, month, day, hour, minute, second);
  epoch = epoch - (timezone * 15 * 60);
  //cr_printf("Epoch: %lu\n", epoch);

  timestamp_t tm;
  RTC.breakTimeAbsolute(epoch, &tm);
  cr_printf("20%02d-%02d-%02d %02d:%02d:%02d\n", tm.year, tm.month, tm.date, tm.hour, tm.minute, tm.second);
}


void loop()
{
  char buffer[150];
  size_t size = sizeof(buffer);

  //uint8_t status = UIO.setTimeFrom4G("19/04/15,16:55:00+08");
  UIO.setTimeFrom4G("19/04/11,00:00:00+08");
  cr_printf("After     : %s\n", UIO.pprintTime(buffer, size));

  cr_printf("\n");

  UIO.setTimeFrom4G("19/04/10,24:00:00+08");
  cr_printf("After     : %s\n", UIO.pprintTime(buffer, size));


/*
  print_values("19/04/11,00:00:00+08");
  cr_printf("\n");
  print_values("19/04/10,24:00:00+08");
  cr_printf("\n");
*/

  cr_printf("================\n");
  delay(20000);
}
