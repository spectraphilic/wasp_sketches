/* ======================== Arduino SDI-12 =================================

Arduino library for SDI-12 communications to a wide variety of environmental
sensors. This library provides a general software solution, without requiring
any additional hardware.

======================== Attribution & License =============================

Copyright (C) 2013  Stroud Water Research Center
Available at https://github.com/StroudCenter/Arduino-SDI-12

Authored initially in August 2013 by:

        Kevin M. Smith (http://ethosengineering.org)
        Inquiries: SDI12@ethosengineering.org

based on the SoftwareSerial library (formerly NewSoftSerial), authored by:
        ladyada (http://ladyada.net)
        Mikal Hart (http://www.arduiniana.org)
        Paul Stoffregen (http://www.pjrc.com)
        Garrett Mace (http://www.macetech.com)
        Brett Hagman (http://www.roguerobotics.com/)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SDI12_h
#define SDI12_h

                //  Import Required Libraries
#include <avr/interrupt.h>      // interrupt handling
#include <util/parity.h>         // optimized parity bit handling
#include <inttypes.h>      // integer types library
#include <WaspClasses.h>            // Waspmote core library
#include <Coroutines.h> // coroutine for millisdiff fucntions
//=======

/*  Notes on interrupts:
  The following pins are interrupt capable for the Atmel ATmega640/1280 family:
    PB0-7 = PCINT0-PCINT7
    PE0   = PCINT8             *This is USB RX on Waspmote, so not using it
    PJ0-6 = PCINT9-PCINT15     *Not available on 1281
    PK0-7 = PCINT16-PCINT23    *Not available on 1281

  The Waspmote uses a 1281 uProcessor and utilizes all its interrupt pins
  for other things. Without the ability to generate a pin change interrupt
  we have to modify this library to poll for a response.
*/

class SDI12
{
private:
  static SDI12 *_activeObject;  // static pointer to active SDI12 instance
  void setState(uint8_t state); // sets the state of the SDI12 objects
  void wakeSensors();      // used to wake up the SDI12 bus
  void writeChar(uint8_t out);   // used to send a char out on the data line
  int receiveChar();      // used by the ISR to grab a char from data line /*  Modified for Waspmote: return value indicates whether char was seen  */

  static const char * getStateName(uint8_t state);     // get state name (in ASCII)

public:
  char sdi12_buffer[75];
  SDI12(uint8_t dataPin);    // constructor
  ~SDI12();            // destructor
  void begin();          // enable SDI-12 object
  void end();          // disable SDI-12 object

  void forceHold();       // sets line state to HOLDING
  void sendCommand(const char* cmd);   // sends the string 'cmd' out on the data line
  void sendResponse(const char* resp); // sends the string 'resp' out on the data line (JH)

  int listen(unsigned long listenTimeout); // returns 0 if chars received   /* Added for Waspmote: polls for characters */

  int available();      // returns the number of bytes available in buffer
  int peek();        // reveals next byte in buffer without consuming
  int read();        // returns next byte in the buffer (consumes)
  void flush();        // clears the buffer

  bool setActive();     // set this instance as the active SDI-12 instance
  bool isActive();      // check if this instance is active

  static inline void handleInterrupt(); // intermediary used by the ISR

  // From University of Oslo
  char buffer[75];

  uint8_t readline();
  uint8_t command2address(uint8_t address, const char* cmd);
  uint8_t identification(uint8_t address);
  int measure(uint8_t address);
  uint8_t data(uint8_t address);

  char read_address();
  uint8_t set_address(char current_address, char new_address);

};

#endif
