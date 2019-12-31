/*
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 April 2017, Simon Filhol
*/

#include <WaspUIO.h>


int upgradeFIFO()
{
  if (SD.isFile("TMP.TXT") == -1)
  {
    return 0;
  }
  cr.println(F("Upgrading FIFO..."));

  FIFO old = FIFO("TMP.TXT", "QSTART.BIN", 8);
  //FIFO fifo = FIFO("FIFO.BIN", "FIDX.BIN", 9);

  // Upgrade
  uint8_t item[9] = {0};
  int idx, status, err = 1;

  for (idx=0; true; idx++)
  {
    // Read from old FIFO
    status = old.peek(&item[1], idx);
    if (status == QUEUE_EMPTY || status == QUEUE_INDEX_ERROR) // Stop condition
    {
      err = 0;
      break;
    }
    if (status) { break; } // Error

    // Write to new FIFO
    if (fifo.push(item)) { break; }
  }

  if (err)
  {
    // Redo new
    SD.del("FIFO.BIN");
    SD.del("FIDX.BIN");
    fifo.make();

    cr.println(F("ERROR Upgrading"));
    return 1;
  }

  // Remove old
  SD.del("TMP.TXT");
  SD.del("QSTART.BIN");

  return 0;
}

int upgradeLIFO()
{
  if (SD.isFile("LIFO.BIN") == -1)
  {
    return 0;
  }
  cr.println(F("Upgrading LIFO..."));

  LIFO old = LIFO("LIFO.BIN", 8);
  LIFO lifo = LIFO("LIFO2.BIN", 9);

  // Upgrade
  uint8_t item[9] = {0};
  int idx, status, err = 1;

  for (idx=0; true; idx++)
  {
    // Read from old FIFO
    status = old.peek(&item[1], idx);
    if (status == QUEUE_EMPTY || status == QUEUE_INDEX_ERROR) // Stop condition
    {
      err = 0;
      break;
    }
    if (status) { break; } // Error

    // Write to new FIFO
    if (lifo.push(item)) { break; }
  }

  if (err)
  {
    // Redo new
    SD.del("LIFO2.BIN");
    lifo.make();

    cr.println(F("ERROR Upgrading"));
    return 1;
  }

  // Remove old
  SD.del("LIFO.BIN");

  return 0;
}


void setup()
{
  // Boot process
  UIO.boot();

  // Upgrade queues
  if (UIO.hasSD)
  {
    upgradeFIFO();
#if WITH_IRIDIUM
    upgradeLIFO();
#endif
  }

  // Boot frame
  frame.setID(UIO.name);
  frame.createFrameBin(BINARY); // TODO Move this logic to UIO.createFrame
  frame.setFrameType(INFORMATION_FRAME_V15 + EVENT_FRAME);
  UIO.addSensor(SENSOR_TST, UIO._epoch);
  UIO.saveFrame();
  #if WITH_XBEE
  frame.setID((char*)""); // We only want to send the name once
  #endif

  // Log configuration
  char buffer[150];
  size_t size = sizeof(buffer);
  info(F("Time      : %s"), UIO.pprintTime(buffer, size));
  info(F("Id        : %s Version=%c Name=%s"), UIO.pprintSerial(buffer, sizeof buffer), _boot_version, UIO.name);
  info(F("Battery   : %s"), UIO.pprintBattery(buffer, size));
  info(F("Hardware  : board=%s SD=%d GPS=%d"), UIO.pprintBoard(buffer, size), UIO.hasSD, UIO.hasGPS);
  #if WITH_XBEE
  if (UIO.lan_type == LAN_XBEE)
  { info(F("XBee      : %s"), UIO.pprintXBee(buffer, size)); }
  #endif
  #if WITH_LORA
  if (UIO.lan_type == LAN_LORA)
  { info(F("Lora      : %s"), UIO.pprintLora(buffer, size)); }
  #endif
  #if WITH_4G
  if (UIO.wan_type == WAN_4G)
  { info(F("4G        : %s"), UIO.pprint4G(buffer, size)); }
  #endif
  #if WITH_IRIDIUM
  if (UIO.wan_type == WAN_IRIDIUM)
  { info(F("Iridium   : %s"), UIO.pprintIridium(buffer, size)); }
  #endif
  info(F("Frames    : %s"), UIO.pprintFrames(buffer, size));
  info(F("Log       : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  info(F("Actions   : %s"), UIO.pprintActions(buffer, size));
}

void loop()
{
  UIO.deepSleep();

  // We expect *only* Alarm 1
  if (intFlag & ~RTC_INT)
  {
    error(F("Expected *only* RTC_INT got %d"), intFlag);
    intFlag = 0;
    return;
  }

  // Clean interruption flag
  intFlag &= ~(RTC_INT);
  PWR.clearInterruptionPin();

  // Low battery level: do nothing
  cr.reset();
  cr.spawn(taskMain);
  cr.run();

  info(F("Loop done in %lu ms"), cr.millisDiff(UIO._loop_start));
}
