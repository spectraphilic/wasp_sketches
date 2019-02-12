#include "WaspUIO.h"


void WaspUIO::networkInit()
{
#if WITH_XBEE
  if (networkType == NETWORK_XBEE) { xbeeInit(); }
#endif

#if WITH_4G
  if (networkType == NETWORK_4G) { _4GInit(); }
#endif

#if WITH_IRIDIUM
  if (networkType == NETWORK_IRIDIUM) { iridiumInit(); }
#endif
}
