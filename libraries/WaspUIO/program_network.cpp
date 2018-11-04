#include "WaspUIO.h"


CR_TASK(taskNetwork4G)
{
  CR_BEGIN;

#if WITH_4G
  if (UIO._4GStart())
  {
    CR_ERROR;
  }

  // Send frames
  debug(F("4G Sending frames..."));
  while (!cr.timeout(UIO.start, UIO.send_timeout * 1000)) // 3min max sending frames
  {
    t0 = millis();

    int size = UIO.readFrame();
    if (size < 0)
    {
      err = true;
      break;
    }
    if (size == 0)
    {
      break;
    }

    // Send the frame
    err = _4G.sendFrameToMeshlium((char*)"wsn.latice.eu", 80, (uint8_t*)SD.buffer, size);
    if (err)
    {
      cr.set_last_error(F("_4G.sendFrameToMeshlium error=%d %d"), err, _4G._errorCode);
      break;
    }

    // Check HTTP status code of the response
    if (_4G._httpCode != 200)
    {
      cr.set_last_error(F("Unexpect HTTP code %d"), _4G._httpCode);
      err = 1;
      break;
    }

    // Next
    UIO.qstartFile.close(); // Close files
    UIO.queueFile.close();
    debug(F("Frame %hhu sent in %lu ms"), UIO.getSequence((uint8_t*)SD.buffer), cr.millisDiff(t0));

    UIO.ack_wait = true;
    cmdAck(""); // Move to the next frame

    CR_DELAY(50); // Give control back
  }

  // Switch off and close files
  UIO._4GStop();
  if (UIO.qstartFile.isOpen()) { UIO.qstartFile.close(); }
  if (UIO.queueFile.isOpen())  { UIO.queueFile.close(); }

  if (err)
  {
    error(cr.last_error);
    CR_ERROR;
  }

#else
  error(F("4G not enabled, define WITH_4G TRUE"));
  CR_ERROR;
#endif

  CR_END;
}


CR_TASK(taskNetworkXBee)
{
  static tid_t tid;

  // Send, once every 3 hours if low battery  and lithium battery
  bool send = false;
  if (UIO.hasSD)
  {
    send = (
      (UIO.battery == BATTERY_HIGH) ||
      (UIO.battery == BATTERY_MIDDLE && UIO.epochTime % (3*60*60) == 0)
    );
  }

  CR_BEGIN;

#if WITH_XBEE
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
  CR_SPAWN(taskNetworkReceive);

  // Schedule sending frames
  if (send)
  {
    CR_SPAWN2(taskNetworkSend, tid);
  }

  CR_DELAY(8000); // Keep the network open at least for 8s

  if (send)
  {
    CR_JOIN(tid);
  }

  // Stop network
  if (xbeeDM.XBee_ON)
  {
    xbeeDM.OFF();
    info(F("Network stopped"));
  }
#else
  error(F("XBee not enabled, define WITH_XBEE TRUE"));
  CR_ERROR;
#endif

  CR_END;
}

CR_TASK(taskNetworkSend)
{
#if WITH_XBEE
  uint32_t t0;
  bool err = false;
  static unsigned long sent;

  CR_BEGIN;
  UIO.ack_wait = false;

  // Send frames
  while (!cr.timeout(UIO.start, UIO.send_timeout * 1000)) // 3min max sending frames
  {
    if (UIO.ack_wait == false)
    {
      t0 = millis();

      int size = UIO.readFrame();
      if (size < 0)
      {
        err = true;
        break;
      }
      if (size == 0)
      {
        break;
      }

      // Send the frame
      if (xbeeDM.send((char*)UIO.xbee.rx_address, (uint8_t*)SD.buffer, size) == 1)
      {
        warn(F("sendFrames: Send failure"));
        break;
      }
      UIO.ack_wait = true;
      sent = millis();

      // Next
      UIO.qstartFile.close(); // Close files
      UIO.queueFile.close();
      debug(F("Frame %hhu sent in %lu ms"), UIO.getSequence((uint8_t*)SD.buffer), cr.millisDiff(t0));
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

  // Close files
  if (UIO.qstartFile.isOpen()) { UIO.qstartFile.close(); }
  if (UIO.queueFile.isOpen())  { UIO.queueFile.close(); }

  if (err)
  {
    error(cr.last_error);
    CR_ERROR;
  }

  CR_END;
#else
  error(F("XBee not enabled, define WITH_XBEE TRUE"));
  return CR_TASK_ERROR;
#endif
}

CR_TASK(taskNetworkReceive)
{
#if WITH_XBEE
  char sourceMAC[17];
  cmd_status_t status;

  CR_BEGIN;

  while (xbeeDM.XBee_ON)
  {
    if (xbeeDM.available())
    {
      // Data is expected to be available before calling this method, that's
      // why we only timout for 50ms, much less should be enough (to be
      // tested).
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
#else
  error(F("XBee not enabled, define WITH_XBEE TRUE"));
  return CR_TASK_ERROR;
#endif
}
