

// Script to test hobbytronics
// help available at http://www.hobbytronics.co.uk/usb-host-flash-drive


int i;
const int ss = DIGITAL8;
const int flashCON = DIGITAL7;



// Create a printing function which has a built-in delay
#define USB_RATE 9600

// Create a printing function which has a built-in delay
void flash_data(char *pstring)
{
  USB.println(pstring); 
  delay(50); 
}  


void setup() 
{ 
  PWR.setSensorPower(SENS_5V,SENS_ON);
  //delay(2000);
  pinMode(flashCON, INPUT);
  USB.println(digitalRead(flashCON));


} 

void loop(){}






























// void setup(){



// 	delay(2000);

// 	flash_data("$CD DCIM");
// 	delay(100);

// 	flash_data("$CD 100MSDCF");
// 	delay(100);

// 	flash_data("$DIR *.JPG"); 
// 	while(!mySerial.available());        // Wait for data to be returned  
// 	delay(1000);   

// }

// void loop(){
	
// }

// // 0. reach directory with images (path: )  $CD
// // 1. read file with original pattern   $DIR *
// // 2. rename file with datetime included  $REN 

// //=========================================================
// //					Functions
// //=========================================================

// void flash_data(char *pstring)
// {
//   USB.println(pstring); 
//   delay(50); 
// }  

