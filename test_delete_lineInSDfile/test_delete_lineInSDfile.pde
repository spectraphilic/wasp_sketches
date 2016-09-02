/*  
 *  ------ [SD_09] - Index Of a pattern inside SD file -------- 
 *  
 *  Explanation: Turn on the SD card. Write a file with some lines.
 *  Then show how to find patterns inside the file.
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
 *  Version:           0.3
 *  Design:            David Gascón 
 *  Implementation:    Yuri Carmona
 */

// define file name to be created: MUST be 8.3 SHORT FILE NAME
char file[]="FILE9.TXT";

uint8_t sd_answer;
int32_t numLines;

void setup()
{
  // open USB port
  USB.ON();
  USB.println(F("SD_09 example"));

  // Set SD ON
  SD.ON();

  // delete file
  sd_answer = SD.del(file);

  // create file
  sd_answer = SD.create(file);

  // Append data to file
  if(SD.appendln(file,"1abcdefghijklmnopqrstuvwxyz0123456789")) USB.println("append ok");
  if(SD.appendln(file,"2abcdefghijklmnopqrstuvwxyz0123456789")) USB.println("append ok");
  if(SD.appendln(file,"3abcdefghijklmnopqrstuvwxyz0123456789")) USB.println("append ok");
  if(SD.appendln(file,"4abcdefghijklmnopqrstuvwxyz0123456789")) USB.println("append ok");
  if(SD.appendln(file,"5abcdefghijklmnopqrstuvwxyz0123456789")) USB.println("append ok");

  USB.println(F("-------------------"));
  USB.print(F("Show file:"));
  SD.showFile(file);
  delay(1000);
  // get number of lines in file
  USB.print("Number of lines in SD: ");
  numLines = SD.numln(file);
  USB.print(numLines);

}


void loop()
{
  while(numLines>1)
  {
    USB.println(SD.catln( file, numLines-1,1)); 

    USB.println(SD.buffer);
    //USB.println(sizeof(SD.buffer));


    delay(500);
  }
}







