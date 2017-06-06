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

// Put your libraries here (#include ...)
#include <WaspUIO.h>
#include <WaspXBeeDM.h>
#include <WaspFrame.h>

// Define variables
//#define key_access "LIBELIUM"   // in use for OTA programing
//char RX_ADDRESS = "0013a20040779085"; // "0013a20040779085" Meshlium_Finse mac address
//char node_ID[10];
void setup() {
  // put your setup code here, to run once:

  UIO.initSD(); // Function OK!

  UIO.initNet(NETWORK_FINSE); // Function OK!

  UIO.logActivity("testing logActivity function"); // UIO.logActivity and UIO.Frame2Sd functions OK!

  USB.println(UIO.RX_ADDRESS);
 // USB.println(UIO.BROADCAST_ADDRESS, HEX);

  UIO.readMaxbotixSerial(6); // Function OK, but need some work!
}
void loop() {
  // put your main code here, to run repeatedly:
}



