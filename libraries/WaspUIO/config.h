// Uncomment for the compiler to detect more format related errors. The program
// will not fit into memory, so this is only used for QA purposes.
//#define WFORMAT

//=========== BOARD AND BATTERY =========
// Presence-absence Lemming board: 
#define BOARD_TYPE BOARD_LEMMING		// BOARD_LEMMING, BOARD_NONE

// Battery type: 
#define BATTERY_TYPE BATTERY_LITHIUM	// BATTERY_LITHIUM, BATTERY_REG3V3

// Features: because we don't have enough program memory to compile everything
#define WITH_GPS TRUE 			// External Libelium GPS
#define WITH_EXT_CHARGE FALSE	// Power management for Lora/4G gateway 12V external power supply

//=========== NETWORK CONFIG ==============================
// WLAN Networking - wireless
#define WITH_4G TRUE
#define WITH_IRIDIUM FALSE
// Iridium specific.
// Save frames to lifo instead of fifo every n hours since epoch at the :mm
// minutes (like actions).
// For example if sensors=5m and gps=1h then with SAVE_TO_LIFO 6 / 0 the GPS
// frame will always be sent, because the frames will be saved to LIFO every 6h
// at :00
#define SAVE_TO_LIFO_HOUR 4
#define SAVE_TO_LIFO_MINUTE 0

// LAN Networking - local radio network, xbee or lora
#define WITH_XBEE FALSE
#define WITH_LORA TRUE
// Lora network configuration
#define LORA_CHANNEL CH_10_868 	// From 10 (865.20 MHz) to 17 (868 MHz)
#define LORA_POWER 'H' 			// Low = 0 dBm / High = 7 dBm / Max = 14 dBm
#define LORA_MAX_FAILS 3 		// In auto mode (lora.dst=0), after 3 fails forget dst and ping again


//=========== SENSOR CONFIG ==============================
// Sensors
#define WITH_I2C FALSE			// I2C protocol for all external I2C sensors (lagopus and lemming boards)
#define WITH_SDI FALSE			// SDI12 protocol for DS2, CTD, ATMOS22
#define WITH_1WIRE FALSE		// 1wire protocol for DS18b20
#define WITH_MB FALSE			// Maxbotix sensor
#define WITH_AS7265 FALSE		// Radiation sensor, I2C 

// Maxbotix and VL53L1X number of samples. These valuse set the length of the array retruned by the sensor of interest
#define MB_SAMPLES 5     		// maximum number of samples for Maxbotix is 25
#define VL_SAMPLES 30    		// maximum number of samples for VL53L1X  is 99


//=======================================================
//================== WARNING: DEV. ONLY ZONE =================
// Cryptogrphy of frames
#define WITH_CRYPTO FALSE		// Enable cryptography (not implemented)

// Timeouts
#define LOOP_TIMEOUT 4 			// [minutes]  Loop max timeout
#define SEND_TIMEOUT 300000L 	// [ms]	LAN max waiting time

// Reboot every N loops
#define MAX_LOOPS 1000

// Pins
#define PIN_POWER_MAXB  DIGITAL1 // Maxbotix power switch
#define PIN_POWER_I2C   DIGITAL2 // I2C power switch
#define PIN_POWER_I2C_2 DIGITAL3 // I2C (2nd line) power switch
#define PIN_ISBD_SLEEP  DIGITAL4 // Iridium
#define PIN_POWER_1WIRE DIGITAL5 // OneWire power switch
#define PIN_1WIRE       DIGITAL6 // Use DIGITAL6 as default (protoboard)
#define PIN_POWER_SDI12 DIGITAL7 // SDI-12 power switch
#define PIN_SDI12       DIGITAL8 // Use DIGITAL8 as default (protoboard)

#define PIN_POWER_EXT   DIGITAL1 // External power charge (same as MB power switch)

// I2C addresses
#define I2C_ADDRESS_LAGOPUS_VL53L1X    0x29 // Distance (Snow depth)
#define I2C_ADDRESS_SHT31              0x44 // Temperature & humidity
#define I2C_ADDRESS_LAGOPUS_TMP102     0x48 // Air temperature
#define I2C_ADDRESS_TMP117             0x48 // Air temperature, replaces TMP102
#define I2C_ADDRESS_LAGOPUS_AS726X     0x49 // Spectral Sensor
#define I2C_ADDRESS_LAGOPUS_MLX90614   0x5A // Surface temperature
#define I2C_ADDRESS_Lemming_BME280     0x76 // Internal temperature, Relative humidity and atmospheric pressure
#define I2C_ADDRESS_LAGOPUS_BME280     0x77 // Air temperature, Relative humidity and atmospheric pressure

