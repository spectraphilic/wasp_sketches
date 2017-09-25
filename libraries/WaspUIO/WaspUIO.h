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

/******************************************************************************
 * Definitions & Declarations
 ******************************************************************************/

// We cannot enable everything at the same time or we run out of program memory
#define USE_AGR false
#define USE_I2C true // I include here OneWire as well
#define USE_SDI true
#define FRAME_BINARY true

// EEPROM addresses used by the library
#define EEPROM_UIO_FLAGS (EEPROM_START + 0)
#define EEPROM_UIO_NETWORK (EEPROM_START + 1)   // 2 bytes to store the network id
#define EEPROM_UIO_WAKEUP_PERIOD (EEPROM_START + 3)
// 3 bytes available
#define EEPROM_UIO_LOG_LEVEL (EEPROM_START + 7) // 1 byte for the log level
// 2 bytes available
#define EEPROM_SENSOR_SENSIRION (EEPROM_START + 10) // Agr
#define EEPROM_SENSOR_PRESSURE (EEPROM_START + 11)
#define EEPROM_SENSOR_LEAFWETNESS (EEPROM_START + 12)
#define EEPROM_SENSOR_CTD10 (EEPROM_START + 13) // SDI-12
#define EEPROM_SENSOR_DS2 (EEPROM_START + 14)
#define EEPROM_SENSOR_DS1820 (EEPROM_START + 15) // OneWire
#define EEPROM_SENSOR_BME280 (EEPROM_START + 16) // I2C

#define FLAG_LOG_USB 1
#define FLAG_NETWORK 2
#define FLAG_LOG_SD  4

#define ADD_SENSOR(type, ...) \
  if (frame.addSensor(type, ## __VA_ARGS__) == -1)\
  {\
    UIO.createFrame();\
    frame.addSensor(type, ## __VA_ARGS__);\
  }


#define encryptionKey "1234567890123456"  // General encryption key for UIO networks
#define key_access (char*) "LIBELIUM"

enum network_t {
  NETWORK_FINSE,
  NETWORK_GATEWAY,
  NETWORK_BROADCAST,
  NETWORK_FINSE_ALT,
  NETWORK_OTHER
};

struct Network {
  char name[12];
  uint8_t panid[2];
  uint8_t channel;
  char rx_address[17];
};

// NOTE The low bit of the network id (panID) must start at zero and increment
// one by one. This is because the low bit is used as an index to this same
// array.

const Network networks[] PROGMEM = {
  {"Finse"    , {0x12, 0x00}, 0x0F, "0013A20040779085"},
  {"Gateway"  , {0x12, 0x01}, 0x0F, "0013A20040DB6048"},
  {"Broadcast", {0x12, 0x02}, 0x0F, "000000000000FFFF"},
  {"Finse_alt", {0x12, 0x03}, 0x0F, "13A200416A072300"},
  {"Other"    , {0x12, 0x04}, 0x0F, "13A200416A072300"}, // Default
};

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

// Helper functions to work with files
int append(SdFile &file, const void* buf, size_t nbyte);

// Filenames
const char* archive_dir = "/data";
const char* logFilename = "LOG.TXT";

// Interactive mode
void menuTime();
void menuTimeManual();
void menuNetwork();
void menuLog();
void menuLog2(uint8_t flag, const char* var);
void menuLogLevel();
void menuSensors();
void menuWakeup();
void menuSensor(uint16_t sensor, uint8_t &value);
void menuSD();
const char* flagStatus(uint8_t flag);
const char* sensorStatus(uint8_t sensor);
void setNetwork(network_t);

/// public methods and attributes ////////////
public:

uint8_t flags;
bool hasSD;
SdFile logFile;

// Variables updated on every loop (see initTime)
uint8_t batteryLevel;
timestamp_t time;        // broken timestamp
uint8_t period;

// Sensors
uint8_t wakeup_period;
uint8_t sensor_sensirion;
uint8_t sensor_pressure;
uint8_t sensor_leafwetness;
uint8_t sensor_ctd10;
uint8_t sensor_ds2;
uint8_t sensor_ds1820;
uint8_t sensor_bme280;
bool action(uint8_t n, ...);

// Network related
Network network;
char buffer[100];
uint8_t receiveGPSsyncTime();
uint8_t readRSSI2Frame(void);

// Frame files
const char* tmpFilename = "TMP.TXT"; // Change name as format did change
SdFile tmpFile;
void getDataFilename(char* filename, uint8_t year, uint8_t month, uint8_t date);

// To keep time without calling RCT each time
unsigned long epochTime; // seconds since the epoch
unsigned long start;     // millis taken at epochTime

// Like Arduino's EEPROM.update, it writes the given value only if different
// from the value already saved.
bool updateEEPROM(int address, uint8_t value);
bool updateEEPROM(int address, uint32_t value);

// Variables: sensors
float rtc_temp;

const char* BROADCAST_ADDRESS = "000000000000FFFF";
uint8_t featureUSB = 1;
uint8_t featureNetwork = 1;

// SD related methods
void startSD();
void stopSD();
uint8_t createFile(const char* filename);

// Init, start and stop methods
void onSetup();
void onLoop();
void initTime();
void readBattery();
void initNet();

// Frames
void createFrame(bool discard=false);
uint8_t frame2Sd();
void showFrame();

// Network related
void OTA_communication(int OTA_duration);
const char* readOwnMAC(char* mac);

// Interactive mode
const char* input(char* buffer, size_t size, const __FlashStringHelper *, unsigned long timeout);
void menu();
const char* menuFormatLog(char* dst, size_t size);
const char* menuFormatNetwork(char* dst, size_t size);
const char* menuFormatSensors(char* dst, size_t size);

// Time related methods
unsigned long getEpochTime();
unsigned long getEpochTime(uint16_t &ms);
uint8_t setTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

// Sleep related
const char* getNextAlarm(char* alarmTime);

// Other
uint16_t readMaxbotixSerial(uint8_t samples = 5);

};

extern WaspUIO UIO;


void vlog(loglevel_t level, const char* message, va_list args);
void beforeSleep();
void afterSleep();


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

// Sensors: internal
CR_TASK(taskAcc);
CR_TASK(taskHealthFrame);
// Sensors: external
CR_TASK(taskSensors);
// Agr board
CR_TASK(taskAgr);
// Agr: Pressure
CR_TASK(taskAgrPressure);
// Agr: Low consumption group
CR_TASK(taskAgrLC);
CR_TASK(taskAgrLeafwetnessFrame);
CR_TASK(taskAgrSensirionFrame);
// SDI-12
CR_TASK(taskSdi);
CR_TASK(taskSdiCtd10);
CR_TASK(taskSdiDs2);
// OneWire
CR_TASK(task1Wire);
// I2C
CR_TASK(taskI2C);
// Network
CR_TASK(taskNetwork);
CR_TASK(taskNetworkSend);
CR_TASK(taskNetworkReceive);
// GPS
CR_TASK(taskGps);
// For testing purposes
CR_TASK(taskSlow);


#endif
