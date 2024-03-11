#include "WaspUIO.h"
#include "minmea.h"


static struct minmea_sentence_rmc rmc;
static struct minmea_sentence_gga gga;
static bool time_ok;
static bool position_ok;

/* RMC - Recommended Minimum Navigation Information */
static int handle_rmc(const char *line)
{
    struct minmea_sentence_rmc frame;
    int ok = minmea_parse_rmc(&frame, line);
    if (ok && frame.valid) {
        rmc = frame;
        time_ok = true;
    }

    return ok;
}

/* GGA - Global Positioning System Fix Data */
static int handle_gga(const char *line)
{
    struct minmea_sentence_gga frame;
    int ok = minmea_parse_gga(&frame, line);
    //int ok = minmea_parse_gga(&frame, "$GPGGA,002153.000,3342.6618,N,11751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E");
    if (ok && frame.satellites_tracked > 1) {
        gga = frame;
        position_ok = true;
    }

    return ok;
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

int8_t WaspUIO::gps(bool setTime, bool getPosition)
{
    char buffer[160];
    uint8_t uart = 1; // UART1 is shared by 4 ports: Socket1, GPS socket, Auxiliar1 and Auxiliar2
    time_ok = false;
    position_ok = false;

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
    int i = 0;
    unsigned long timeout = 150 * 1000L; // 150s
    unsigned long t0 = millis();
    while (!cr.timeout(t0, timeout)) {
        if (serialAvailable(1)) {
            char c = serialRead(uart);
            buffer[i++] = c;
            buffer[i] = 0;
            if (c == '\n') {
                i = 0;
                if (getPosition && !position_ok) {
                    handle_sentence(buffer, MINMEA_SENTENCE_GGA);
                }
                else if (setTime && !time_ok) {
                    handle_sentence(buffer, MINMEA_SENTENCE_RMC);
                }
                else {
                    break;
                }
            }
            if (i >= sizeof(buffer) - 1) {
                break;
            }
        }
    }

    // Stop GPS
    GPS.OFF();

    // TODO Set time
    if (time_ok) {
        USB.print("date=");
        USB.print(rmc.date.year);
        USB.print(rmc.date.month);
        USB.println(rmc.date.day);
        USB.print("time=");
        USB.print(rmc.time.hours);
        USB.print(rmc.time.minutes);
        USB.println(rmc.time.seconds);

//      RTC.setTime(rmc.date.year, rmc.date.month, rmc.date.day,
//                  RTC.dow(rmc.date.year, rmc.date.month, rmc.date.day),
//                  rmc.time.hours, rmc.time.minutes, rmc.time.seconds);
    }

    // Start SD card
    startSD();
    if (time_ok) {
        log_info("GPS Time updated!");
    }

    // TODO Add frame
    if (position_ok) {
        float lat = minmea_tocoord(&gga.latitude);
        float lon = minmea_tocoord(&gga.longitude);
        float alt = minmea_tofloat(&gga.altitude);
        float acc = minmea_tofloat(&gga.hdop);
        int sat = gga.satellites_tracked;

        USB.print("lat="); USB.println(lat);
        USB.print("lng="); USB.println(lon);
        USB.print("alt="); USB.println(alt);
        USB.print("acc="); USB.println(acc);
        USB.print("sat="); USB.println(sat);

        // Debug
//      char str[15];
//      Utils.float2String(lat, str, 6);
//      log_debug("GPS latitude  %s", lat_str);
//      Utils.float2String(lon, str, 6);
//      log_debug("GPS longitude %s", str);
//      Utils.float2String(alt, str, 6);
//      log_debug("GPS altitude  %s", str);
//      Utils.float2String(acc, str, 6);
//      log_debug("GPS satellites=%d accuracy=%s", sat, str);

        // Frames
//      ADD_SENSOR(SENSOR_GPS, lat, lon);
//      ADD_SENSOR(SENSOR_ALTITUDE, alt)
//      ADD_SENSOR(SENSOR_GPS_ACCURACY, gga.satellites_tracked, acc);
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
    // TODO optimize, part of the work in setTimeFromGPS is already done in
    // getPosition above
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
