#include "WaspUIO.h"
#include "minmea.h"


static struct minmea_sentence_rmc rmc;
static struct minmea_sentence_gga gga;

/* RMC - Recommended Minimum Navigation Information */
static int handle_rmc(const char *line)
{
    struct minmea_sentence_rmc frame;
    int ok = minmea_parse_rmc(&frame, line);
    //int ok = minmea_parse_rmc(&frame, "$GPRMC,114933.401,A,3959.0757,N,00002.8708,W,0.50,154.92,210324,,,A*7A");
    if (ok) {
        //USB.print(line);
        if (frame.valid && frame.date.year >= 24) {
            rmc = frame;
            return 1;
        }
    }

    return 0;
}

/* GGA - Global Positioning System Fix Data */
static int handle_gga(const char *line)
{
    struct minmea_sentence_gga frame;
    int ok = minmea_parse_gga(&frame, line);
    //int ok = minmea_parse_gga(&frame, "$GPGGA,114933.401,3959.0757,N,00002.8708,W,1,03,4.4,-52.0,M,52.0,M,,0000*5E");
    if (ok && frame.satellites_tracked) {
        //USB.print(line);
        if (frame.satellites_tracked >= GPS_MIN_SATS && frame.hdop.scale > 0) {
            gga = frame;
            return 1;
        }
    }

    return 0;
}

static int handle_sentence(const char *line, enum minmea_sentence_id expect)
{
    enum minmea_sentence_id id = minmea_sentence_id(line, true);
    if (id != expect) {
        return 0;
    }

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
        if (cr.timeout(t0, GPS_TIMEOUT * 1000L)) {
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
    if (time_ok && time >= 2) {
        RTC.setTime(rmc.date.year, rmc.date.month, rmc.date.day,
                    RTC.dow(rmc.date.year, rmc.date.month, rmc.date.day),
                    rmc.time.hours, rmc.time.minutes, rmc.time.seconds);
        _epoch = RTC.getEpochTime();
        _epoch_millis = millis();
    }

    // Start SD card
    startSD();

    // Time
    if (time_ok) {
        if (time >= 2) {
            log_info("GPS Time updated!");
        }
        else {
            log_debug("GPS time=%d/%d/%d %d:%d:%d.%d",
                rmc.date.year,
                rmc.date.month,
                rmc.date.day,
                rmc.time.hours,
                rmc.time.minutes,
                rmc.time.seconds,
                rmc.time.microseconds
            );
        }
    }

    // Position
    if (position_ok) {
        float lat = minmea_tocoord(&gga.latitude);
        float lon = minmea_tocoord(&gga.longitude);
        float alt = minmea_tofloat(&gga.altitude);
        float acc = minmea_tofloat(&gga.hdop);
        int sat = gga.satellites_tracked;

        // Debug
        Utils.float2String(lat, buffer, 6);
        log_debug("GPS lat=%s", buffer);
        Utils.float2String(lon, buffer, 6);
        log_debug("GPS lng=%s", buffer);
        Utils.float2String(alt, buffer, 6);
        log_debug("GPS alt=%s", buffer);
        Utils.float2String(acc, buffer, 6);
        log_debug("GPS sat=%d acc=%s", sat, buffer);

        // Frames
        if (position >= 2) {
            ADD_SENSOR(SENSOR_GPS, lat, lon);
            ADD_SENSOR(SENSOR_ALTITUDE, alt)
            ADD_SENSOR(SENSOR_GPS_ACCURACY, gga.satellites_tracked, acc);
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
