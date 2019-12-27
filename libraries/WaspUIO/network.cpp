#include "WaspUIO.h"


void WaspUIO::networkInit()
{
#if WITH_XBEE
  if (lan_type == LAN_XBEE) { xbeeInit(); }
#endif

#if WITH_LORA
  if (lan_type == LAN_LORA) { loraInit(); }
#endif

#if WITH_4G
  if (wan_type == WAN_4G) { _4GInit(); }
#endif

#if WITH_IRIDIUM
  if (wan_type == WAN_IRIDIUM) { iridiumInit(); }
#endif
}
