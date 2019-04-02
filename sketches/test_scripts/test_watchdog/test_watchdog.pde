/*
 * Derived from
 * http://www.libelium.com/development/waspmote/examples/rtc-10-set-watchdog/
 */


void setup()
{
}


void test_absolute_or_relative()
{
  USB.ON();
  USB.println(F("Watchdog: test whether it's absolute or relative."));
  USB.println(F("- Some Libelium docs say it's relative."));
  USB.println(F("- The RTC DS1337C specification says it's absolute."));
  USB.println();

  // Set time to 19/04/02 12:07:20
  RTC.ON();
  RTC.setTime(19, 4, 2, 3, 12, 6, 20);
  USB.print(F("RTC time: ")); USB.println(RTC.getTime());

  // Set watchdog to 1 minute
  RTC.setWatchdog(1);
  USB.print(F("Watchdog ")); USB.println(RTC.getWatchdog());
  USB.println();

  // Change time to the past 19/04/01 00:00:55
  RTC.setTime(19, 4, 1, 2, 0, 0, 55);
  USB.print(F("RTC time: ")); USB.println(RTC.getTime());
  USB.print(F("Watchdog ")); USB.println(RTC.getWatchdog());
  RTC.OFF();

  // The question we want to know
  USB.println();
  USB.println(F("Will reboot at next 00s or wait for the absolute date?"));
  USB.println();

  // enter an infinite while
  while(1)
  {
    USB.print(F("Inside infinite loop. Time: "));
    USB.print(RTC.getTime());
    USB.println();
    delay(1000);
  }
}

void test_both_alarms()
{
  USB.ON();
  USB.println(F("What happens when both alarm 1 and 2 are triggered at the same time?"));
  USB.println();

  RTC.ON();
  RTC.setTime(19, 4, 2, 3, 13, 5, 40);                      // Time  : 19/04/02 13:05:40
  RTC.setAlarm1(2, 13, 6, 0, RTC_ABSOLUTE, RTC_ALM1_MODE2); // Alarm1: XX/XX/02 13:06:00
  RTC.setWatchdog(1);                                       // Alarm2: XX/XX/02 13:06:00

  USB.print(F("RTC time: ")); USB.println(RTC.getTime());
  USB.print(F("Alarm 1  ")); USB.println(RTC.getAlarm1());
  USB.print(F("Watchdog ")); USB.println(RTC.getWatchdog());
  USB.println();

  // Sleep!
  USB.println(F("Sleep.."));
  USB.println();
  PWR.sleep(ALL_OFF);

  // Awake!
  USB.ON();
  USB.println(F("Alarm 1 wins!!"));
  if (intFlag & RTC_INT)
  {
    USB.println(F("Clear flag"));
    intFlag &= ~(RTC_INT); // Clear flag
  }
  delay(5000);

}


void loop()
{
  //test_absolute_or_relative();
  test_both_alarms();
}
