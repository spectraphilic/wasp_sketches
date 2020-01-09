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
      t0 = millis();

      // Read
      int size = UIO.readFrame(n);
      if (size <= 0) { break; }

      // Send the frame
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
    else
    {
      // Do not wait more than 5s
      if (cr.timeout(sent, 5 * 1000))
      {
         break;
      }
    }

    CR_DELAY(50); // Give control back
  }

  CR_END;
}

CR_TASK(taskNetworkXBeeReceive)
{
  char sourceMAC[17];
  cmd_status_t status;

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
        status = exeCommand((const char*)xbeeDM._payload);
        if (status == cmd_bad_input)
        {
          warn(F("unexpected frame received from %s"), sourceMAC);
        }
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

  if (UIO.loraStart())
  {
    error(F("taskNetworkLora loraStart() failed"));
    CR_ERROR;
  }
  info(F("Network started"));

  // Spawn first the receive task
  CR_SPAWN(taskNetworkLoraReceive);

  // Schedule sending frames
  // Do not send with Lora if I'm the gateway
  if (UIO.hasSD && UIO.lora_addr != 1)
  {
    CR_SPAWN2(taskNetworkLoraSend, tid);
  }

  // Keep the network open, default is 45s
  wait = UIO.lan_wait ? (UIO.lan_wait * 1000UL) : 45000UL;
  CR_DELAY(wait);

  if (UIO.hasSD && UIO.lora_addr != 1)
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

  // TODO See whether sx1272._sendTime changes much in every call, if it
  // doesn't set it once here (probably use it as well for receving data)

  // TODO Randomize sending time? Because only one packet can be in the network
  // at the same time. Also consider higher modes to reduce the time in the air,
  // so to improve the network availability. See pages 41-42

  // Send frames
  while (!cr.timeout(UIO._epoch_millis, SEND_TIMEOUT))
  {
    if (UIO.ack_wait == 0)
    {
      t0 = millis();

      // Read
      int size = UIO.readFrame(n);
      if (size <= 0) { break; }

      // Send the frame. If not given explicitely (last parameter) the timeout
      // will be automatically calculated; sending is aborted if the timeout is
      // reached.
      if (sx1272.sendPacketTimeout(1, (uint8_t*)SD.buffer, size))
      {
        debug(F("XXX sx1272._sendTime = %u"), sx1272._sendTime); // XXX
        warn(F("xbeeDM.send(..) failure"));
        break;
      }
      debug(F("XXX sx1272._sendTime = %u"), sx1272._sendTime); // XXX
      sent = millis();

      // Next
      debug(F("%d frame(s) sent in %lu ms"), n, cr.millisDiff(t0));
      UIO.ack_wait = n;
    }
    else
    {
      // Do not wait more than 5s
      if (cr.timeout(sent, 5 * 1000))
      {
         break;
      }
    }

    CR_DELAY(50); // Give control back
  }

  CR_END;
}

CR_TASK(taskNetworkLoraReceive)
{
  cmd_status_t status;
  const char *data;

  CR_BEGIN;

  while (SPI.isSocket0)
  {
    if (sx1272.availableData(10)) // If timeout not given max will be used!
    {
      // Data is expected to be available before calling this method, that's
      // why we only timeout for 100ms, less should be enough
      // TODO Find out how much timeout is auto-caculated
      // TODO For a multi-hop setup use receiveAll (promiscous mode, page 36)
      if (sx1272.receivePacketTimeout())
      {
        debug(F("XXX sx1272._sendTime = %u"), sx1272._sendTime); // XXX
        warn(F("receivePacket: timeout (we will retry)"));
      }
      else
      {
        debug(F("XXX sx1272._sendTime = %u"), sx1272._sendTime); // XXX
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
        if (strncmp("<=>", data, 3) == 0) {
          // TODO Save the frame with the correct filepath and send ACK command
          UIO.saveFrame(
            sx1272.packet_received.src,
            sx1272.packet_received.data,
            sx1272.packet_received.length
          );

        } else {
          status = exeCommand(data);
          if (status == cmd_bad_input)
          {
            warn(F("unexpected frame received from %u"), sx1272.packet_received.src);
          }
        }
      }
    }

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}
#endif
