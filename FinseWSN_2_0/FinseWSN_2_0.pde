/*  
 */

// Put your libraries here (#include ...)
#include <xbeeDM.h>
#include <WaspUIO.h>

// Define variables
char log_file = "logfile.txt";
char tmp_file = "tmpfile.txt";
char data_file = "datafile.txt";
char unsent_file = "usentfil.txt";

#define key_access "LIBELIUM"   // in use for OTA programing
char RX_ADDRESS = "0013a20040779085"; // "0013a20040779085" Meshlium_Finse mac address
char node_ID[10];



void setup() {

	USB.ON();
	SD.ON();
	xbeeDM.ON();
	xbeeDM.checkNewProgram(); // CheckNewProgram is mandatory in every OTA program


  	// Function to initialize SD card
	UIO.initSD();

  	// Function to initialize
	UIO.initNet();

	USB.OFF();	
	SD.OFF();
	xbeeDM.OFF();
}


void loop() {

  // loop to record sensors, add data to tmp file
	if()
	{
		SD.ON();
		if(USB.available()){USB.ON();}

		// [INCLUDE CODE HERE] to read sensor and store values into tmp file

		SD.OFF();
		USB.OFF();
	}


  // loop to start xbeeDM and exchange data through network
	if()
	{
		xbeeDM.ON();
		SD.ON();
		if(USB.available()){USB.ON();}

		// [INCLUDE CODE HERE] Synchronize time


		// Transfer frames from tmp_file to data file, unsent_file if no connectoin
		UIO.Frame2Meshlium( tmp_file, data_file, unsent_file,  RX_ADDRESS,  log_file );

		

  		// [INCLUDE CODE HERE]

		xbeeDM.OFF();
		SD.OFF();
		USB.OFF();
	}
}

