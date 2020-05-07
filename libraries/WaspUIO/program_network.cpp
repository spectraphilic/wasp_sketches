#include "WaspUIO.h"


#if WITH_4G
CR_TASK(taskNetwork4G)
{
  int size;
  uint32_t t0;
  uint8_t status;
  uint8_t n;

  CR_BEGIN;

  // Check we've at least 30s to send something
  if (cr.timeout(UIO._epoch_millis, SEND_TIMEOUT - 30000L))
  {
    warn(F("No time left to send anything"));
    CR_RETURN;
  }

  if (UIO._4GStart())
  {
    error(F("_4GStart() failed"));
    CR_ERROR;
  }

  // Send frames
  debug(F("Sending frames..."));
  while (!cr.timeout(UIO._epoch_millis, SEND_TIMEOUT))
  {
    t0 = millis();

    // Read
    size = UIO.readFrame(n);
    if (size <= 0) { break; }

    // Send
    status = _4G.sendFrameToMeshlium((char*)"wsn.latice.eu", 80, (uint8_t*)SD.buffer, size);
    if (status)
    {
      warn(F("_4G.sendFrameToMeshlium error=%d %d"), status, _4G._errorCode);
      break;
    }
    if (_4G._httpCode != 200) // Check HTTP status code of the response
    {
      error(F("Unexpect HTTP code %d"), _4G._httpCode);
      break;
    }

    // Next
    debug(F("%d frame(s) sent in %lu ms"), n, cr.millisDiff(t0));
    UIO.ack_wait = n;
    cmdAck(""); // Move to the next frame

    CR_DELAY(50); // Give control back
  }

  // Set time from network
  status = UIO.setTimeFrom4G();
  UIO._4GStop(); // Switch off

  CR_END;
}
#endif


#if WITH_IRIDIUM
CR_TASK(taskNetworkIridium)
{
  int size;
  uint32_t t0;
  int status;
  int quality;
  uint8_t n;

  CR_BEGIN;

  // Check we've at least 1m30 to send something
  if (cr.timeout(UIO._epoch_millis, SEND_TIMEOUT - 90000L))
  {
    warn(F("No time left to send anything"));
    CR_RETURN;
  }

  status = UIO.iridium_start(); // This takes ~700ms
  if (status != ISBD_SUCCESS)
  {
    CR_ERROR;
  }

  // Check signal quality, informational
  status = iridium.getSignalQuality(quality); // This takes ~4s
  if (status != ISBD_SUCCESS)
  {
    error(F("iridium.getSignalQuality(..) error=%d"), status);
    UIO.iridium_stop();
    CR_ERROR;
  }
  debug(F("Iridium signal quality %d"), quality);
  if (quality == 0)
  {
    UIO.iridium_stop();
    CR_ERROR;
  }

  // Send frames
  debug(F("Sending frames..."));
  while (!cr.timeout(UIO._epoch_millis, SEND_TIMEOUT))
  {
    t0 = millis();

    // Read
    size = UIO.readFrame(n);
    if (size <= 0) { break; }

    // Send
    status = iridium.sendSBDBinary((uint8_t*)SD.buffer, size);
    if (status != ISBD_SUCCESS)
    {
      warn(F("iridium.sendSBDBinary(..) error=%d"), status);
      break;
    }

    // Next
    debug(F("%d frame(s) sent in %lu ms"), n, cr.millisDiff(t0));
    UIO.ack_wait = n;
    cmdAck(""); // Move to the next frame

    CR_DELAY(50); // Give control back
  }

  // Switch off
  UIO.iridium_stop();

  CR_END;
}
#endif


#if WITH_XBEE
CR_TASK(taskNetworkXBee)
{
  static tid_t tid;
  uint32_t wait;

  CR_BEGIN;

  if (!xbeeDM.XBee_ON)
  {
    if (xbeeDM.ON())
    {
      error(F("startNetwork: xbeeDM.ON() failed"));
      CR_ERROR;
    }
  }
  info(F("Network started"));

  // Spawn first the receive task
  CR_SPAWN(taskNetworkXBeeReceive);

  // Schedule sending frames
  if (UIO.hasSD)
  {
    CR_SPAWN2(taskNetworkXBeeSend, tid);
  }

  // Keep the network open, default is 45s
  wait = UIO.lan_wait ? (UIO.lan_wait * 1000UL) : 45000UL;
  CR_DELAY(wait);

  if (UIO.hasSD)
  {
    CR_JOIN(tid);
  }

  // RSSI
  if (UIO.xbeeQuality() == 0) // FIXME This outputs garbage to USB ??
  {
    ADD_SENSOR(SENSOR_RSSI, UIO.rssi);
  }

  // Stop network
  if (xbeeDM.XBee_ON)
  {
    xbeeDM.OFF();
    info(F("Network stopped"));
  }

  CR_END;
}

