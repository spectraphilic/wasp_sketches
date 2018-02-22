/*! \def myLibrary_h
 \brief The library flag
 */
#ifndef WaspUIO_h
#define WaspUIO_h

/******************************************************************************
 * Includes
 ******************************************************************************/

// We cannot enable everything at the same time or we run out of program memory
#define USE_AGR false
#define USE_I2C true // I include here OneWire as well
#define USE_SDI true

// Pins
#define PIN_1WIRE DIGITAL6 // Use DIGITAL6 as default (protoboard)
#define PIN_SDI12 DIGITAL8 // Use DIGITAL8 as default (protoboard)

#include <inttypes.h>

#include <WaspFrame.h>
#include <WaspGPS.h>
#include <WaspXBeeDM.h>
#include <BME280.h>

#include <Coroutines.h>
#include <SDI12.h>

// RTC uses I2C. If there are other sensors with I2C they must be powered on
// when using RTC, otherwise it won't work. See TwoWire::secureBegin
#if USE_AGR
#include <WaspSensorAgr_v20.h>
#elif USE_I2C
#include <WaspSensorAmbient.h> // This is used to enable 3V3
#endif


/******************************************************************************
 * Definitions & Declarations
 ******************************************************************************/

// EEPROM addresses used by the library
#define EEPROM_UIO_FLAGS (EEPROM_START + 0)
#define EEPROM_UIO_NETWORK (EEPROM_START + 1)   // 2 bytes to store the network id
// 1 byte available
#define EEPROM_UIO_BATTERY_TYPE (EEPROM_START + 4)
// 2 bytes available
#define EEPROM_UIO_LOG_LEVEL (EEPROM_START + 7) // 1 byte for the log level
// 1 bytes available
#define EEPROM_RUN (EEPROM_START + 9)
enum run_t {
  RUN_NETWORK,
  RUN_SENSIRION, // Agr
  RUN_PRESSURE,
  RUN_LEAFWETNESS,
  RUN_CTD10, // SDI-12
  RUN_DS2,
  RUN_DS1820, // OneWire
  RUN_BME280, // I2C
  RUN_MB, // TTL
  RUN_LEN // Special value
};

// Flags available: 2 8 16 32 64 128
#define FLAG_LOG_USB 1
#define FLAG_LOG_SD  4

#define UIO_SD 1
#define UIO_PRESSURE 2
#define UIO_SENSIRION 4
#define UIO_LEAFWETNESS 8
#define UIO_I2C 16
#define UIO_1WIRE 32
#define UIO_SDI12 64

