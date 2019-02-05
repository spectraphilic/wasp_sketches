/**
 * Functions to read the Maxbotix (MB7389) sensor.
 *
 * TTL Serial.
 * Wiring: GND -> GND, V+ -> 3V3, pin5 -> AUX SERIAL 1RX
 */

#include "WaspUIO.h"


/** getMaxbotixSample
 *
 * Returns a single sample. If error will return -1. An error may happen if
 * there is a timeout (5s) or if the value is out of an acceptable range
 * (300-5000).
 */
int WaspUIO::getMaxbotixSample()
{
  const uint16_t timeout = 5000; // 5s
  const uint8_t port = 1; // Aux1
  const uint8_t bytes = 4; // Number of bytes to read
  char buffer[bytes]; // Store serial data
  int8_t i = -1;
  uint32_t t0 = millis();

  //return rand() % 5000;

  // Implement an automata, so we only have one loop for the timeout
  serialFlush(port);
  do
  {
    if (i == -1)
    {
      if (serialAvailable(port) && serialRead(port) == 'R')
      {
        i = 0;
      }
    }
    else if (i < bytes)
    {
      if (serialAvailable(port))
      {
        buffer[i] = serialRead(port);
        i++;
      }
    }
    else
    {
      break;
    }
  }
  while ((millis() - t0) < timeout);

  if (i == 4)
  {
    int sample = atoi(buffer);
    return (sample > 300 && sample < 5000)? sample: -1;
  }

  return -1;
}


/** readMaxbotixSerial
 *
 * Reads the sensor several times, returns the median and standard deviation.
 *
 * Parameters:
 *   uint16_t &median - output variable for the Median
 *   uint16_t &sd     - output variable for the Standard Deviation
 *   uint8_t samples  - Number of readings to sample (Default is set to 5)
 *
 * Returns: bool      - 0 if success, 1 if error
 */

bool WaspUIO::readMaxbotixSerial(uint16_t &median, uint16_t &std, uint8_t nsamples)
{
  const uint8_t port = 1;
  uint8_t max = nsamples * 2; // max number of readings, to avoid infinite loop
  uint16_t samples[nsamples];
  uint8_t i, j;
  uint16_t mean;

  // ON Sensor needs 3.3 voltage
  bool old_state = pwr_mb(1);

  Utils.setMuxAux1(); // check the manual to find out where you connect the sensor
  beginSerial(9600, port); // set baud rate to 9600

  // Get samples
  for (i=0, j=0; (i < max) && (j < nsamples); i++)
  {
    int sample = getMaxbotixSample();
    if (sample < 0)
    {
      delay(10);
    }
    else
    {
      samples[j] = (uint16_t) sample;
      j++;
      delay(1000);
    }
  }

  // OFF
  closeSerial(port);
  Utils.muxOFF1();
  pwr_mb(old_state);

  // Error
  if (j < nsamples)
  {
    warn(F("readMaxbotixSerial: fail to read MB7389"));
    return 1;
  }

  // Success
  for (j=0; j < nsamples; j++)
  {
    debug(F("readMaxbotixSerial: sample=%d"), samples[j]);
  }

  median = median_uint16(samples, nsamples);
  mean = mean_uint16(samples, nsamples);
  std = std_uint16(samples, nsamples, mean);
  info(F("readMaxbotixSerial: median=%d, std=%d"), median, std);

  return 0;
}
