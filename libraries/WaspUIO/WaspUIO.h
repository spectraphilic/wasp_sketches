/*! \def myLibrary_h
 \brief The library flag
 */
#ifndef WaspUIO_h
#define WaspUIO_h

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "config.h"

#include <inttypes.h>
#include <Coroutines.h>

#include <WaspFrame.h>
#include <WaspGPS.h>
#include <WaspAES.h>

#if WITH_XBEE
#include <WaspXBeeDM.h>
#endif

#if WITH_4G
#include <Wasp4G.h>
#endif



/******************************************************************************
 * Definitions & Declarations
 ******************************************************************************/

// EEPROM addresses used by the library
#define EEPROM_UIO_FLAGS (EEPROM_START + 0)
#define EEPROM_UIO_XBEE (EEPROM_START + 1)   // 2 bytes to store the xbee id
#define EEPROM_UIO_BOARD_TYPE (EEPROM_START + 3)
#define EEPROM_UIO_BATTERY_TYPE (EEPROM_START + 4)
#define EEPROM_UIO_NETWORK_TYPE (EEPROM_START + 5)
// 1 byte available
#define EEPROM_UIO_LOG_LEVEL (EEPROM_START + 7) // 1 byte for the log level
// 1 byte available
#define EEPROM_UIO_RUN (EEPROM_START + 9) // Many bytes, leave room for future actions
                                          // (every action takes 2 bytes)
#define EEPROM_UIO_PIN (EEPROM_START + 50) // 2 bytes
#define EEPROM_UIO_APN (EEPROM_START + 52) // 30 bytes
#define EEPROM_UIO_PWD (EEPROM_START + 82) // 33 bytes

