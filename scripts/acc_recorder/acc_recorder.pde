/*
    ------ Waspmote Pro Code Example --------

    Explanation: This is the basic Code for Waspmote Pro

    Copyright (C) 2016 Libelium Comunicaciones Distribuidas S.L.
    http://www.libelium.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Put your libraries here (#include ...)

uint8_t status;
int x_acc;
int y_acc;
int z_acc;

char filename[]="ACC.TXT";
char sample_txt[100];
uint8_t sd_answer;


void setup()
{
  // put your setup code here, to run once:

  ACC.ON();
  SD.ON();
  RTC.ON();
  USB.ON(); // starts using the serial port
  USB.println(F("Starting ACC"));

  sd_answer = SD.create(filename);
  if( sd_answer == 1 )
  {
    USB.println(F("file created"));
  }
  else 
  {
    USB.println(F("file NOT created"));  
  }

}


void loop()
{
  // put your main code here, to run repeatedly:

  status = ACC.check();
  x_acc = ACC.getX();
  y_acc = ACC.getY();
  z_acc = ACC.getZ();
  
  memset(sample_txt, 0, sizeof(sample_txt));
  snprintf(sample_txt, sizeof(sample_txt), "20%02u-%02u-%02u %02u:%02u:%02u, %d, %d, %d", RTC.year, RTC.month, RTC.date, RTC.hour, RTC.minute, RTC.second, x_acc, y_acc, z_acc);

  sd_answer = SD.appendln(filename, sample_txt);

  USB.println("time, x_acc, y_acc, z_acc"); 
  USB.println(sample_txt);

  delay(100);

}
