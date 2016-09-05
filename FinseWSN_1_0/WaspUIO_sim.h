/*! \def myLibrary_h
 \brief The library flag
 */
#ifndef WaspUIO_sim_h
#define WaspUIO_sim_h

/******************************************************************************
 * Includes
 ******************************************************************************/

#include <inttypes.h>
#include <WaspFrame.h>
#include <stdio.h>
#include <WaspXBeeDM.h>
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
class WaspUIO_sim
{

  /// private methods //////////////////////////
private:


  /// public methods and attributes ////////////
public:
 /* Function to to send and store a frame on SD card 
*/

void initSD(void);
void initNet(int network);



uint8_t check_for_connection_to_Meshlium(char logfile);
uint8_t synchronize_time_from_meshlium(int RX_ADDRESS, char logfile);
uint8_t check_for_OTA_new_program(char logfile);
uint8_t logActivity(char* filename, char* message);
uint8_t Frame2SdFile(char* filename);
uint8_t Frame2Meshlium(char* filename_tmp,char* filename_final, char* filename_unsent, int32_t* RX_ADDRESS, char* logfile)


};

extern WaspUIO_sim UIO;

#endif