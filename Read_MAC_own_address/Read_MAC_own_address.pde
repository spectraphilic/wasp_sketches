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

#include <WaspXBeeDM.h>

char macHigh[10];
char macLow[10];

void setup() {
    // put your setup code here, to run once:
  USB.ON();
  USB.println("Script to read own Mac address...");
  xbeeDM.ON(); 
}


void loop() {
    // put your main code here, to run repeatedly:
  xbeeDM.getOwnMac();
  
  
  
  // convert mac address from array to string
  Utils.hex2str(xbeeDM.sourceMacHigh, macHigh);
  Utils.hex2str(xbeeDM.sourceMacLow, macLow);


  /////////////////////////////////
  // 5. Print XBee module information
  /////////////////////////////////
  USB.print("My mac address is:");
  USB.print(macHigh);
  USB.println(macLow);
  USB.println("==================");
  delay(5000);
}

