/*
 *  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.

 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "wiring.h"

#include "HardwareSerial.h"

// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(uint8_t uart)
{
  if(uart == 0){
    _uart = 0;
  }else{
    _uart = 1;
  }
}

// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(long speed)
{
  beginSerial(speed, _uart);
}

int HardwareSerial::available()
{
  return serialAvailable(_uart);
}

int HardwareSerial::read()
{
  return serialRead(_uart);
}

int HardwareSerial::peek()
{
  return serialPeek(_uart);
}

void HardwareSerial::flush()
{
  serialFlush(_uart);
}

size_t HardwareSerial::write(uint8_t c)
{
  printByte(c, _uart);
  return 1;
}

void HardwareSerial::print(char c)
{
  printByte(c, _uart);
}

void HardwareSerial::print(const char c[])
{
  printString(c, _uart);
}

void HardwareSerial::print(uint8_t b)
{
  printByte(b, _uart);
}

void HardwareSerial::print(int n)
{
  print((long) n);
}

void HardwareSerial::print(unsigned int n)
{
  print((unsigned long) n);
}

void HardwareSerial::print(long n)
{
  if (n < 0) {
    print('-');
    n = -n;
  }
  printNumber(n, 10);
}

void HardwareSerial::print(unsigned long n)
{
  printNumber(n, 10);
}

void HardwareSerial::print(long n, int base)
{
  if (base == 0)
    print((char) n);
  else if (base == 10)
    print(n);
  else
    printNumber(n, base);
}

void HardwareSerial::print(double n)
{
  printFloat(n, 2);
}

void HardwareSerial::println()
{
  print('\r');
  print('\n');
}

void HardwareSerial::println(char c)
{
  print(c);
  println();
}

void HardwareSerial::println(const char c[])
{
  print(c);
  println();
}

void HardwareSerial::println(uint8_t b)
{
  print(b);
  println();
}

void HardwareSerial::println(int n)
{
  print(n);
  println();
}

void HardwareSerial::println(long n)
{
  print(n);
  println();
}

void HardwareSerial::println(unsigned long n)
{
  print(n);
  println();
}

void HardwareSerial::println(long n, int base)
{
  print(n, base);
  println();
}

void HardwareSerial::println(double n)
{
  print(n);
  println();
}

// Private Methods /////////////////////////////////////////////////////////////

void HardwareSerial::printNumber(unsigned long n, uint8_t base)
{
  printIntegerInBase(n, base, _uart);
}

void HardwareSerial::printFloat(double number, uint8_t digits)
{
  // Handle negative numbers
  if (number < 0.0)
  {
     print('-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i)
    rounding /= 10.0;

  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    print(".");

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    print(toPrint);
    remainder -= toPrint;
  }
}

// Preinstantiate Objects //////////////////////////////////////////////////////

//HardwareSerial Serial = HardwareSerial(0);
HardwareSerial Serial1 = HardwareSerial(1);

