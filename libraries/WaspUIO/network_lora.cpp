#include "WaspUIO.h"


#if WITH_LORA
int WaspUIO::loraStart()
{
  int err;

  err = sx1272.ON();
  if (err) { cr_printf("sx1272.ON() error=%d\n", err); return err; }

  // Configuration is not saved in the module, so the module must be configured
  // every time it's switched ON

  err = sx1272.setMode(lora_mode);
  if (err) { cr_printf("sx1272.setMode(%u) error=%d\n", lora_mode, err); goto exit; }

  err = sx1272.setChannel(LORA_CHANNEL);
  if (err) { cr_printf("sx1272.setChannel(..) error=%d\n", err); goto exit; }

  err = sx1272.setNodeAddress(lora_addr); // 1-255
  if (err) { cr_printf("sx1272.setNodeAddress(%u) error=%d\n", lora_addr, err); goto exit; }

  err = sx1272.setPower(LORA_POWER);
  if (err) { cr_printf("sx1272.setPower(%u) error=%d\n", lora_addr, err); goto exit; }

  // Used for sending
  err = sx1272.setHeaderON();
  if (err) { cr_printf("sx1272.setHeaderON() error=%d\n", err); goto exit; }

  err = sx1272.setCRC_ON();
  if (err) { cr_printf("sx1272.setCRC_ON() error=%d\n", err); goto exit; }

  // Read registers (read values should match the values above, for those that
  // have been set)
  err = sx1272.getRegs();
  if (err) { cr_printf("sx1272.getRegs() error=%d\n", err); goto exit; }

exit:
  if (err) { loraStop(); }
  return err;
}


void WaspUIO::loraStop()
{
  // This is safe. The SD card uses SPI as well, but SPI.close() will only
  // close SPI if all devices (SD and Socket0) are closed.
  sx1272.OFF();
}


int WaspUIO::loraInit()
{
  int err = loraStart();
  if (err) { return err; }

  loraStop();
  return err;
}

int WaspUIO::loraSend(uint8_t dst, const char* msg, bool ack)
{
    int err;

    log_debug("loraSend(dst=%d, msg=\"%s\", ack=%d)", dst, msg, ack);

    // UART0 is shared by USB and Socket0 (XBee, Lora)
    USB.OFF();

    // Switch ON
    bool isOn = SPI.isSocket0;
    if (! isOn) {
        err = loraStart();
        if (err)
            goto exit;
    }

    if (ack) {
        //err = sx1272.sendPacketTimeoutACK(dst, (char*)msg);
        err = sx1272.sendPacketMAXTimeoutACK(dst, (char*)msg);
        //err = sx1272.sendPacketMAXTimeoutACKRetries(dst, (char*)msg);
        if (err == 0)
            err = loraQuality();
    } else {
        err = sx1272.sendPacketTimeout(dst, (char*)msg);
    }

exit:
    if (! isOn)
        loraStop();

    // Print
    //USB.ON();
    //USB.flush();
    if (err) {
        log_error("loraSend failed error=%d", err);
        return 1;
    }

    // Set network address in auto mode (lora.dst 0)
    if (ack && dst == 0) {
        log_info("loraSend success, ack received from %u", sx1272.ACK.src);
        if (sx1272.ACK.src < lora_addr)
            lora_dst2 = sx1272.ACK.src;
    }

    return 0;
}

int WaspUIO::loraQuality()
{
    int err = sx1272.getRSSI() or sx1272.getRSSIpacket() or sx1272.getSNR();
    if (err == 0) {
        rssi = sx1272._RSSI;
        rssi_packet = sx1272._RSSIpacket;
        snr = sx1272._SNR;
    }

    return err;
}

#endif
