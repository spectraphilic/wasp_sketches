/******************************************************************************
 * Includes
 ******************************************************************************/

#ifndef __WPROGRAM_H__
#include <WaspClasses.h>
#endif

#include "testing.h"
#include <stdio.h>
#include <WaspXBeeDM.h>
#include <WaspUSB.h>


/******************************************************************************
* Constructors 
******************************************************************************/
//UIO::UIO()
//{
		
//}

/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/
 
 uint8_t testing::logActivity( char filename[], char message[] )
{
  int answer = 0;

  // Make sure a file called filename exists before writing to SD
  if(SD.isFile(filename) == 1)
  {
    SD.append(filename, RTC.getTime());
    SD.append(filename, "\t");
    answer = SD.appendln(filename, message);
    delay(100);
  }
  return answer;
}
