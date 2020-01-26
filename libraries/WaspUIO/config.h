// Features: because we don't have enough program memory to compile everything
#define WITH_GPS TRUE
#define WITH_CRYPTO FALSE

// Networking
#define WITH_XBEE FALSE
#define WITH_4G TRUE
#define WITH_IRIDIUM FALSE
#define WITH_LORA TRUE

// Sensors
#define WITH_I2C FALSE
#define WITH_SDI FALSE
#define WITH_1WIRE FALSE
#define WITH_MB FALSE

// Timeouts
#define LOOP_TIMEOUT 4 // minutes
#define SEND_TIMEOUT 300000L // ms

// Reboot every N loops
#define MAX_LOOPS 1000

// Pins
#define PIN_1WIRE DIGITAL6 // Use DIGITAL6 as default (protoboard)
#define PIN_SDI12 DIGITAL8 // Use DIGITAL8 as default (protoboard)

// I2C addresses
#define I2C_ADDRESS_Lemming_BME280     0x76 // Internal temperature, Relative humidity and atmospheric pressure
#define I2C_ADDRESS_LAGOPUS_BME280     0x77 // Air temperature, Relative humidity and atmospheric pressure
#define I2C_ADDRESS_LAGOPUS_TMP102     0x48 // Air temperature
#define I2C_ADDRESS_LAGOPUS_VL53L1X    0x29 // Distance (Snow depth)
#define I2C_ADDRESS_LAGOPUS_MLX90614   0x5A // Surface temperature
#define I2C_ADDRESS_LAGOPUS_AS726X     0x49 // Spectral Sensor

// Iridium specific.
// Save frames to lifo instead of fifo every n hours since epoch at the :mm
// minutes (like actions).
// For example if sensors=5m and gps=1h then with SAVE_TO_LIFO 6 / 0 the GPS
// frame will always be sent, because the frames will be saved to LIFO every 6h
// at :00
#define SAVE_TO_LIFO_HOUR 6
#define SAVE_TO_LIFO_MINUTE 0

// Maxbotix and VL53L1X number of samples. These valuse set the length of the array retruned by the sensor of interest
#define MB_SAMPLES 5     // maximum number of samples for Maxbotix is 25
#define VL_SAMPLES 30    // maximum number of samples for VL53L1X  is 99

// Lora configuration
#define LORA_CHANNEL CH_10_868 // From 10 (865.20 MHz) to 17 (868 MHz)
#define LORA_POWER 'H' // Low = 0 dBm / High = 7 dBm / Max = 14 dBm
