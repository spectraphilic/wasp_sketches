

// Script to test hobbytronics
// help available at http://www.hobbytronics.co.uk/usb-host-flash-drive


int i;

// Create a printing function which has a built-in delay


void setup(){
	delay(2000);
	USB.begin(9600); 

	flash_data("$WRITE ARDUINO.TXT");   
	delay(1000);   

	flash_data("$DIR *.JPG");   
	delay(1000);   


	flash_data()
}

void loop(){
	
}

// 0. reach directory with images (path: )  $CD
// 1. read file with original pattern   $DIR *
// 2. rename file with datetime included  $REN 

//=========================================================
//					Functions
//=========================================================

void flash_data(char *pstring)
{
  Serial.println(pstring); 
  delay(50); 
}  
