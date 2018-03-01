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

char filename[]="ONEWIRE.TXT";
char* buf;

int nblines, nline;
int i=1;

void setup()
{
  // put your setup code here, to run once:
  SD.ON();
  nblines=SD.numln(filename);
  USB.println(filename);
  USB.println(nblines);
  while (i <=nblines){
    SD.catln(filename,i,1);
    buf = strtok(SD.buffer, "\n");
    
    USB.println(buf);
    i = i + 1;
    //if(i>nblines){
      //break;}
    }

}


void loop()
{
  // put your main code here, to run repeatedly:

}
