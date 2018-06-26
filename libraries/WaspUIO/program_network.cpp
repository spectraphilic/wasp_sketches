#include "WaspUIO.h"


CR_TASK(taskNetwork4G)
{
  CR_BEGIN;

  warn(F("4G networking not yet implemented"));

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
      (UIO.battery == BATTERY_MIDDLE && UIO.minute % 180 == 0)
    );
  }

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

  CR_END;
}

CR_TASK(taskNetworkSend)
{
  static uint32_t offset;
  uint8_t item[8];
  uint32_t t0;
  char dataFilename[18]; // /data/YYMMDD.txt
  SdFile dataFile;
  int size;
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

      // Open files
      UIO.startSD();
      if (UIO.openFile(UIO.qstartFilename, UIO.qstartFile, O_READ))
      {
        err = true;
        break;
      }
      if (UIO.openFile(UIO.queueFilename, UIO.queueFile, O_READ))
      {
        err = true;
        break;
      }

      // Read offset
      if (UIO.qstartFile.read(item, 4) != 4)
      {
        cr.set_last_error(F("sendFrames (%s): read error"), UIO.qstartFilename);
        err = true;
        break;
      }
      offset = *(uint32_t *)item;
      if (offset >= UIO.queueFile.fileSize())
      {
        break;
      }

      // Read the record
      UIO.queueFile.seekSet(offset);
      if (UIO.queueFile.read(item, 8) != 8)
      {
        cr.set_last_error(F("sendFrames (%s): read error"), UIO.queueFilename);
        err = true;
        break;
      }

      // Read the frame
      UIO.getDataFilename(dataFilename, item[0], item[1], item[2]);
      if (!SD.openFile((char*)dataFilename, &dataFile, O_READ))
      {
        cr.set_last_error(F("sendFrames: fail to open %s"), dataFilename);
        err = true;
        break;
      }
      dataFile.seekSet(*(uint32_t *)(item + 3));
      size = dataFile.read(SD.buffer, (size_t) item[7]);
      dataFile.close();

      if (size < 0 || size != (int) item[7])
      {
        cr.set_last_error(F("sendFrames: fail to read frame from disk %s"), dataFilename);
        err = true;
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
}

CR_TASK(taskNetworkReceive)
{
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
}
