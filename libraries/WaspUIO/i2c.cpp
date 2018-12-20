#include "WaspUIO.h"

// From Libelium
#include <BME280.h>

// Forks of SparkFun libraries
#include "AS726X.h"
#include "SparkFun_AS7265X.h"
#include "SparkFunMLX90614.h"
#include "SparkFunTMP102.h"
#include "SparkFun_VL53L1X_Arduino_Library.h"


void WaspUIO::i2c_scan()
{
  I2C.begin();
  cr.println(F("EEPROM   (%02x) %hhu"), I2C_ADDRESS_EEPROM, I2C.scan(I2C_ADDRESS_EEPROM));
  cr.println(F("RTC      (%02x) %hhu"), I2C_ADDRESS_WASP_RTC, I2C.scan(I2C_ADDRESS_WASP_RTC));
  cr.println(F("ACC      (%02x) %hhu"), I2C_ADDRESS_WASP_ACC, I2C.scan(I2C_ADDRESS_WASP_ACC));
  cr.println(F("BME280   (%02x) %hhu"), I2C_ADDRESS_Lemming_BME280, I2C.scan(I2C_ADDRESS_Lemming_BME280));

  cr.println(F("AS726X   (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_AS726X, I2C.scan(I2C_ADDRESS_LAGOPUS_AS726X));
  cr.println(F("BME280   (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_BME280, I2C.scan(I2C_ADDRESS_LAGOPUS_BME280));
  cr.println(F("MLX90614 (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_MLX90614, I2C.scan(I2C_ADDRESS_LAGOPUS_MLX90614));
  cr.println(F("TMP102   (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_TMP102, I2C.scan(I2C_ADDRESS_LAGOPUS_TMP102));
  cr.println(F("VL53L1X  (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_VL53L1X, I2C.scan(I2C_ADDRESS_LAGOPUS_VL53L1X));
  cr.println(F("0=success 1=no-state .. 5=protocol-error .. 10=busy .. 255=operation-in-progress"));
}



/** i2c_AS726X
 *
 * Returns: bool      - 0 if success, 1 if error
 */

bool WaspUIO::i2c_AS7263(uint8_t &temp, float &r, float &s, float &t, float &u, float &v, float &w)
{
  AS726X sensor;

  sensor.begin();
  sensor.takeMeasurements();

  temp = sensor.getTemperature();
  r = sensor.getCalibratedR();
  s = sensor.getCalibratedS();
  t = sensor.getCalibratedT();
  u = sensor.getCalibratedU();
  v = sensor.getCalibratedV();
  w = sensor.getCalibratedW();

  // Debug
  char str[20];
  debug(F("AS7263 temp=%d"), temp);
  debug(F("AS7263 r=%s"), Utils.float2String(r, str, 2));
  debug(F("AS7263 s=%s"), Utils.float2String(s, str, 2));
  debug(F("AS7263 t=%s"), Utils.float2String(t, str, 2));
  debug(F("AS7263 u=%s"), Utils.float2String(u, str, 2));
  debug(F("AS7263 v=%s"), Utils.float2String(v, str, 2));
  debug(F("AS7263 w=%s"), Utils.float2String(w, str, 2));

  return 0;
}

bool WaspUIO::i2c_AS7265(
  uint8_t &temp,
  float &A, float &B, float &C, float &D, float &E, float &F,
  float &G, float &H, float &I, float &J, float &K, float &L,
  float &R, float &S, float &T, float &U, float &V, float &W)
{
  AS7265X sensor;

  if (sensor.begin() == false)
  {
    return 1;
  }

  sensor.takeMeasurements();
  temp = sensor.getTemperature();
  A = sensor.getCalibratedA();
  B = sensor.getCalibratedB();
  C = sensor.getCalibratedC();
  D = sensor.getCalibratedD();
  E = sensor.getCalibratedE();
  F = sensor.getCalibratedF();
  G = sensor.getCalibratedG();
  H = sensor.getCalibratedH();
  I = sensor.getCalibratedI();
  J = sensor.getCalibratedJ();
  K = sensor.getCalibratedK();
  L = sensor.getCalibratedL();
  R = sensor.getCalibratedR();
  S = sensor.getCalibratedS();
  T = sensor.getCalibratedT();
  U = sensor.getCalibratedU();
  V = sensor.getCalibratedV();
  W = sensor.getCalibratedW();

  // DEBUG
  char str[20];
  debug(F("AS7263 temp=%d"), temp);
  debug(F("AS7265 A=%s"), Utils.float2String(A, str, 2));
  debug(F("AS7265 B=%s"), Utils.float2String(B, str, 2));
  debug(F("AS7265 C=%s"), Utils.float2String(C, str, 2));
  debug(F("AS7265 D=%s"), Utils.float2String(D, str, 2));
  debug(F("AS7265 E=%s"), Utils.float2String(E, str, 2));
  debug(F("AS7265 F=%s"), Utils.float2String(F, str, 2));
  debug(F("AS7265 G=%s"), Utils.float2String(G, str, 2));
  debug(F("AS7265 H=%s"), Utils.float2String(H, str, 2));
  debug(F("AS7265 I=%s"), Utils.float2String(I, str, 2));
  debug(F("AS7265 J=%s"), Utils.float2String(J, str, 2));
  debug(F("AS7265 K=%s"), Utils.float2String(K, str, 2));
  debug(F("AS7265 L=%s"), Utils.float2String(L, str, 2));
  debug(F("AS7265 R=%s"), Utils.float2String(R, str, 2));
  debug(F("AS7265 S=%s"), Utils.float2String(S, str, 2));
  debug(F("AS7265 T=%s"), Utils.float2String(T, str, 2));
  debug(F("AS7265 U=%s"), Utils.float2String(U, str, 2));
  debug(F("AS7265 V=%s"), Utils.float2String(V, str, 2));
  debug(F("AS7265 W=%s"), Utils.float2String(W, str, 2));

  return 0;
}


/** i2c_BME280
 *
 * Returns: bool      - 0 if success, 1 if error
 */

bool WaspUIO::i2c_BME280(float &temperature, float &humidity, float &pressure, uint8_t address)
{
  BME280 bme(address);

  if (bme.checkID() != 1)
  {
    return 1;
  }

  // Read the calibration registers
  // XXX Can this be done once in the setup?
  bme.readCalibration();

  // Read
  temperature = bme.getTemperature(BME280_OVERSAMP_1X, 0);
  humidity = bme.getHumidity(BME280_OVERSAMP_1X);
  pressure = bme.getPressure(BME280_OVERSAMP_1X, 0);

  // Debug
  char aux[20];
  debug(F("BME280 Temperature: %s Celsius Degrees"), Utils.float2String(temperature, aux, 2));
  debug(F("BME280 Humidity   : %s %%RH"), Utils.float2String(humidity, aux, 2));
  debug(F("BME280 Pressure   : %s Pa"), Utils.float2String(pressure, aux, 2));

  return 0;
}


/** i2c_MLX90614
 *
 * Returns: bool      - 0 if success, 1 if error
 */
bool WaspUIO::i2c_MLX90614(float &object, float &ambient)
{
  IRTherm therm;

  therm.begin();
  therm.setUnit(TEMP_F);

  if (therm.read())
  {
    return 1;
  }

  object = therm.object();
  ambient = therm.ambient();

  // Debug
  char str[20];
  debug(F("MLX 90614 object=%s"), Utils.float2String(object, str, 2));
  debug(F("MLX 90614 ambient=%s"), Utils.float2String(ambient, str, 2));

  return 0;
}


/** i2c_TMP102
 *
 * Returns: bool      - 0 if success, 1 if error
 */
bool WaspUIO::i2c_TMP102(float &temperature)
{
  TMP102 sensor0(I2C_ADDRESS_LAGOPUS_TMP102);

  sensor0.begin();  // Join I2C bus
  sensor0.wakeup();
  temperature = sensor0.readTempC();
  sensor0.sleep();

  // Debug
  char str[20];
  debug(F("TMP102 %s Celsius"), Utils.float2String(temperature, str, 2));

  return 0;
}


/** i2c_VL53L1X
 *
 * Returns: bool      - 0 if success, 1 if error
 *
 * This code is derived from
 * https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library/tree/master/examples/Example1_ReadDistance
 */

bool WaspUIO::i2c_VL53L1X(uint16_t &distance)
{
  VL53L1X distanceSensor;

  if (distanceSensor.begin() == false)
  {
    return 1;
  }

  // Poll for completion of measurement. Takes 40-50ms.
  while (distanceSensor.newDataReady() == false)
    delay(5);

  distance = distanceSensor.getDistance();
  debug(F("Distance(mm): %u"), distance);

  return 0;
}
