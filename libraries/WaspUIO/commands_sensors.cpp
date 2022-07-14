#include "WaspUIO.h"


/**
 * Scan I2C slaves
 * TODO Add support to scan the 2nd I2C line
 */

COMMAND(cmdI2C)
{
    // Parse command line
    char name[20];
    int n = sscanf(str, "%19s", name);
    if (n != -1 && n != 1) {
        return cmd_bad_input;
    }

    // Power ON
    UIO.pwr_i2c(1);
    RTC.ON();

    // Scan
    if (n == -1) {
        UIO.i2c_scan();
        return cmd_quiet;
    }

    // Read
    bool err;
    int8_t value = UIO.index(run_names, sizeof run_names / sizeof run_names[0], name);
    if (value == RUN_ACC) {
        int x, y, z;
        err = UIO.i2c_acc(x, y, z);
    }
    else if (value == RUN_BME280) {
        float temperature, humidity, pressure;
        err = UIO.i2c_BME280(temperature, humidity, pressure);
    }
#if WITH_AS7265
    else if (value == RUN_LAGOPUS_AS7263) {
        uint8_t temp;
        float r, s, t, u, v, w;
        err = UIO.i2c_AS7263(temp, r, s, t, u, v, w);
    }
    else if (value == RUN_LAGOPUS_AS7265) {
        uint8_t temp;
        float A, B, C, D, E, F, G, H, I, J, K, L, R, S, T, U, V, W;
        err = UIO.i2c_AS7265(temp, A, B, C, D, E, F, G, H, I, J, K, L, R, S, T, U, V, W);
    }
#endif
    else if (value == RUN_LAGOPUS_BME280) {
        float temperature, humidity, pressure;
        err = UIO.i2c_BME280(temperature, humidity, pressure, I2C_ADDRESS_LAGOPUS_BME280);
    }
    else if (value == RUN_LAGOPUS_MLX90614) {
        float object, ambient;
        err = UIO.i2c_MLX90614(object, ambient);
    }
    else if (value == RUN_LAGOPUS_TMP102) {
        float temperature;
        err = UIO.i2c_TMP102(temperature);
    }
    else if (value == RUN_TMP117) {
        double temperature;
        err = UIO.i2c_TMP117(temperature);
    }
    else if (value == RUN_LAGOPUS_VL53L1X) { 
        uint8_t nbsamples = 3;
        int distance[nbsamples];
        uint8_t total = UIO.i2c_VL53L1X(distance, nbsamples);
        log_debug("Total = %u", total);
        err = (total == nbsamples) ? 0: 1;
    }
    else if (value == RUN_SHT31) {
        float temperature, humidity;
        err = UIO.i2c_SHT31(temperature, humidity);
    }
    else {
        RTC.OFF();
        return cmd_bad_input;
    }

    RTC.OFF();
    return (err)? cmd_error: cmd_quiet;
}


/**
 * Read sensor now
 */

COMMAND(cmdMB)
{
    uint8_t nbsamples = 3;
    int distance[nbsamples];
    uint8_t total = UIO.readMaxbotixSerial(distance, nbsamples);
    if (total == nbsamples) {
        return cmd_ok;
    }
    else {
        return cmd_error;
    }
}


/**
 * OneWire DS18B20 string: read
 */

COMMAND(cmd1WireRead)
{
    uint8_t max = 99;
    int values[max];
    UIO.readDS18B20(values, max);

    return cmd_ok;
}


/**
 * OneWire DS18B20 string: scan the given pins
 * Save to onewire.txt
 */

uint8_t _getPin(uint8_t pin)
{
    // 0 - 8
    switch (pin) {
        case 0: return DIGITAL0;
        case 1: return DIGITAL1;
        case 2: return DIGITAL2;
        case 3: return DIGITAL3;
        case 4: return DIGITAL4;
        case 5: return DIGITAL5;
        case 6: return DIGITAL6;
        case 7: return DIGITAL7;
        case 8: return DIGITAL8;
    }

    return 255;
}

COMMAND(cmd1WireScan)
{
    uint8_t npins;
    uint8_t pins[] = {255, 255, 255};
    uint8_t pin;
    uint8_t addr[8];
    char addr_str[17];
    uint8_t crc;
    SdFile file;
    size_t size = 20;
    char buffer[size];

    // Check input
    npins = sscanf(str, "%hhu %hhu %hhu", &pins[0], &pins[1], &pins[2]);
    if (npins < 1) {
        return cmd_bad_input;
    }

    if (! UIO.hasSD) {
        return cmd_unavailable;
    }

    // ON
    if (! SD.openFile("onewire.txt", &file, O_WRITE | O_CREAT | O_TRUNC)) {
        cr_printf("Error opening onewire.txt");
        return cmd_error;
    }

    UIO.pwr_1wire(1);

    for (uint8_t i = 0; i < npins; i++) {
        pin = _getPin(pins[i]);
        if (pin == 255) continue;
        pinMode(pin, INPUT);

        cr_snprintf(buffer, size, "%hhu", pins[i]);
        USB.print(buffer); file.write(buffer);
        WaspOneWire oneWire(pin);

        // For now we only support the DS1820, so here just read that directly
        // We assume we have a chain of DS1820 sensors, and read all of them.
        if (! oneWire.reset()) {
            cr_printf(" nothing");
            goto next;
        }

        // Search
        oneWire.reset_search();
        while (oneWire.search(addr)) {
            Utils.hex2str(addr, addr_str, 8);
            cr_snprintf(buffer, size, " %s", addr_str);
            USB.print(buffer); file.write(buffer);

            // Check address CRC
            crc = oneWire.crc8(addr, 7);
            if (crc != addr[7]) {
                cr_printf("(crc error)"); continue;
            }

            // Only DS18B20 is supported for now
            if (addr[0] != 0x28) {
                cr_printf("(not DS18B20)"); continue;
            }
        }

next:
        oneWire.depower();
        USB.println(); file.write("\n");
    }

    // OFF
    file.close();

    return cmd_quiet;
}


/**
 * SDI-12
 */

COMMAND(cmdSDI12)
{
    UIO.pwr_sdi12(1);
    UIO.sdi_command(str);
    UIO.pwr_sdi12(0);
    return cmd_quiet;
}