CR_TASK(taskNetworkXBeeSend)
{
  uint32_t t0;
  static unsigned long sent;
  uint8_t n;

  CR_BEGIN;
  UIO.ack_wait = 0;

  // Send frames
  while (!cr.timeout(UIO._epoch_millis, SEND_TIMEOUT))
  {
    if (UIO.ack_wait == 0)
    {
      // Read
      int size = UIO.readFrame(n);
      if (size <= 0) { break; }

      // Send the frame
      t0 = millis();
      if (xbeeDM.send((char*)UIO.xbee.rx_address, (uint8_t*)SD.buffer, size) == 1)
      {
        warn(F("xbeeDM.send(..) failure"));
        break;
      }
      sent = millis();

      // Next
      debug(F("%d frame(s) sent in %lu ms"), n, cr.millisDiff(t0));
      UIO.ack_wait = n;
    }
    else if (cr.timeout(sent, 10 * 1000))
    {
      break; // If waiting for an ACK more than 10s, stop sending.
    }

    CR_DELAY(50); // Give control back
  }

  CR_END;
}

CR_TASK(taskNetworkXBeeReceive)
{
  char sourceMAC[17];

  CR_BEGIN;

  while (xbeeDM.XBee_ON)
  {
    if (xbeeDM.available())
    {
      // Data is expected to be available before calling this method, that's
      // why we only timeout for 100ms, less should be enough
      if (xbeeDM.receivePacketTimeout(100))
      {
        warn(F("receivePacket: timeout (we will retry)"));
      }
      else
      {
        Utils.hex2str(xbeeDM._srcMAC, sourceMAC, 8);
        exeCommands((const char*)xbeeDM._payload, false);
      }
    }

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}
#endif

#if WITH_LORA
CR_TASK(taskNetworkLora)
{
  static tid_t tid;
  uint32_t wait;

  CR_BEGIN;

  if (! UIO.hasSD)
  {
    CR_ERROR;
  }

  if (UIO.loraStart())
  {
    error(F("taskNetworkLora loraStart() failed"));
    CR_ERROR;
  }
  info(F("Network started"));

  // Schedule sending frames. Don't send with Lora if I'm the gateway.
  if (UIO.lora_addr != 1)
  {
    CR_SPAWN2(taskNetworkLoraSend, tid);
  }

  // Receive: Senders need to listen as well for cmdAck
  CR_SPAWN(taskNetworkLoraReceive);

  // Keep the network open, default is 45s
  wait = UIO.lan_wait ? (UIO.lan_wait * 1000UL) : 45000UL;
  CR_DELAY(wait);

  if (UIO.lora_addr != 1)
  {
    CR_JOIN(tid);
  }

  // RSSI
  if (UIO.loraQuality() == 0)
  {
    ADD_SENSOR(SENSOR_RSSI, UIO.rssi);
    //ADD_SENSOR(SENSOR_SNR, UIO.snr);
  }

  UIO.loraStop();
  info(F("Network stopped"));

  CR_END;
}

CR_TASK(taskNetworkLoraSend)
{
  static uint32_t t0;
  static uint8_t dst;
  static uint8_t n_packages; // Number of packages sent in one loop
  uint8_t n_frames;
  int offset;

  CR_BEGIN;
  UIO.ack_wait = 0;
  n_packages = 0;

  // Higher modes have lower time-on-air, this improves channel availability,
  // so we should have higher rates of successful messages sent; choose the
  // higher mode that has enough range). See pages 41-42.

  // Wait at least 5s before sending, so when we send there's a fair chance
  // for the receiver to be listening.
  // - Add a fixed offset depending on address to reduce the chance of collisions?
  // - Add a random value to reduce the chance of collisions?
  offset = 5000;//+ rand() % 1001;
  info(F("Lora send: wait %d ms before sending"), offset);
  CR_DELAY(offset); // 200-1200 ms

  // Discover destination address, auto mode routing (lora.dst 0)
  dst = UIO.lora_dst ? UIO.lora_dst : UIO.lora_dst2;
  if (dst == 0)
  {
    if (UIO.loraSend(UIO.lora_dst, "ping", true))
    {
      CR_ERROR;
    }
    else
    {
      dst = UIO.lora_dst2;
      info(F("Lora auto mode dst=%hhu"), dst);
    }
  }

  // Send frames
  while (!cr.timeout(UIO._epoch_millis, SEND_TIMEOUT))
  {
    if (UIO.ack_wait == 0)
    {
      // Read
      int size = UIO.readFrame(n_frames);
      if (size <= 0) { break; }

      // Send the frame. The timeout is calculated by sx1272
      t0 = millis();
      if (sx1272.sendPacketTimeout(dst, (uint8_t*)SD.buffer, size))
      {
        warn(F("sx1272.send(..) failure timeout=%u"), sx1272._sendTime);
        break;
      }
      n_packages++;
      info(F("Sent packet=%hhu with %hhu frame(s) to dst=%hhu in %lu ms"),
           sx1272.packet_sent.packnum, n_frames, dst, cr.millisDiff(t0));

      // Debug
//    char str[50];
//    debug(F("length=%u time-on-air=%s"),
//          sx1272._payloadlength,
//          Utils.float2String(sx1272.timeOnAir(), str, 2)
//    );

      // Next
      UIO.ack_wait = n_frames;
    }
    else if (cr.timeout(t0, 10 * 1000))
    {
      // Max 10s waiting for an ACK
      // If didn't receive the ACK for the 1st frame consider it a failure
      if (n_packages == 1)
      {
        UIO.lora_fails++;
        warn(F("Didn't get an ACK in this loop, fails=%hhu"), UIO.lora_fails);
        // If too many fails reset dst2 (this only has an effect in auto routing mode)
        if (UIO.lora_fails >= LORA_MAX_FAILS)
        {
          UIO.lora_dst2 = 0;
        }
      }
      break;
    }

    CR_DELAY(100); // Give control back
  }

  CR_END;
}

CR_TASK(taskNetworkLoraReceive)
{
  const char *data;

  CR_BEGIN;

  while (SPI.isSocket0)
  {
    if (sx1272.receivePacketTimeout() == 0)
    {
      data = (const char*)sx1272.packet_received.data;
      if (strncmp("ping", data, 4) == 0) {
        info(F("ping received from lora address=%u"), sx1272.packet_received.src);
        if (sx1272.packet_received.dst == UIO.lora_addr || sx1272.packet_received.src > UIO.lora_addr)
        {
          if (sx1272.setACK() || sx1272.sendWithTimeout())
          {
            warn(F("Lora: Failed to send ACK"));
          }
        }
        else
        {
          info(F("ignore ping"));
        }
      } else if (strncmp("<=>", data, 3) == 0) {
        info(F("frame received from lora address=%u"), sx1272.packet_received.src);
        uint8_t n = UIO.saveFrames(
          sx1272.packet_received.src,
          sx1272.packet_received.data,
          sx1272.packet_received.length - 5 // dst(1) + src(1) + packnum(1) + length(1) + data + retry(1)
        );
        if (n > 0)
        {
          const int size = 25;
          char cmd[size];
          float timeOnAir = sx1272.timeOnAir(21); // strlen(cmd)
          uint32_t time = UIO.getEpochTime() + (uint32_t)round(timeOnAir/1000);
          snprintf_F(cmd, size, F("time %lu;ack %u"), time, n);
          UIO.loraSend(sx1272.packet_received.src, cmd, false);
        }
      } else {
        //info(F("command received from lora address=%u"), sx1272.packet_received.src);
        exeCommands(data, false);
      }

/*
      sx1272.showReceivedPacket();
      info(F("Packet received from Lora network"));
      info(F("dst=%u src=%u packnum=%u length=%u retry=%u"),
        sx1272.packet_received.dst,
        sx1272.packet_received.src,
        sx1272.packet_received.packnum,
        sx1272.packet_received.length,
        sx1272.packet_received.retry,
      );
*/

    }

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}
#endif
