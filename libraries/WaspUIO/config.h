// Features: because we don't have enough program memory to compile everything
#define WITH_XBEE TRUE
#define WITH_4G FALSE
#define WITH_GPS TRUE
#define WITH_CRYPTO FALSE

#define WITH_I2C TRUE
#define WITH_SDI TRUE
#define WITH_1WIRE FALSE
#define WITH_MB FALSE

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

#define TWI_SPEED_HZ                   80000 // Max I2C clock speed is 100000 for MLX sesnor       

// Reboot every N loops
#define MAX_LOOPS 5000
