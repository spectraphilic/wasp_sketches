#include "WaspUIO.h"
#include "minmea.h"


static struct minmea_sentence_rmc rmc;
static struct minmea_sentence_gga gga;

/* RMC - Recommended Minimum Navigation Information */
static int handle_rmc(const char *line)
{
    struct minmea_sentence_rmc frame;
    int ok = minmea_parse_rmc(&frame, line);
    if (ok && frame.valid) {
        rmc = frame;
        return 1;
    }

    return 0;
}

/* GGA - Global Positioning System Fix Data */
static int handle_gga(const char *line)
{
    struct minmea_sentence_gga frame;
    int ok = minmea_parse_gga(&frame, line);
    //int ok = minmea_parse_gga(&frame, "$GPGGA,100121.219,3959.0924,N,00002.8586,W,1,03,4.6,-52.0,M,52.0,M,,0000*53");
    if (ok && frame.satellites_tracked > 2) {
        gga = frame;
        return 1;
    }

    return 0;
}

static int handle_sentence(const char *line, enum minmea_sentence_id expect)
{
    enum minmea_sentence_id id = minmea_sentence_id(line, true);
    if (id != expect) {
        return 0;
    }

    USB.print(line);

    switch (id) {
        case MINMEA_SENTENCE_GGA:
            return handle_gga(line);
        case MINMEA_SENTENCE_RMC:
            return handle_rmc(line);
        default:
            return 0;
    }
}

static void read() {
}

int8_t WaspUIO::gps(int time, int position)
{
    uint8_t uart = 1; // UART1 is shared by 4 ports: Socket1, GPS socket, Auxiliar1 and Auxiliar2
    char buffer[160];
    int max = sizeof(buffer) - 1;  // Max number of chars that the buffer can hold
    bool time_ok = false;
    bool position_ok = false;

    log_debug("GPS start");

    // Stop SD
    if (_boot_version >= 'J') {
        stopSD();
    }

    // Start GPS
    if (GPS.ON() == 0) {
        startSD();
        log_error("GPS.ON() failure");
        return -1;
    }

    // Read from GPS module in UART
    int next;
    if (position) {
        next = MINMEA_SENTENCE_GGA;
    }
    else if (time) {
        next = MINMEA_SENTENCE_RMC;
    }
    else {
        next = 0;
    }

    int i = 0;
    int error = 0;  // 1: timeout 2: line too long
    unsigned long t0 = millis();
    while (next) {
        // Stop conditions
        if (cr.timeout(t0, 120 * 1000L)) {
            error = 1;  // Timeout
            break;
        }
        if (i >= sizeof(buffer) - 1) {
            error = 2;  // Avoid buffer overflow
            break;
        }

        if (serialAvailable(1)) {
            char c = serialRead(uart);
            buffer[i++] = c;
            buffer[i] = 0;
            if (c == '\n') {
                i = 0;
                if (handle_sentence(buffer, (enum minmea_sentence_id)next)) {
                    if (next == MINMEA_SENTENCE_GGA) {
                        position_ok = true;
                        next = time ? MINMEA_SENTENCE_RMC : 0;
                    }
                    else if (next == MINMEA_SENTENCE_RMC) {
                        time_ok = true;
                        next = 0;
                    }
                    else {
                        next = 0;
                    }
                }
            }
        }
    }

    // Stop GPS
    GPS.OFF();

    // Set time as soon as possible
    // TODO Test
    if (time_ok && time >= 2) {
        RTC.setTime(rmc.date.year, rmc.date.month, rmc.date.day,
                    RTC.dow(rmc.date.year, rmc.date.month, rmc.date.day),
                    rmc.time.hours, rmc.time.minutes, rmc.time.seconds);
    }

    // Start SD card
    startSD();

    // Time
    if (time_ok) {
        if (time >= 2) {
            log_info("GPS Time updated!");
        }
        else {
            USB.print("date=");
            USB.print(rmc.date.year);
            USB.print(rmc.date.month);
            USB.println(rmc.date.day);
            USB.print("time=");
            USB.print(rmc.time.hours);
            USB.print(rmc.time.minutes);
            USB.println(rmc.time.seconds);
        }
    }

    // Position
    if (position_ok) {
        float lat = minmea_tocoord(&gga.latitude);
        float lon = minmea_tocoord(&gga.longitude);
        float alt = minmea_tofloat(&gga.altitude);
        float acc = minmea_tofloat(&gga.hdop);
        int sat = gga.satellites_tracked;

        if (position >= 2) {
            // Debug
            Utils.float2String(lat, buffer, 6);
            log_debug("GPS latitude  %s", buffer);
            Utils.float2String(lon, buffer, 6);
            log_debug("GPS longitude %s", buffer);
            Utils.float2String(alt, buffer, 6);
            log_debug("GPS altitude  %s", buffer);
            Utils.float2String(acc, buffer, 6);
            log_debug("GPS satellites=%d accuracy=%s", sat, buffer);

            // Frames
            ADD_SENSOR(SENSOR_GPS, lat, lon);
            ADD_SENSOR(SENSOR_ALTITUDE, alt)
            ADD_SENSOR(SENSOR_GPS_ACCURACY, gga.satellites_tracked, acc);
        }
        else if (position == 1) {
            USB.print("lat="); USB.println(lat);
            USB.print("lng="); USB.println(lon);
            USB.print("alt="); USB.println(alt);
            USB.print("acc="); USB.println(acc);
            USB.print("sat="); USB.println(sat); // FIXME 03 should be 3 not 311
        }
    }

    if (error == 1) {
        log_warn("GPS Timeout");
        return -1;
    }
    else if (error == 2) {
        log_warn("Line too long");
        return -1;
    }

    return 0;
}


