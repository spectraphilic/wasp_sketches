/*  
 *  ------ [PWR_3] Setting Hibernate Mode -------- 
 *  
 *  Explanation: This example shows how to set Waspmote in the lowest 
 *  power consumption mode, disconnecting all the board but RTC, which
 *  powers from auxiliary battery.
 *  
 *  Copyright (C) 2015 Libelium Comunicaciones Distribuidas S.L. 
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
 *  
 *  Version:           0.2
 *  Design:            David Gascón 
 *  Implementation:    Marcos Yarza
 */

void setup()
{  
  // Checks if we come from a normal reset or an hibernate reset
  PWR.ifHibernate();

  USB.ON();
  USB.println(F("PWR_3 example"));


}

void loop()
{

  // If Hibernate has been captured, execute the associated function
  if( intFlag & HIB_INT )
  {
    USB.println(F("---------------------"));
    USB.println(F("Hibernate Interruption captured"));
    USB.println(RTC.getTimestamp());                    // Print Timestamp
    USB.println(F("---------------------"));
    intFlag &= ~(HIB_INT);
    delay(5000);
  }
  else if( intFlag & RTC_INT )
  {
    USB.println(F("---------------------"));
    USB.println(F("Deep sleep Interruption captured"));
    USB.println(RTC.getTimestamp());                    // Print Timestamp
    USB.println(F("---------------------"));
    intFlag &= ~(RTC_INT);
    delay(5000);
  }
  else



    /*
   * Do whatever your code needs
     *
     */

    USB.println(F("enter sleep mode"));

  // Set Waspmote to Hibernate, waking up after 10 seconds
  //PWR.hibernate("00:00:00:10",RTC_OFFSET,RTC_ALM1_MODE2);
  PWR.deepSleep("00:00:00:10",RTC_OFFSET,RTC_ALM1_MODE2,ALL_OFF);



}




