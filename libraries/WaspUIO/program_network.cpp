#include "WaspUIO.h"

CR_TASK(taskNetwork)
{
  static tid_t tid;

  // Send, once every 3 hours if low battery  and lithium battery
  bool send;
  if (UIO.batteryType == 1)
  {
    send = (
      UIO.hasSD &&
      (UIO.batteryLevel > 75) || (UIO.batteryLevel > 65 && UIO.time.hour % 3 == 0)
    );
  }

  // send period for Lead Acid battery
  else if (UIO.batteryType == 2)
  {
    send = UIO.hasSD;
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
  //CR_SPAWN(taskNetworkReceive);

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
  SdFile archive;
  uint32_t fileSize;
  uint8_t item[8];
  uint32_t t0;
  char dataFilename[18]; // /data/YYMMDD.txt
  int size;

  CR_BEGIN;

  UIO.startSD();

  // Delay sending of frame by a random time within 50 to 550 ms to avoid
  // jaming the network. XXX
  // CR_DELAY(rand() % 500);

  // Security check, the file size must be a multiple of 8. If it is not we
  // consider there has been a write error, and we trunctate the file.
  fileSize = UIO.tmpFile.fileSize();
  if (fileSize % 8 != 0)
  {
    UIO.tmpFile.truncate(fileSize - fileSize % 8);
    warn(F("sendFrames: wrong file size (%s), truncated"), UIO.tmpFilename);
  }

  // Send frames
  while (UIO.tmpFile.fileSize() && (! cr.timeout(UIO.start, UIO.send_timeout * 1000)))
  {
    t0 = millis();

    // Read the frame length
    UIO.tmpFile.seekEnd(-8);
    if (UIO.tmpFile.read(item, 8) != 8)
    {
      error(F("sendFrames (%s): read error"), UIO.tmpFilename);
      CR_ERROR;
    }

    // Read the frame
    UIO.getDataFilename(dataFilename, item[0], item[1], item[2]);
    if (!SD.openFile((char*)dataFilename, &archive, O_RDONLY))
    {
      error(F("sendFrames: fail to open %s"), dataFilename);
      CR_ERROR;
    }
    archive.seekSet(*(uint32_t *)(item + 3));
    size = archive.read(SD.buffer, (size_t) item[7]);
    archive.close();

    if (size < 0 || size != (int) item[7])
    {
      error(F("sendFrames: fail to read frame from disk %s"), dataFilename);
      CR_ERROR;
    }

    // Send the frame
    if (xbeeDM.send((char*)UIO.network.rx_address, (uint8_t*)SD.buffer, size) == 1)
    {
      warn(F("sendFrames: Send failure"));
      CR_ERROR;
    }

    // Truncate (pop)
    if (UIO.tmpFile.truncate(UIO.tmpFile.fileSize() - 8) == false)
    {
      error(F("sendFrames: error in tmpFile.truncate"));
      CR_ERROR;
    }

    debug(F("Frame sent in %lu ms"), cr.millisDiff(t0));

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}

CR_TASK(taskNetworkReceive)
{
  char sourceMAC[17];

  CR_BEGIN;

  while (xbeeDM.XBee_ON)
  {
    if (xbeeDM.available())
    {
      debug(F("receivePacket: data available"));
      // Data is expected to be available before calling this method, that's
      // why we only timout for 50ms, much less should be enough (to be
      // tested).
      if (xbeeDM.receivePacketTimeout(100))
      {
        warn(F("receivePacket: timeout (we will retry)"));
      }
      else
      {
        // RSSI
        UIO.readRSSI2Frame();

        // Proxy call to appropriate handler
        if (strstr((const char*)xbeeDM._payload, "GPS_sync") != NULL)
        {
          UIO.receiveGPSsyncTime();
        }
        else
        {
          warn(F("receivePacket: unexpected packet"));
          // Show data stored in '_payload' buffer indicated by '_length'
          debug(F("Data: %s"), xbeeDM._payload);
          // Show data stored in '_payload' buffer indicated by '_length'
          debug(F("Length: %d"), xbeeDM._length);
          // Show data stored in '_payload' buffer indicated by '_length'
          Utils.hex2str(xbeeDM._srcMAC, sourceMAC, 8);
          debug(F("Source MAC Address: %s"), sourceMAC);
        }
      }
    }

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}