int8_t WaspUIO::gps_old(bool setTime, bool getPosition)
{
    PGM_P error = NULL;
    uint8_t satellites;

    log_debug("GPS start");
    if (_boot_version >= 'J')
        stopSD();

    // On
    if (GPS.ON() == 0) {
        startSD();
        log_error("GPS.ON() failure");
        return -1;
    }

    // Connect
    if (GPS.waitForSignal(150) == false) { // 150s = 2m30s
        error = PSTR("GPS Timeout");
        goto exit;
    }

    // Position
    if (getPosition) {
        // Try twice to get enough satellites (4), wait 10s before each try
        for (int i=0; i < 3; i++) {
            delay(10000); // 10s
            int8_t status = GPS.getPosition();
            if (status == 1) {
                satellites = (uint8_t) atoi(GPS.satellites);
                if (satellites > 4)
                    break;
            } else if (status == -1) {
                error = PSTR("GPS.getPosition() No GPS signal");
                goto exit;
            } else { // if (status == 0)
                error = PSTR("GPS.getPosition() Timeout");
                goto exit;
            }
        }
    }

    // Time
    if (setTime)
        GPS.setTimeFromGPS(); // Save time to RTC

    GPS.OFF();
    startSD();

    // Set system time. Do this here because we need the SD
    if (setTime) {
        UIO.loadTime();
        log_info("GPS Time updated!");
    }

    if (getPosition) {
        float lat = GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator);
        float lon = GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator);
        float alt = atof(GPS.altitude);
        float acc = atof(GPS.accuracy);

        // Debug
        char lat_str[15];
        char lon_str[15];
        Utils.float2String(lat, lat_str, 6);
        Utils.float2String(lon, lon_str, 6);
        log_debug("GPS latitude  %s %c => %s", GPS.latitude, GPS.NS_indicator, lat_str);
        log_debug("GPS longitude %s %c => %s", GPS.longitude, GPS.EW_indicator, lon_str);
        log_debug("GPS altitude=%s", GPS.altitude);
        log_debug("GPS satellites=%s accuracy=%s", GPS.satellites, GPS.accuracy);

        // Frames
        ADD_SENSOR(SENSOR_GPS, lat, lon);
        ADD_SENSOR(SENSOR_ALTITUDE, alt)
        ADD_SENSOR(SENSOR_GPS_ACCURACY, satellites, acc);
    }

exit:
    if (error) {
        GPS.OFF();
        startSD();

        cr.log_P(LOG_ERROR, error);
        return -1;
    }

    return 0;
}