enum battery_type_t {
  BATTERY_LITHIUM = 1,
  BATTERY_LEAD,
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

enum network_type_t {
  NETWORK_XBEE,
  NETWORK_4G,
  NETWORK_LEN
};

enum run_t {
  RUN_NETWORK,
  RUN_BATTERY,
  RUN_GPS,
  RUN_FREE_1, // Available
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
  RUN_LEN // Special value
};

const char RUN_NETWORK_NAME [] PROGMEM = "net";            // 0 network
const char RUN_BATTERY_NAME [] PROGMEM = "bat";            // 1 battery
const char RUN_GPS_NAME     [] PROGMEM = "gps";            // 2 gps
const char RUN_FREE_1_NAME  [] PROGMEM = "";
const char RUN_CTD10_NAME   [] PROGMEM = "ctd";            // 4 water
const char RUN_DS2_NAME     [] PROGMEM = "ds2";            // 5 wind
const char RUN_DS1820_NAME  [] PROGMEM = "ds1820";         // 6 temperature string
const char RUN_MB_NAME      [] PROGMEM = "mb";             // 8 sonar
const char RUN_WS100_NAME   [] PROGMEM = "ws100";          // 9 rain
// I2C
const char RUN_BME280_NAME  [] PROGMEM = "bme76";        // 7 atmospheric (internal)
const char RUN_LAGOPUS_AS7263_NAME   [] PROGMEM = "as7263";  // 10 spectrum
const char RUN_LAGOPUS_AS7265_NAME   [] PROGMEM = "as7265";  // 11 spectrum
const char RUN_LAGOPUS_BME280_NAME   [] PROGMEM = "bme"; // 12 atmospheric
const char RUN_LAGOPUS_MLX90614_NAME [] PROGMEM = "mlx"; // 13 infrared thermometer
const char RUN_LAGOPUS_TMP102_NAME   [] PROGMEM = "tmp"; // 14 digital temperature
const char RUN_LAGOPUS_VL53L1X_NAME  [] PROGMEM = "vl";  // 15 distance

const char* const run_names[] PROGMEM = {
  RUN_NETWORK_NAME,
  RUN_BATTERY_NAME,
  RUN_GPS_NAME,
  RUN_FREE_1_NAME,
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
};

// Flags available: 2 8 16 32 64 128
#define FLAG_LOG_USB 1
#define FLAG_LOG_SD  4

#define UIO_12V 1
#define UIO_5V 2
#define UIO_3V3 4
#define UIO_LEAD_VOLTAGE 8
#define UIO_MAXB 16
#define UIO_I2C 32
#define UIO_1WIRE 64
#define UIO_SDI12 128

#define ADD_SENSOR(type, ...) \
  if (UIO.addSensor(type, ## __VA_ARGS__) == -1)\
  {\
    UIO.createFrame();\
    UIO.addSensor(type, ## __VA_ARGS__);\
  }


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
  {XBEE_BROADCAST, {0x12, 0x02}, 0x0F, "000000000000FFFF"}, // Default (2)
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

/// private methods //////////////////////////
private:

/// public methods and attributes ////////////
public:

// Like Arduino's EEPROM.update, it writes the given value only if different
// from the value already saved.
bool updateEEPROM(int address, uint8_t value);
bool updateEEPROM(int address, uint16_t value);
bool updateEEPROM(int address, uint32_t value);
bool writeEEPROM(int address, char* src, size_t size);
char* readEEPROM(int address, char* dst, size_t size);

// Configuration variables
uint8_t flags;
board_type_t boardType;
battery_type_t batteryType;
network_type_t networkType;
uint16_t actions[RUN_LEN];

// Variables updated on every loop (see onLoop)
float batteryVolts;
uint8_t batteryLevel;
battery_t battery;
uint8_t cooldown; // Reduces frequency of action, depends on battery
// To keep time without calling RCT each time
unsigned long epochTime; // seconds since the epoch
unsigned long start;     // millis taken at epochTime

// Autodetect
bool hasSD;
bool hasGPS = false;

// SD
const char* archive_dir = "/data";
const char* logFilename = "LOG.TXT";
const char* qstartFilename = "QSTART.BIN";
const char* queueFilename = "TMP.TXT"; // TODO Rename to QUEUE.BIN
const char* timeFilename = "TIME.TXT"; //
SdFile logFile;
SdFile queueFile;
SdFile qstartFile;
bool ack_wait;
int createFile(const char*);
int createDir(const char*);
int openFile(const char* filename, SdFile &file, uint8_t mode);
int baselayout();
void getDataFilename(char* filename, uint8_t year, uint8_t month, uint8_t date);
void startSD();
void stopSD();
int readline(SdFile &file);
int write(SdFile &file, const void* buf, size_t nbyte);
int append(SdFile &file, const void* buf, size_t nbyte);
int8_t walk(SdBaseFile &root,
            bool (*before_cb)(SdBaseFile &me, char* name),
            bool (*file_cb)(SdBaseFile &parent, char* name),
            bool (*after_cb)(SdBaseFile &me, char* name));

// Network
void networkInit();

#if WITH_XBEE
// Network: Xbee
void OTA_communication(int OTA_duration); // TODO
XBee xbee;
void xbeeInit();
const char* BROADCAST_ADDRESS = "000000000000FFFF";
#endif

#if WITH_4G
// Network: 4G
uint16_t pin; // Pin for 4G module
void _4GInit();
uint8_t _4GStart();
uint8_t _4GStop();
#endif

#if WITH_CRYPTO
// Crypto
char password[33]; // To encrypt frames
#endif

// Power
// (power) state management
uint8_t state;
uint8_t saved_state;
uint16_t saved_WaspRegister;
void saveState();
void loadState();
bool _setState(uint8_t device, bool new_state);
// Power board
void startPowerBoard();
bool _powerBoardSwitch(uint8_t device, uint8_t pin, bool new_state);
bool v33(bool new_state);
bool v5(bool new_state);
bool v12(bool new_state);
bool leadVoltage(bool new_state);
void setSensorPower(uint8_t type, uint8_t mode);
// Sensor board
void startSensorBoard();
bool maxbotix(bool new_state);
bool i2c(bool new_state);
bool onewire(bool new_state);
bool sdi12(bool new_state);
// Battery
void readBattery();
float getBatteryVolts();
float getLeadBatteryVolts();

// Init, start and stop methods
void boot();
void onSetup();
void onLoop();
bool action(uint8_t n, ...);
const uint8_t loop_timeout = 4; // minutes
const uint32_t send_timeout = 3 * 60; // seconds

// Frames
void createFrame(bool discard=false);
int8_t addSensor(uint8_t type, ...);
uint8_t addSensorValue(float value);
uint8_t addSensorValue(int8_t value);
uint8_t addSensorValue(uint8_t value);
uint8_t addSensorValue(int16_t value);
uint8_t addSensorValue(uint16_t value);
uint8_t addSensorValue(int32_t value);
uint8_t addSensorValue(uint32_t value);
uint8_t getSequence(uint8_t *p);
void showFrame(uint8_t *p);
uint8_t frame2Sd();
int readFrame();
void setFrameSize();

// Menu
void clint();
const char* pprint4G(char* dst, size_t size);
const char* pprintAction(char* dst, size_t size, uint8_t action, const __FlashStringHelper* name);
const char* pprintActions(char* dst, size_t size);
const char* pprintBattery(char* dst, size_t size);
const char* pprintBoard(char* dst, size_t size);
const char* pprintFrames(char* dst, size_t size);
const char* pprintLog(char* dst, size_t size);
const char* pprintSerial(char* str, size_t size);
const char* pprintXBee(char* dst, size_t size);

// Time
uint8_t saveTime();
uint8_t saveTimeToSD();
uint8_t setTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
uint8_t setTime(uint32_t time);
void loadTime();
unsigned long getEpochTime();
unsigned long getEpochTime(uint16_t &ms);
uint8_t setTimeFromNetwork();
int8_t gps(bool setTime, bool getPosition);

// Sleep
int nextAlarm(char* alarmTime);
void deepSleep();

// Sensors
int getMaxbotixSample();
bool readMaxbotixSerial(uint16_t &median, uint16_t &sd, uint8_t samples=5);
uint8_t readDS18B20(int values[], uint8_t max);

// I2C
void i2c_scan();
bool i2c_AS7263(byte &temp, float &r, float &s, float &t, float &u, float &v, float &w);
bool i2c_AS7265(
  uint8_t &temp,
  float &A, float &B, float &C, float &D, float &E, float &F,
  float &G, float &H, float &I, float &J, float &K, float &L,
  float &R, float &S, float &T, float &U, float &V, float &W);
bool i2c_BME280(float &temperature, float &humidity, float &pressure, uint8_t address=I2C_ADDRESS_Lemming_BME280);
bool i2c_MLX90614(float &object, float &ambient);
bool i2c_TMP102(float &temperature);
bool i2c_VL53L1X(uint16_t &distance);

// SDI-12
const char* sdi_identify(uint8_t address);
char sdi_read_address();
uint8_t sdi_set_address(char current_address, char new_address);

// Utils
int8_t index(const char* const list[], size_t size, const char* str);
void sort_uint16(uint16_t* array, uint8_t size);
uint16_t median_uint16(uint16_t* array, uint8_t size);
uint16_t sd_uint16(uint16_t* array, uint8_t size, uint16_t mean);

};


extern WaspUIO UIO;


void vlog(loglevel_t level, const char* message, va_list args);
void beforeSleep();
void afterSleep();
void onHAIwakeUP_after(void);


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

#define COMMAND(name) cmd_status_t name(const char* str)
COMMAND(exeCommand);
COMMAND(cmd4G_APN);
COMMAND(cmd4G_Pin);
COMMAND(cmd4G_Test);
COMMAND(cmdAck);
COMMAND(cmdBattery);
COMMAND(cmdBoard);
COMMAND(cmdCat);
COMMAND(cmdCatx);
COMMAND(cmdDisable);
COMMAND(cmdEnable);
COMMAND(cmdExit);
COMMAND(cmdFormat);
COMMAND(cmdGPS);
COMMAND(cmdHelp);
COMMAND(cmdI2C);
COMMAND(cmdLogLevel);
COMMAND(cmdLs);
COMMAND(cmdMB);
COMMAND(cmdName);
COMMAND(cmdNetwork);
COMMAND(cmd1WireRead);
COMMAND(cmd1WireScan);
COMMAND(cmdPassword);
COMMAND(cmdPrint);
COMMAND(cmdRm);
COMMAND(cmdRun);
COMMAND(cmdSDI12);
COMMAND(cmdTail);
COMMAND(cmdTime);
COMMAND(cmdXBee);


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
// Externally powered devices
CR_TASK(taskExt);
CR_TASK(taskSdiWS100); // Uses SDI-12
// 3V3
CR_TASK(task1Wire);
CR_TASK(taskI2C);
CR_TASK(taskTTL);
// Network
CR_TASK(taskNetwork4G);
CR_TASK(taskNetworkXBee);
CR_TASK(taskNetworkSend);
CR_TASK(taskNetworkReceive);
// GPS
CR_TASK(taskGps);
// For testing purposes
CR_TASK(taskSlow);


#define SENSOR_BAT        52
#define SENSOR_GPS        53
#define SENSOR_BME_TC     74
#define SENSOR_BME_HUM    76
#define SENSOR_BME_PRES   77
#define SENSOR_TST       123
#define SENSOR_CTD10     200
//#define SENSOR_DS2_1     201
//#define SENSOR_DS2_2     202
#define SENSOR_DS18B20   203
#define SENSOR_MB73XX    204
#define SENSOR_GPS_STATS 205
#define SENSOR_VOLTS     206
#define SENSOR_WS100     207
#define SENSOR_DS2       208

#endif
