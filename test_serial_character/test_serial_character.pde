/*  
 *  ------ Waspmote Pro Code Example -------- 
 *  
 *  Explanation: This is the basic Code for Waspmote Pro
 *  
 *  Copyright (C) 2013 Libelium Comunicaciones Distribuidas S.L. 
 *  http://www.libelium.com 
 *  
 *  This program is free software: you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by  
 *  the Free Software Foundation, either version 3 of the License, or  
 *  (at your option) any later version.  
 *   
 *  This program is distributed in the hope that it will be useful,  
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of  
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
 *  GNU General Public License for more details.  
 *   
 *  You should have received a copy of the GNU General Public License  
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 */
 
#include <WString.h>
     
// Put your libraries here (#include ...)
int val = 0;
unsigned long time = 0;
char te[100];


void setup() {
    // put your setup code here, to run once:
USB.ON();
USB.println("wasp ready");
USB.println("-------------------------------");
}


void loop() {
    // put your main code here, to run repeatedly:

 char message[100];
    while(!USB.available())
    {
       //wait for available
    }
    strcpy( message, "" );
    while (USB.available() > 0)
    {
        val = USB.read();
        //USB.print(val,BYTE);
        snprintf(message, sizeof(message),"%s%c", message,val);
     }
     USB.println();

     USB.println(message);
//  if (USB.available() > 0)
//        {
//            val = serialRead();
//            char l = (char)val;
//            String h = String(l) + String(l);
//            USB.println(val,BYTE);
//            printString(h);
//            USB.println("-------------------------------");
//            
//            tmp = Byte2String(val);
//            
//            //tmp =  string.concat(tmp, h);
//            
//        }   
        
     //String tmp;
//     while(val != -1){
//       val = USB.read();
//       if(val != -1){
//         char l = (char)val;
//         String tmp = tmp + String(l);
//       }
//     }   
}




char Byte2char(char val){
  // Function to convert BYTE to char
  char l = (char)val;
  return(l);
}

String Byte2String(char val){
  // Function to convert BYTE to String object
  char l = (char)val;
  String mystring = String(l);
  return(mystring);
}

const char* String2char(String mystring){
  // Function to convert String object for printing on USB serial
  return((const char*)&mystring[0]);
}

void printString(String mystring){
  // Function to print String object to serial
  USB.println((const char*)&mystring[0]);
}
