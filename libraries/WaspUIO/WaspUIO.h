/*! \def myLibrary_h
 \brief The library flag
 */
#ifndef WaspUIO_h
#define WaspUIO_h

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <inttypes.h>
#include <Coroutines.h>
#include <Queue.h>

#include "config.h"

#include <WaspFrame.h>
#include <WaspGPS.h>
#include <WaspAES.h>

#if WITH_XBEE
#include <WaspXBeeDM.h>
#endif

#if WITH_4G
#include <Wasp4G.h>
#endif

#if WITH_IRIDIUM
#include "IridiumSBD.h"
#include "HardwareSerial.h"
#endif

#if WITH_LORA
#include <WaspSX1272.h>
#endif


/******************************************************************************
 * Definitions & Declarations
 ******************************************************************************/

// EEPROM addresses used by the library
// 9 bytes available
#define EEPROM_UIO_RUN (EEPROM_START + 9) // Many bytes, leave room for future actions
                                          // (every action takes 2 bytes)
#define EEPROM_UIO_PIN (EEPROM_START + 50) // 2 bytes
#define EEPROM_UIO_APN (EEPROM_START + 52) // 30 bytes
#define EEPROM_UIO_PWD (EEPROM_START + 82) // 33 bytes
// 17 bytes available
#define EEPROM_UIO_VARS (EEPROM_START + 100) // 100 bytes reserved

#define GPS_NO 0
#define GPS_YES 1
#define GPS_4G 2

enum battery_type_t {
  BATTERY_LITHIUM,
  BATTERY_LEAD, // Unused
  BATTERY_REG3V3,
  BATTERY_LEN
};

enum board_type_t {
  BOARD_NONE,
  BOARD_LEMMING,
  BOARD_LEN
};

enum battery_t {
  BATTERY_LOW,
  BATTERY_MIDDLE,
  BATTERY_HIGH
};

enum lan_type_t {
  LAN_DISABLED,
  LAN_XBEE,
  LAN_LORA,
  LAN_LEN
};

enum wan_type_t {
  WAN_DISABLED,
  WAN_4G,
  WAN_IRIDIUM,
  WAN_LEN
};

enum run_t {
  RUN_BATTERY = 1,
  RUN_GPS,
  RUN_ACC,
  RUN_CTD10, // SDI-12
  RUN_DS2,
  RUN_DS1820, // OneWire
  RUN_BME280, // I2C
  RUN_MB, // TTL
  RUN_WS100, // SDI-12
  RUN_LAGOPUS_AS7263,
  RUN_LAGOPUS_AS7265,
  RUN_LAGOPUS_BME280,
  RUN_LAGOPUS_MLX90614,
  RUN_LAGOPUS_TMP102,
  RUN_LAGOPUS_VL53L1X,
  RUN_ATMOS, // SDI-12
  RUN_LAN, // Lora or XBee
  RUN_WAN, // 4G or Iridium
  RUN_LEN // Special value
};

const char EMPTY_STRING     [] PROGMEM = "";

const char RUN_BATTERY_NAME [] PROGMEM = "bat";             // 1 battery
const char RUN_GPS_NAME     [] PROGMEM = "gps";             // 2 gps
const char RUN_ACC_NAME     [] PROGMEM = "acc";             // 3 accelerometer
// Sensors
const char RUN_CTD10_NAME   [] PROGMEM = "ctd";             // 4 water
const char RUN_DS2_NAME     [] PROGMEM = "ds2";             // 5 wind
const char RUN_DS1820_NAME  [] PROGMEM = "ds1820";          // 6 temperature string
const char RUN_MB_NAME      [] PROGMEM = "mb";              // 8 sonar
const char RUN_WS100_NAME   [] PROGMEM = "ws100";           // 9 rain
// I2C
const char RUN_BME280_NAME  [] PROGMEM = "bme76";           // 7 atmospheric (internal)
const char RUN_LAGOPUS_AS7263_NAME   [] PROGMEM = "as7263"; // 10 spectrum
const char RUN_LAGOPUS_AS7265_NAME   [] PROGMEM = "as7265"; // 11 spectrum
const char RUN_LAGOPUS_BME280_NAME   [] PROGMEM = "bme";    // 12 atmospheric
const char RUN_LAGOPUS_MLX90614_NAME [] PROGMEM = "mlx";    // 13 infrared thermometer
const char RUN_LAGOPUS_TMP102_NAME   [] PROGMEM = "tmp";    // 14 digital temperature
const char RUN_LAGOPUS_VL53L1X_NAME  [] PROGMEM = "vl";     // 15 distance
// More
const char RUN_ATMOS_NAME   [] PROGMEM = "atmos";           // 16 wind
const char RUN_LAN_NAME     [] PROGMEM = "lan";             // 17 Network: LAN
const char RUN_WAN_NAME     [] PROGMEM = "wan";             // 18 Network: WAN

