#include "WaspUIO.h"

// From Libelium
#include <BME280.h>

// Forks of SparkFun libraries
#include "AS726X.h"
#include "SparkFun_AS7265X.h"
#include "SparkFunMLX90614.h"
#include "SparkFunTMP102.h"
#include "SparkFun_TMP117.h"
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
  cr.println(F("TMP1XX   (%02x) %hhu"), I2C_ADDRESS_TMP117, I2C.scan(I2C_ADDRESS_TMP117));
  cr.println(F("VL53L1X  (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_VL53L1X, I2C.scan(I2C_ADDRESS_LAGOPUS_VL53L1X));
  cr.println(F("0=success 1=no-state .. 5=protocol-error .. 10=busy .. 255=operation-in-progress"));
}


/*
 * Built-in accelerometer
 * IS3331LDH, by STMicroelectronics
 * H3LIS331DL ??
 */

bool WaspUIO::i2c_acc(int &x, int &y, int &z)
{
  bool err = 0;

  ACC.ON(FS_2G);
  //ACC.boot();
  //ACC.getStatus();

  // The high-pass filter is by default 00, controlled by CTRL2
  //cr.println(F("CTRL2 %d"), ACC.getCTRL2());
  //delay(100); // Needed for the accelerometers high-pass filter

  // Check
  if (ACC.check() != 0x32)
  {
    err = 1;
    error(F("ACC.check() failure"));
    goto exit;
  }

  // Read (XXX Check ACC.flag?)
  x = ACC.getX();
  y = ACC.getY();
  z = ACC.getZ();
  info(F("ACC x=%d, y=%d, z=%d"), x, y, z);

exit:
  // Off
  // Modes: ON, POWER_DOWN, LOW_POWER_{1-5}
  ACC.setMode(ACC_POWER_DOWN); // .OFF closes I2C as well, we don't want that
  return err;
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
  therm.setUnit(TEMP_C);

  if (! therm.read())
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


/** i2c_TMP102 & i2c_TMP117
 *
 * Returns: bool      - 0 if success, 1 if error
 */
bool WaspUIO::i2c_TMP102(float &temperature)
{
  TMP102 sensor(I2C_ADDRESS_LAGOPUS_TMP102);

  sensor.begin();  // Join I2C bus
  sensor.wakeup();
  temperature = sensor.readTempC();
  sensor.sleep();

  // Debug
  char str[20];
  debug(F("TMP102 %s Celsius"), Utils.float2String(temperature, str, 2));

  return 0;
}


bool WaspUIO::i2c_TMP117(double &temperature)
{
  TMP117 sensor;

  if (sensor.begin(I2C_ADDRESS_TMP117) == false)
    return 1;

  //sensor.wakeup();
  temperature = sensor.readTempC();
  //sensor.sleep();

  // Debug
  char str[20];
  debug(F("TMP117 %s Celsius"), Utils.float2String(temperature, str, 2));

  return 0;
}

/** i2c_VL53L1X
 *
 * Returns: bool      - 0 if success, 1 if error
 *
 * This code is derived from
 * https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library/tree/master/examples/Example1_ReadDistance
 */

uint8_t WaspUIO::i2c_VL53L1X(int distances[], uint8_t nbsample)
{

  //modifying code to get multiple measurements (like in the case of ds18b10)
  VL53L1X distanceSensor;
  uint8_t max_sample = 99;
  uint8_t n = 0;

  if (distanceSensor.begin() == false)
  {
    return 1;
  }
  if(nbsample>=max_sample)
  {
    nbsample = max_sample;
  }

  while(n < nbsample){
    // Poll for completion of measurement. Takes 40-50ms.
    while (distanceSensor.newDataReady() == false)
      delay(5);

      int16_t tmp = distanceSensor.getDistance();
      debug(F("Distance %u (mm): %u"), n, tmp);
      distances[n++] = tmp;
      delay(200);
  }
  return n;
}
