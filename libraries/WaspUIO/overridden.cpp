/*
 * The functions override weak functions defined elsewhere (in Coroutines/
 * actually). Define behaviour specific to the project.
 */

#include "WaspUIO.h"


/**
 * Function to log waspmote activity.
 *
 * Parameters:
 * - log level: FATAL, ERROR, WARNING, INFO, DEBUG, TRACE
 * - formatted message: F string
 * - arguments for the formatted string
 *
 * Returns: bool             - true on success, false for error
 *
 * The output message max length is 149 chars (not counting null char).
 */

void vlog(loglevel_t level, const char* message, va_list args)
{
  size_t size = 150;
  char buffer[size];
  size_t max = size - 1;
  size_t len;
  uint32_t seconds;
  uint16_t ms;

  // (1) Prepare message
  // Timestamp
  seconds = UIO.getEpochTime(ms);
  sprintf(buffer, "%lu.%03u ", seconds, ms);
  len = strlen(buffer);
  // Level
  sprintf(buffer + len, "%s ", cr.loglevel2str(level));
  len = strlen(buffer);
  // Message
  vsnprintf(buffer + len, size - len - 1, message, args);
  len = strlen(buffer);
  // Newline
  if (len == max) { len--; } // Avoid buffer overflow
  buffer[len] = '\n';
  buffer[len + 1] = '\0';

  // (2) Print to USB
  if (UIO.flags & FLAG_LOG_USB)
  {
    USB.ON();
    USB.flush(); // XXX This fixes a weird bug with XBee
    USB.print(buffer);
    USB.OFF();
  }

  // (3) Print to log file
  if (UIO.hasSD && (UIO.flags & FLAG_LOG_SD))
  {
    UIO.startSD();
    if (UIO.openFile(UIO.logFilename, UIO.logFile, O_WRITE | O_CREAT | O_APPEND))
    {
      cr.println(F("%s"), cr.last_error);
    }
    else
    {
      if (UIO.append(UIO.logFile, buffer, strlen(buffer)))
      {
        cr.println(F("%s"), cr.last_error);
	UIO.logFile.close();
      }
    }

  }
}

void beforeSleep()
{
  UIO.stopSD();
}

void afterSleep()
{
#if USE_AGR
  if (UIO.isOn(UIO_PRESSURE))
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_PRESSURE);
  }
  if (UIO.isOn(UIO_LEAFWETNESS))
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_LEAF_WETNESS);
  }
  if (UIO.isOn(UIO_SENSIRION))
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_SENSIRION);
  }
#endif

#if USE_SDI
  if (UIO.isOn(UIO_SDI12))
  {
    mySDI12.forceHold(); // XXX
  }
#endif

#if USE_I2C
  if (UIO.isOn(UIO_I2C))
  {
    // XXX
  }
  if (UIO.isOn(UIO_1WIRE))
  {
    // XXX
  }
#endif
}