const char* const run_names[] PROGMEM = {
  EMPTY_STRING,
  RUN_BATTERY_NAME,
  RUN_GPS_NAME,
  RUN_ACC_NAME,
  RUN_CTD10_NAME,
  RUN_DS2_NAME,
  RUN_DS1820_NAME,
  RUN_BME280_NAME,
  RUN_MB_NAME,
  RUN_WS100_NAME,
  RUN_LAGOPUS_AS7263_NAME,
  RUN_LAGOPUS_AS7265_NAME,
  RUN_LAGOPUS_BME280_NAME,
  RUN_LAGOPUS_MLX90614_NAME,
  RUN_LAGOPUS_TMP102_NAME,
  RUN_LAGOPUS_VL53L1X_NAME,
  RUN_ATMOS_NAME,
  RUN_LAN_NAME,
  RUN_WAN_NAME,
};

enum var_indexes {
  VAR_BAT_IDX,
  VAR_BOARD_IDX,
  VAR_LOG_LEVEL_IDX,
  VAR_LOG_SD_IDX,
  VAR_LOG_USB_IDX,
  VAR_LAN_TYPE_IDX,
  VAR_WAN_TYPE_IDX,
  VAR_LORA_ADDR_IDX,
  VAR_LORA_MODE_IDX,
  VAR_XBEE_NETWORK_IDX,
  VAR_LAN_WAIT_IDX,
  VAR_LORA_DST_IDX,
};

const char VAR_BAT       [] PROGMEM = "bat";
const char VAR_BOARD     [] PROGMEM = "board";
const char VAR_LOG_LEVEL [] PROGMEM = "log.level";
const char VAR_LOG_SD    [] PROGMEM = "log.sd";
const char VAR_LOG_USB   [] PROGMEM = "log.usb";
const char VAR_LAN_TYPE  [] PROGMEM = "lan.type";
const char VAR_WAN_TYPE  [] PROGMEM = "wan.type";
const char VAR_LORA_ADDR [] PROGMEM = "lora.addr";
const char VAR_LORA_MODE [] PROGMEM = "lora.mode";
const char VAR_XBEE_NETWORK [] PROGMEM = "xbee.network";
const char VAR_LAN_WAIT  [] PROGMEM = "lan.wait";
const char VAR_LORA_DST  [] PROGMEM = "lora.dst";

const char VAR_BAT_HELP          [] PROGMEM = ": 0=lithium 1=lead 2=3V3 regulator";
const char VAR_BOARD_HELP        [] PROGMEM = ": 0=none 1=lemming";
const char VAR_LOG_LEVEL_HELP    [] PROGMEM = ": 0=off 1=fatal 2=error 3=warn 4=info 5=debug 6=trace";
const char VAR_LOG_FLAG_HELP     [] PROGMEM = ": 0/1";
const char VAR_LAN_TYPE_HELP     [] PROGMEM = ": 0=disabled 1=xbee 2=lora";
const char VAR_WAN_TYPE_HELP     [] PROGMEM = ": 0=disabled 1=4g 2=iridium";
const char VAR_LORA_ADDR_HELP    [] PROGMEM = ": 1-255 (1=Gateway)";
const char VAR_LORA_MODE_HELP    [] PROGMEM = ": 1-10 (1 = higher range, 10 = lower energy)";
const char VAR_XBEE_NETWORK_HELP [] PROGMEM = ": 0=Finse 1=<unused> 2=Broadcast 3=Pi@UiO 4=Pi@Finse 5=Pi@Spain";
const char VAR_LAN_WAIT_HELP     [] PROGMEM = ": 0-255 seconds to keep LAN open (zero means use default)";
const char VAR_LORA_DST_HELP     [] PROGMEM = ": destination address";

