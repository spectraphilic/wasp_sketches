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
 

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <inttypes.h>
#include <Stream.h>

class HardwareSerial : public Stream
{
  private:
    uint8_t _uart;
    void printNumber(unsigned long, uint8_t);
    void printFloat(double, uint8_t);
  public:
    explicit HardwareSerial(uint8_t);
    void begin(long);

    // XXX Commented functions are those defined in Arduino, implement to have
    // 100% compatibility with Arduino

    //void begin(unsigned long baud) { begin(baud, SERIAL_8N1); }
    //void begin(unsigned long, uint8_t);
    //void end();
    virtual int available();
    virtual int peek();
    virtual int read();
    //virtual int availableForWrite(void);
    virtual void flush();
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return true; }

    // XXX These come from Waspmote v010, probably should remove as not needed
    // for compatibility with Arduino
    void print(char);
    void print(const char[]);
    void print(uint8_t);
    void print(int);
    void print(unsigned int);
    void print(long);
    void print(unsigned long);
    void print(long, int);
    void print(double);
    void println();
    void println(char);
    void println(const char[]);
    void println(uint8_t);
    void println(int);
    void println(long);
    void println(unsigned long);
    void println(long, int);
    void println(double);
};

//extern HardwareSerial Serial;
extern HardwareSerial Serial1;

#endif