#define ADD_SENSOR(type, ...) \
  if (frame.addSensorBin(type, ## __VA_ARGS__) == -1)\
  {\
    UIO.createFrame();\
    frame.addSensorBin(type, ## __VA_ARGS__);\
  }


#define encryptionKey "1234567890123456"  // General encryption key for UIO networks
#define key_access (char*) "LIBELIUM"

struct Network {
  const char *name;
  uint8_t panid[2];
  uint8_t channel;
  char rx_address[17];
};

// NOTE The low bit of the network id (panID) must start at zero and increment
// one by one. This is because the low bit is used as an index to this same
// array.

const char NETWORK_FINSE     [] PROGMEM = "Finse";
const char NETWORK_UNUSED    [] PROGMEM = "unused";
const char NETWORK_BROADCAST [] PROGMEM = "Broadcast";
const char NETWORK_PI_UIO    [] PROGMEM = "Pi UiO";
const char NETWORK_PI_FINSE  [] PROGMEM = "Pi Finse";
const char NETWORK_PI_CS     [] PROGMEM = "Pi CS";

const Network networks[] PROGMEM = {
  {NETWORK_FINSE,     {0x12, 0x00}, 0x0F, "0013A20040779085"},
  {NETWORK_UNUSED,    {0x12, 0x01}, 0x0F, "0000000000000000"}, // Available
  {NETWORK_BROADCAST, {0x12, 0x02}, 0x0F, "000000000000FFFF"}, // Default (2)
  {NETWORK_PI_UIO,    {0x12, 0x03}, 0x0F, "0013A200416B1BA0"},
  {NETWORK_PI_FINSE,  {0x12, 0x04}, 0x0F, "0013A200416A0724"},
  {NETWORK_PI_CS,     {0x12, 0x05}, 0x0F, "0013A200412539D3"}, // Office of jdavid
};
const uint8_t network_len = sizeof(networks) / sizeof(Network);

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
bool updateEEPROM(int address, uint32_t value);

// Configuration variables
uint8_t flags;
uint8_t batteryType; // 1 => lithium battery  |||||| 2 => Lead acid battery
uint8_t actions[RUN_LEN];

// Variables updated on every loop (see onLoop)
uint8_t batteryLevel;
uint8_t cooldown; // Reduces frequency of action, depends on batteryLevel
// To keep time without calling RCT each time
float rtc_temp;          // internal temperature
unsigned long epochTime; // seconds since the epoch
unsigned long start;     // millis taken at epochTime
int minute;              // minute of the day, from 0 to 1439
int day;                 // days since the epoch
int next_minute;         // minute of the next alarm

// Autodetect
bool hasSD;
bool hasGPS;

// SD
const char* archive_dir = "/data";
const char* logFilename = "LOG.TXT";
const char* tmpFilename = "TMP.TXT";
int createFile(const char*);
int createDir(const char*);
int openFile(const char* filename, SdFile &file, uint8_t mode);
int baselayout();
SdFile logFile;
SdFile tmpFile;
void getDataFilename(char* filename, uint8_t year, uint8_t month, uint8_t date);
void startSD();
void stopSD();
int readline(SdFile &file);
int append(SdFile &file, const void* buf, size_t nbyte);

// Network
char myMac[17];
const char* BROADCAST_ADDRESS = "000000000000FFFF";
Network network;
uint8_t readRSSI2Frame(void);
void OTA_communication(int OTA_duration);
const char* readOwnMAC();

// Init, start and stop methods
void onSetup();
void onLoop();
void readBattery();
void initNet();
bool action(uint8_t n, ...);
const uint8_t loop_timeout = 4; // minutes
const uint32_t send_timeout = 3 * 60; // seconds

// Frames
void createFrame(bool discard=false);
uint8_t frame2Sd();
void showBinaryFrame();

// Menu
void clint();
const char* pprintSerial(char* str, size_t size);
const char* pprintBattery(char* dst, size_t size);
const char* pprintLog(char* dst, size_t size);
const char* pprintNetwork(char* dst, size_t size);
const char* pprintActions(char* dst, size_t size);

// Time
unsigned long getEpochTime();
unsigned long getEpochTime(uint16_t &ms);
uint8_t saveTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
void loadTime(bool temp=false);

// Sleep
void nextAlarm();
const char* nextAlarm(char* alarmTime);
void deepSleep();

// Register of "devices" that are On
uint8_t onRegister;
void on(uint8_t device);
void off(uint8_t device);
bool isOn(uint8_t device);

// Sensors
int getMaxbotixSample();
bool readMaxbotixSerial(uint16_t &median, uint16_t &sd, uint8_t samples=5);

// Utils
void sort_uint16(uint16_t* array, uint8_t size);
uint16_t median_uint16(uint16_t* array, uint8_t size);
uint16_t sd_uint16(uint16_t* array, uint8_t size, uint16_t mean);

};


extern WaspUIO UIO;
extern SDI12 mySDI12;


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
COMMAND(cmdBattery);
COMMAND(cmdCat);
COMMAND(cmdDisable);
COMMAND(cmdEnable);
COMMAND(cmdExit);
COMMAND(cmdFormat);
COMMAND(cmdHelp);
COMMAND(cmdLs);
COMMAND(cmdNetwork);
COMMAND(cmdOneWire);
COMMAND(cmdPrint);
COMMAND(cmdRead);
COMMAND(cmdRun);
COMMAND(cmdLogLevel);
COMMAND(cmdSDI12);
COMMAND(cmdTime);
COMMAND(cmdTimeGPS);


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
// 3V3
CR_TASK(task1Wire);
CR_TASK(taskI2C);
CR_TASK(taskTTL);
// Network
CR_TASK(taskNetwork);
CR_TASK(taskNetworkSend);
CR_TASK(taskNetworkReceive);
// GPS
CR_TASK(taskGps);
// For testing purposes
CR_TASK(taskSlow);


#endif
