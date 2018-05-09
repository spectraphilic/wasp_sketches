#include "WaspUIO.h"

CR_TASK(taskNetwork)
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

  CR_BEGIN;
  UIO.startSD();

  // Open queue file
  if (UIO.openFile(UIO.queueFilename, UIO.queueFile, O_RDWR | O_CREAT))
  {
    error(cr.last_error);
    CR_ERROR;
  }

  // Security check, the file size must be a multiple of 8. If it is not we
  // consider there has been a write error, and we trunctate the file.
  offset = UIO.queueFile.fileSize() % 8;
  if (offset != 0)
  {
    UIO.queueFile.truncate(UIO.queueFile.fileSize() - offset);
    warn(F("sendFrames: wrong file size (%s), truncated"), UIO.queueFilename);
  }

  // Read offset
  if (UIO.openFile(UIO.qstartFilename, UIO.qstartFile, O_RDWR))
  {
    error(cr.last_error);
    CR_ERROR;
  }
  if (UIO.qstartFile.read(item, 4) != 4)
  {
      error(F("sendFrames (%s): read error"), UIO.qstartFilename);
      CR_ERROR;
  }
  offset = *(uint32_t *)item;

  // Send frames
  while (offset < UIO.queueFile.fileSize() && !cr.timeout(UIO.start, UIO.send_timeout * 1000))
  {
    t0 = millis();

    // Read the record
    UIO.queueFile.seekSet(offset);
    if (UIO.queueFile.read(item, 8) != 8)
    {
      error(F("sendFrames (%s): read error"), UIO.queueFilename);
      CR_ERROR;
    }

    // Read the frame
    UIO.getDataFilename(dataFilename, item[0], item[1], item[2]);
    if (!SD.openFile((char*)dataFilename, &dataFile, O_READ))
    {
      error(F("sendFrames: fail to open %s"), dataFilename);
      CR_ERROR;
    }
    dataFile.seekSet(*(uint32_t *)(item + 3));
    size = dataFile.read(SD.buffer, (size_t) item[7]);
    dataFile.close();

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
    offset += 8;
    if (offset >= UIO.queueFile.fileSize())
    {
      offset = 0;
      if (UIO.queueFile.truncate(0) == false)
      {
        error(F("sendFrames: error in queueFile.truncate"));
        CR_ERROR;
      }
    }

    // Update offset
    UIO.qstartFile.seekSet(0);
    if (UIO.write(UIO.qstartFile, (void*)(&offset), 4))
    {
      error(F("sendFrames: error updating offset"));
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
        debug(F("frame received from %s"), sourceMAC);
        exeCommand((const char*)xbeeDM._payload);
      }
    }

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}
