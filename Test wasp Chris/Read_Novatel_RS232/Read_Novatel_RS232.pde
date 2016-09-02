/*   
 *  ------ Raed Novatel GPS data with RS232 -------- 
 * John Hulth, 2016-07-04
 * john.hulth@geo.uio.no 
 */

//Include always this library when you are using the Wasp232 functions  
#include <Wasp232.h>

uint8_t error;

void setup() {

  // Power on the USB for viewing data in the serial monitor.  
  // Note : if you are using the socket 0 for communication, 
  // for viewing data in the serial monitor, you should open
  // the USB at the same speed. 
  USB.ON();
  delay(100);


  // Powers on the module and assigns the UART in socket0 (Xbee socket = 0)
  W232.ON(SOCKET0);

  // Configure 
  W232.baudRateConfig(19200); //baud rate
  W232.parityBit(NONE);  //No parity bit
  W232.stopBitConfig(1);  //One stop bit

  // Print hello message
  USB.println("Reading Novatel GPS data with RS232");

  W232.send ("UNLOGALL COM1_ALL \n"); //reset all logs
  W232.send ("LOG COM1 rangecmpb ontime 5 0 hold \n"); //log #rangecmpb
  W232.send ("LOG COM1 rawephemb onnew hold \n"); //log rawephemb


}



void loop() 
{  

  //  /*
  -----Read and print all data recived-----
    if(W232.receive() > 0)
  { 
    USB.println(W232._buffer, W232._length);
  }

  delay(100);
  //--------------------------
  //   */


  /*
  // ----Read only GPS messages----
   
   W232.flush();
   
   while (!W232.available() || W232.receive() != '#');
   
   if(W232.receive() > 0)
   { 
   USB.println(W232._buffer, W232._length);
   }
   
   delay(100);
   */



}