const char* const var_names[] PROGMEM = {
  VAR_BAT,
  VAR_BOARD,
  VAR_LOG_LEVEL,
  VAR_LOG_SD,
  VAR_LOG_USB,
  VAR_LAN_TYPE,
  VAR_WAN_TYPE,
  VAR_LORA_ADDR,
  VAR_LORA_MODE,
  VAR_XBEE_NETWORK,
  VAR_LAN_WAIT,
  VAR_LORA_DST,
};

const char* const var_help[] PROGMEM = {
  VAR_BAT_HELP,
  VAR_BOARD_HELP,
  VAR_LOG_LEVEL_HELP,
  VAR_LOG_FLAG_HELP,
  VAR_LOG_FLAG_HELP,
  VAR_LAN_TYPE_HELP,
  VAR_WAN_TYPE_HELP,
  VAR_LORA_ADDR_HELP,
  VAR_LORA_MODE_HELP,
  VAR_XBEE_NETWORK_HELP,
  VAR_LAN_WAIT_HELP,
  VAR_LORA_DST_HELP,
};


#define ADD_SENSOR(type, ...) \
  if (UIO.addSensor(type, ## __VA_ARGS__) == -1)\
  {\
    UIO.createFrame();\
    UIO.addSensor(type, ## __VA_ARGS__);\
  }


/*
 * Actions
 */

enum action_t {
  action_disabled,
  action_minutes, // Run every M minutes
  action_hours,   // Run every H hours at M minutes
};

struct Action {
  action_t type;
  uint8_t hour;
  uint8_t minute;
};


/*
 * Network
 */

/* XBee stuff */
struct XBee {
  const char *name;
  uint8_t panid[2];
  uint8_t channel;
  char rx_address[17];
};

// NOTE The low bit of the xbee id (panID) must start at zero and increment one
// by one. This is because the low bit is used as an index to this same array.

const char XBEE_FINSE     [] PROGMEM = "Finse";
const char XBEE_UNUSED    [] PROGMEM = "unused";
const char XBEE_BROADCAST [] PROGMEM = "Broadcast";
const char XBEE_PI_UIO    [] PROGMEM = "Pi UiO";
const char XBEE_PI_FINSE  [] PROGMEM = "Pi Finse";
const char XBEE_PI_CS     [] PROGMEM = "Pi CS";

const XBee xbees[] PROGMEM = {
  {XBEE_FINSE,     {0x12, 0x00}, 0x0F, "0013A20040779085"},
  {XBEE_UNUSED,    {0x12, 0x01}, 0x0F, "0000000000000000"}, // Available
  {XBEE_BROADCAST, {0x12, 0x02}, 0x0F, "000000000000FFFF"},
  {XBEE_PI_UIO,    {0x12, 0x03}, 0x0F, "0013A200416B1B9B"},
  {XBEE_PI_FINSE,  {0x12, 0x00}, 0x0F, "0013A20040779085"},
  {XBEE_PI_CS,     {0x12, 0x05}, 0x0F, "0013A200412539D3"}, // Office of jdavid
};
const uint8_t xbee_len = sizeof(xbees) / sizeof(XBee);

/******************************************************************************
 * Class
 ******************************************************************************/

//! WaspUIO Class
/*!
  defines all the variables and functions used
 */
class WaspUIO
{

private:

public:

  // Utilities

  // Like Arduino's EEPROM.update, it writes the given value only if different
  // from the value already saved.
  bool updateEEPROM(int address, uint8_t value);
  bool updateEEPROM(int address, uint16_t value);
  bool updateEEPROM(int address, uint32_t value);
  bool writeEEPROM(int address, char* src, size_t size);
  char* readEEPROM(int address, char* dst, size_t size);

  // Math and algo utilities
  int8_t index(const char* const list[], size_t size, const char* str);
  void sort_uint16(uint16_t* array, uint8_t size);
  uint16_t median_uint16(uint16_t* array, uint8_t size);
  uint16_t mean_uint16(uint16_t* array, uint8_t size);
  uint16_t std_uint16(uint16_t* array, uint8_t size, uint16_t mean);

  // Lifecycle
  uint16_t nloops;
  void boot();
  void bootConfig();
  void bootDetect();
  void onLoop();

  uint32_t __nextAlarmHelper(uint32_t now, uint32_t size, uint8_t factor=1, uint8_t offset=0, uint8_t step=1);
  uint32_t nextAlarm();
  void reboot();
  void deepSleep();

  // Actions (tasks)
  Action actions[RUN_LEN];
  bool action(uint8_t n, ...);

  // General configuration
  char name[17];
  board_type_t boardType = BOARD_LEN; // Defaults to undefined

  // Variables configured with the 'var' command
  uint8_t log_sd;
  uint8_t log_usb;
  lan_type_t lan_type;
  uint8_t lan_wait;
  wan_type_t wan_type;
  uint8_t lora_addr;
  uint8_t lora_mode;
  uint8_t lora_dst;
  uint8_t xbee_network;

  // Power related
  battery_type_t batteryType = BATTERY_LEN; // Defaults to undefined
  float batteryVolts;
  uint8_t batteryLevel;
  battery_t battery;
  uint8_t cooldown; // Reduces frequency of action, depends on battery
  // Power: State management
  const uint8_t PWR_MAIN = 1;
  const uint8_t PWR_3V3 = 2;
  const uint8_t PWR_5V = 4;
  const uint8_t PWR_LEAD_VOLTAGE = 8; // Unused
  const uint8_t PWR_MB = 16;
  const uint8_t PWR_I2C = 32;
  const uint8_t PWR_1WIRE = 64;
  const uint8_t PWR_SDI12 = 128;
  uint8_t pwr_state = 0;
  uint8_t pwr_saved_state;
  void saveState();
  void loadState();
  bool _setState(uint8_t device, bool new_state);
  // Power board
  bool pwr_switch(uint8_t device, uint8_t pin, bool new_state);
  bool pwr_main(bool new_state);
  bool pwr_3v3(bool new_state);
  bool pwr_5v(bool new_state);
  void setSensorPower(uint8_t type, uint8_t mode);
  // Sensor board
  bool pwr_mb(bool new_state);
  bool pwr_i2c(bool new_state);
  bool pwr_1wire(bool new_state);
  bool pwr_sdi12(bool new_state);
  // Battery
  void readBattery();

  // Time
  // To keep time without calling RCT each time
  uint32_t _epoch;         // seconds since the epoch
  uint32_t _epoch_millis;  // millis taken at _epoch
  uint32_t _epoch_minutes; // ref time for actions, set once in the loop
  uint32_t _loop_start;    // millis, set once in the loop
  uint8_t saveTime();
  uint8_t saveTimeToSD();
  uint8_t setTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
  uint8_t setTime(uint32_t time);
  void loadTime();
  unsigned long getEpochTime();
  unsigned long getEpochTime(uint16_t &ms);
  uint8_t setTimeFromNetwork();

  // SD
  bool hasSD;
  const char* archive_dir = "/data";
  const char* logFilename = "LOG.TXT";
  const char* timeFilename = "TIME.TXT"; //
  SdFile logFile;
  uint8_t ack_wait;
  int baselayout();
  void startSD();
  void stopSD();
  int readline(SdFile &file);
  int8_t walk(SdBaseFile &root,
              bool (*before_cb)(SdBaseFile &me, char* name),
              bool (*file_cb)(SdBaseFile &parent, char* name),
              bool (*after_cb)(SdBaseFile &me, char* name));
  static void dateTime(uint16_t* date, uint16_t* time);

  // Network
  void networkInit();
  int rssi, snr;
  #if WITH_XBEE
  // Network: Xbee
  XBee xbee;
  void xbeeInit();
  int xbeeSend(const char* dst, const char* msg);
  int xbeeQuality();
  #endif
  #if WITH_LORA
  // Network: Lora
  int loraStart();
  void loraStop();
  int loraInit();
  int loraSend(uint8_t dst, const char* msg, bool ack=false);
  int loraQuality();
  uint8_t lora_dst2;
  #endif
  #if WITH_4G
  // Network: 4G
  uint16_t pin; // Pin for 4G module
  void _4GInit();
  uint8_t _4GStart();
  uint8_t _4GStop();
  uint8_t _4GGPS();
  int _4GPing();
  uint8_t setTimeFrom4G(const char* value=NULL);
  #endif
  #if WITH_IRIDIUM
  char iridium_fw[9]; // firmware version
  void iridiumInit();
  int iridium_start();
  int iridium_stop();
  int iridium_ping();
  #endif
  #if WITH_CRYPTO
  // Crypto
  char password[33]; // To encrypt frames
  #endif

  // GPS
  uint8_t hasGPS;
  int8_t gps(bool setTime, bool getPosition);

  // Frames
  uint8_t getDataFilename(char* filename, uint8_t src, uint8_t year, uint8_t month, uint8_t date);
  void createFrame(bool discard=false);
  int8_t addSensor(uint8_t type, ...);
  uint8_t addSensorValue(uint8_t size, void* value); // XXX Should be private
  uint8_t addSensorValue(float value);
  uint8_t addSensorValue(int8_t value);
  uint8_t addSensorValue(uint8_t value);
  uint8_t addSensorValue(int16_t value);
  uint8_t addSensorValue(uint16_t value);
  uint8_t addSensorValue(int32_t value);
  uint8_t addSensorValue(uint32_t value);
  uint8_t getSequence(uint8_t *p);
  uint16_t parseFrame(uint8_t *p, uint16_t max_length);
  uint8_t saveFrames(uint8_t src, uint8_t *buffer, uint16_t max_length);
  uint8_t saveFrame(uint8_t src, uint8_t *buffer, uint16_t length);
  uint8_t saveFrame();
  int readFrame(uint8_t &n);
  uint8_t frameSize;
  uint16_t payloadSize;
  void setFrameSize();

  // CLI
  void clint();
  const char* pprint4G(char* dst, size_t size);
  const char* pprintAction(char* dst, size_t size, uint8_t action, const __FlashStringHelper* name);
  const char* pprintActions(char* dst, size_t size);
  const char* pprintBattery(char* dst, size_t size);
  const char* pprintBoard(char* dst, size_t size);
  const char* pprintFrames(char* dst, size_t size);
  const char* pprintIridium(char* dst, size_t size);
  const char* pprintLog(char* dst, size_t size);
  const char* pprintLora(char* dst, size_t size);
  const char* pprintSerial(char* str, size_t size);
  const char* pprintTime(char* dst, size_t size);
  const char* pprintXBee(char* dst, size_t size);

  // Sensors
  int getMaxbotixSample();
  uint8_t readMaxbotixSerial(int samples[], uint8_t nsamples);
  uint8_t readDS18B20(int values[], uint8_t max);

  // I2C
  void i2c_scan();
  bool i2c_acc(int &x, int &y, int &z);
  bool i2c_AS7263(byte &temp, float &r, float &s, float &t, float &u, float &v, float &w);
  bool i2c_AS7265(uint8_t &temp,
                  float &A, float &B, float &C, float &D, float &E, float &F,
                  float &G, float &H, float &I, float &J, float &K, float &L,
                  float &R, float &S, float &T, float &U, float &V, float &W);
  bool i2c_BME280(float &temperature, float &humidity, float &pressure, uint8_t address=I2C_ADDRESS_Lemming_BME280);
  bool i2c_MLX90614(float &object, float &ambient);
  bool i2c_TMP102(float &temperature);

  uint8_t i2c_VL53L1X(int distances[], uint8_t nbsample);

  // SDI-12
  char sdi_read_address();
  const char* sdi_identify(uint8_t address);
  uint8_t sdi_set_address(uint8_t current_address, uint8_t new_address);
};


extern WaspUIO UIO;
extern FIFO fifo;
#if WITH_IRIDIUM
extern IridiumSBD iridium;
extern LIFO lifo;
#endif


void vlog(loglevel_t level, const char* message, va_list args);


/*
 * Commands
 */
enum cmd_status_t {
  cmd_exit,
  cmd_ok,
  cmd_quiet,
  cmd_bad_input,
  cmd_unavailable,
  cmd_error,
};

uint8_t _getFlag(const char*);
uint8_t _getPin(uint8_t);

#define COMMAND(name) cmd_status_t name(const char *str)
cmd_status_t exeCommands(char *str, bool interactive);
COMMAND(exeCommand);
COMMAND(cmd4G_APN);
COMMAND(cmd4G_GPS);
COMMAND(cmd4G_Pin);
COMMAND(cmdAck);
COMMAND(cmdCat);
COMMAND(cmdCatx);
COMMAND(cmdExit);
COMMAND(cmdFormat);
COMMAND(cmdGPS);
COMMAND(cmdHelp);
COMMAND(cmdI2C);
COMMAND(cmdLora);
COMMAND(cmdLs);
COMMAND(cmdMB);
COMMAND(cmdName);
COMMAND(cmd1WireRead);
COMMAND(cmd1WireScan);
COMMAND(cmdPassword);
COMMAND(cmdPing);
COMMAND(cmdPrint);
COMMAND(cmdReboot);
COMMAND(cmdRm);
COMMAND(cmdRun);
COMMAND(cmdSDI12);
COMMAND(cmdTail);
COMMAND(cmdTime);
COMMAND(cmdVar);


/*
 * Actions. Base components to implement a flexible and efficient main loop.
 *
 * Problem(s):
 *
 * - Power efficiency. The operation of the device is defined by the long
 *   delays between some actions (e.g. wait 500ms for power to stabilize before
 *   measuring.  Sometimes this delay is variable: in the SDI-12 protocol the
 *   measure command returns the number of seconds to wait before the data is
 *   available.
 *
 *   If writing the program sequentially this would lead to a long loop, keeping
 *   awake the device for a long time, wasting battery.
 *
 *   Solution: Split functions where there are big delays into smaller ones
 *   (called actions), and implement the main loop in such a way that we can do
 *   something else instead of just waiting.
 *
 * - Flexibility. Every device will have a different set of sensors attached.
 *   This may lead to have different programs for every configuration, or a
 *   difficult to maintain source code with intricate if/else.
 *
 *   Solution: make the loop configuration driven, where an action is scheduled
 *   or not based on configuration. An action may schedule another action.
 *
 * Components:
 *
 * - Actions, free functions that do something, detailed below.
 * - The "actions" table, associates to each action an info string to be logged.
 * - The "action_type", an enum with one value per action, identifies the action.
 * - The "actionsQ" array, for every action tells if/when it will be run.
 *
 * An action is a free function that takes no parameters and returns:
 *
 *    < -2: error
 *      -1: stop condition, finished with success
 *   >=  0: number of ms to wait before calling me again
 *
 * These free functions are prefixed with "action" to tell them apart.
 *
 * We use free functions because it is easier to make pointers to free
 * functions thant to member functions. Alternatively we could have used static
 * member functions.
 *
 * IMPORTANT The action enums (ACTION_) are used as indexes into both arrays
 * (actions and actionsQ), this is the way the identify an action. This means
 * that the order of the enums must match the order in the action table.
 *
 */

CR_TASK(taskMain);
// Sensors: internal
CR_TASK(taskHealthFrame);
// Sensors: external
CR_TASK(taskSensors);
// SDI-12
CR_TASK(taskSdi);
CR_TASK(taskSdiCtd10);
CR_TASK(taskSdiDs2);
CR_TASK(taskSdiAtmos);
// Externally powered devices
CR_TASK(taskExt);
CR_TASK(taskSdiWS100); // Uses SDI-12
// 3V3
CR_TASK(task1Wire);
CR_TASK(taskI2C);
CR_TASK(taskI2C_ACC);
CR_TASK(taskI2C_BME280_76);
CR_TASK(taskI2C_AS7263);
CR_TASK(taskI2C_AS7265);
CR_TASK(taskI2C_BME280);
CR_TASK(taskI2C_MLX90614);
CR_TASK(taskI2C_TMP102);
CR_TASK(taskI2C_VL53L1X);
CR_TASK(taskTTL);
// Network
CR_TASK(taskNetwork4G);
CR_TASK(taskNetworkIridium);
CR_TASK(taskNetworkXBee);
CR_TASK(taskNetworkXBeeSend);
CR_TASK(taskNetworkXBeeReceive);
CR_TASK(taskNetworkLora);
CR_TASK(taskNetworkLoraSend);
CR_TASK(taskNetworkLoraReceive);
// GPS
CR_TASK(taskGPS);
CR_TASK(taskGPS4G);
// For testing purposes
CR_TASK(taskSlow);


#define SENSOR_BAT           52
#define SENSOR_GPS           53
#define SENSOR_TST          123
#define SENSOR_DS18B20      203
#define SENSOR_GPS_ACCURACY 205
#define SENSOR_VOLTS        206
#define SENSOR_WS100        207
#define SENSOR_BME_76       209
#define SENSOR_BME_77       210
#define SENSOR_MLX90614     211
#define SENSOR_TMP102       212
#define SENSOR_VL53L1X      213
#define SENSOR_MB73XX       214
#define SENSOR_CTD10        216
#define SENSOR_DS2          217
#define SENSOR_ATMOS        218

#endif
