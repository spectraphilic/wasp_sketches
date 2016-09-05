/*! \def myLibrary_h
 \brief The library flag
 */
#ifndef testing
#define testing_h

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <inttypes.h>
#include <stdio.h>
#include <WaspUSB.h>

/******************************************************************************
 * Definitions & Declarations
 ******************************************************************************/
#define DEBUG_UIO 1  // 1=debug on, 0=debug off


/******************************************************************************
 * Class
 ******************************************************************************/

//! WaspUIO Class
/*!
  defines all the variables and functions used 
 */
class testing
{

  /// private methods //////////////////////////
private:


  /// public methods and attributes ////////////
public:
 /* Function to to send and store a frame on SD card 
*/

uint8_t logActivity(char* filename, char* message);

};

extern testing test;

#endif