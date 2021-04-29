#include <WaspUIO.h>

/*
 * Every 30s this sketch produces a frame with the RSSI, GPS location and
 * altitude.
 *
 * A frame is only generated if the mote can get both the RSSI and GPS. Note
 * that getting the GPS data may take more than 30s. For these 2 reasons the
 * final values may not be once every 30s.
 *
 * The frame is not sent, just stored. When done running the main sketch can be
 * used to send the frames.
 *
 * To get the RSSI we have to ping (send a frame). The frame is sent to the hub
 * (Pi), but the RSSI only concerns the first hop. The hub will ignore this
 * frame, just log it.
 *
 * Please before using this sketch first configure the mote properly with the
 * main sketch, specially the time and name. And logging to the SD, so we can
 * debug it later.
 */


// 0: Disabled
// 1: Enabled, only when needed
// 2: Enabled, always
#define USE_GPS 0
#define USE_SD 1

int gps_on()
{
    if (GPS.getMode() == GPS_ON)
        return 0;

    if (USE_GPS == 0)
        return -1;

    if (GPS.ON() == 0) {
        log_error("GPS.ON() ERROR: reboot!");
        UIO.reboot();
    }

    return 0;
}


void gps_off()
{
    if (GPS.getMode() == GPS_OFF)
        return;

    if (USE_GPS < 2)
        GPS.OFF();
}

int8_t gps(bool getPosition)
{
    PGM_P error = NULL;
    uint8_t satellites;

    // On
    log_info("GPS...");
    gps_on();

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

    gps_off();

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
        gps_off();
        UIO.startSD();

        cr.log_P(LOG_ERROR, error);
        return -1;
    }

    return 0;
}


uint8_t getPowerLevel()
{
    bool success = false;
    uint8_t powerLevel;

    USB.flush();
    USB.OFF();

  // Action
#if WITH_XBEE
    if (xbeeDM.ON() == 0 && xbeeDM.getPowerLevel() == 0) {
        powerLevel = xbeeDM.powerLevel;
        success = true;
    }
    xbeeDM.OFF();
#elif WITH_LORA
    if (UIO.loraStart() == 0 && sx1272.getPower() == 0) {
        powerLevel = sx1272._power;
        success = true;
    }
    UIO.loraStop();
#endif

    USB.ON();
    USB.flush();

    // Print
    if (success) {
        log_info("Power level = %hhu", powerLevel);
        return 0;
    } else {
        log_error("Failed to read the power level");
        return 1;
    }
}


void setup()
{
    // Boot
    USB.ON();
    UIO.boot();
    USB.println();

    // Switch stuff
    RTC.OFF();

    if (USE_SD == 2)
        UIO.startSD();
    else
        UIO.stopSD();

    if (USE_GPS == 2)
        gps_on();
    else
        gps_off();

    // Network power level
    getPowerLevel();
}


void loop()
{
    uint32_t time, wait;
    int err;

    // Wait until the next 30s slot
    wait = 30 - UIO.getEpochTime() % 30; // Every 30s
    log_info("Wait %ds", wait); // Wait until the next minute
    delay(wait * 1000);

    // Get data and create frame
    time = UIO.getEpochTime();
    log_info("Ping...");

#if WITH_XBEE
    err = UIO.xbeeSend(UIO.xbee.rx_address, "ping");
#elif WITH_LORA
    err = UIO.loraSend(UIO.lora_dst, "ping", true);
#endif

    if (err == 0) {
#if WITH_XBEE
        log_info("RSSI=%d", UIO.rssi);
#endif
        frame.createFrameBin(BINARY);
        ADD_SENSOR(SENSOR_TST, time);
        ADD_SENSOR(SENSOR_RSSI, UIO.rssi_packet);
        //ADD_SENSOR(SENSOR_SNR, UIO.snr);
        if (USE_GPS)
            gps(true);

        if (USE_SD) {
            UIO.startSD();
            UIO.saveFrame();
            if (USE_SD < 2)
                UIO.stopSD();
        }
    }
}
