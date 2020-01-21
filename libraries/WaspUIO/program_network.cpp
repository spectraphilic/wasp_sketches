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

  // Receive: Senders need to listen as well for cmdAck
  CR_SPAWN(taskNetworkLoraReceive);

  // Schedule sending frames. Don't send with Lora if I'm the gateway.
  if (UIO.lora_addr != 1)
  {
    CR_SPAWN2(taskNetworkLoraSend, tid);
  }

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
  uint32_t t0;
  static unsigned long sent;
  uint8_t n;

  CR_BEGIN;
  UIO.ack_wait = 0;

  // Higher modes have lower time-on-air, this improves channel availability,
  // so we should have higher rates of successful messages sent; choose the
  // higher mode that has enough range). See pages 41-42.

  // Wait 200ms to let the GW time to start, plus a random value to avoid
  // all motes to start sending at the same time.
  CR_DELAY(200 + rand() % 1001); // 200-1200 ms

  // Send frames
  while (!cr.timeout(UIO._epoch_millis, SEND_TIMEOUT))
  {
    if (UIO.ack_wait == 0)
    {
      // Read
      int size = UIO.readFrame(n);
      if (size <= 0) { break; }

      //Utils.blinkGreenLED(500, 3);
      // Send the frame. The timeout is calculated by sx1272
      t0 = millis();
      if (sx1272.sendPacketTimeout(1, (uint8_t*)SD.buffer, size))
      {
        //Utils.blinkRedLED(500, 3);
        warn(F("sx1272.send(..) failure (sx1272._sendTime=%u)"), sx1272._sendTime);
        break;
      }
      sent = millis();

      // Next
      debug(F("%d frame(s) sent in %lu ms (sx1272._sendTime=%u)"), n, cr.millisDiff(t0), sx1272._sendTime);
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

CR_TASK(taskNetworkLoraReceive)
{
  const char *data;
  uint8_t err;

  CR_BEGIN;

  //Utils.blinkGreenLED(500, 3);
  while (SPI.isSocket0)
  {
    // Data is expected to be available before calling this method, that's
    // why we only timeout for 100ms, less should be enough
    // TODO For a multi-hop setup use receiveAll (promiscous mode, page 36)
    if (sx1272.receivePacketTimeout(200) == 0)
    {
      debug(F("Packet received!! sx1272._sendTime = %u"), sx1272._sendTime);
      sx1272.showReceivedPacket();
/*
      info(F("Packet received from Lora network"));
      info(F("dst=%u src=%u packnum=%u length=%u retry=%u"),
        sx1272.packet_received.dst,
        sx1272.packet_received.src,
        sx1272.packet_received.packnum,
        sx1272.packet_received.length,
        sx1272.packet_received.retry,
      );
*/

      data = (const char*)sx1272.packet_received.data;
      if (strncmp("ping", data, 4) == 0) {
        info(F("ping received from lora network address=%u"), sx1272.packet_received.src);
        if (sx1272.setACK() || sx1272.sendWithTimeout())
        {
          warn(F("Lora: Failed to send ACK"));
        }
      } else if (strncmp("<=>", data, 3) == 0) {
        uint8_t n = UIO.saveFrames(
          sx1272.packet_received.src,
          sx1272.packet_received.data,
          sx1272.packet_received.length - 5 // dst(1) + src(1) + packnum(1) + length(1) + data + retry(1)
        );
        if (n > 0)
        {
          //Utils.blinkLEDs(1000);
          const int size = 25;
          char cmd[size];
          snprintf_F(cmd, size, F("ack %u;time %lu"), n, UIO.getEpochTime());
          UIO.loraSend(sx1272.packet_received.src, cmd, false);
        }
      } else {
        exeCommands((char*)data, false);
      }
    }

    // Give control back
    CR_DELAY(0);
  }
  //Utils.blinkRedLED(500, 3);

  CR_END;
}
#endif
