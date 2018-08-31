/*! \file SDI12.cpp
  	\brief Library for the SDI-12 protocol

    Copyright (C) 2018 Libelium Comunicaciones Distribuidas S.L.
    http://www.libelium.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

		Based on SDI12 library of Kevin M. Smith (http://ethosengineering.org)

    Version:		3.0
    Design:			David Gascón
    Implementation: Javier Siscart

 */
 #ifndef __WPROGRAM_H__
 #include <WaspClasses.h>
 #endif
 
 #include <Coroutines.h>
 #include "SDI12.h"
 
 ///////////////////////////////////////// Private methods/////////////
 
 /*	wakeSensors: performs start sequence to begin a transmission
 *	Parameters: void
 *  Return:	void
 */
 void WaspSDI12::wakeSensors()
 {
	 setState(TRANSMITTING);
	 //Send the BREAK
	 digitalWrite(_dataPin, HIGH);
	 delayMicroseconds(12500);
	 
	 //Send the MARKING
	 digitalWrite(_dataPin, LOW);
	 delayMicroseconds(9000);
 }
 
 /*	writeChar: writes a character out on the data line
 *	Parameters: byte to send
 *  Return: void
 */
 void WaspSDI12::writeChar(uint8_t out)
 {
	 // parity bit
	 out |= (parity_even_bit(out)<<7);
	 
	 digitalWrite(_dataPin, HIGH);
	 delayMicroseconds(SPACING);
	 
	 for (byte mask = 0x01; mask; mask<<=1)
	 {
		 // send payload
		 if(out & mask)
		 {
			 digitalWrite(_dataPin, LOW);
		 }
		 else
		 {
			 digitalWrite(_dataPin, HIGH);
		 }
		 delayMicroseconds(SPACING);
	 }
	 
	 // Stop bit
	 digitalWrite(_dataPin, LOW);
	 delayMicroseconds(SPACING);
 }
 
 /*	peek: reveals the next character in the buffer without consuming
 *	Parameters: void
 *  Return:	next byte available on buffer
 */
 int WaspSDI12::peek()
 {
	 // Empty buffer? If yes, -1
	 if (_rxBufferHead == _rxBufferTail) return -1;
	 // Otherwise, read from "head"
	 return _rxBuffer[_rxBufferHead];
 }
 
 /*	flush: clears the buffer contents and resets the status of the buffer overflow.
 *	Parameters: void
 *  Return:	void
 */
 void WaspSDI12::flush()
 {
	 _rxBufferHead = _rxBufferTail = 0;
	 _bufferOverflow = false;
 }
 
 
 /*	receiveChar: reads a new character and saves it into the buffer.
 *	Parameters: void
 *  Return:	void
 */
 void WaspSDI12::receiveChar()
 {
	 // Start bit?
	 if (digitalRead(_dataPin))
	 {
		 // buffer for new char
		 uint8_t newChar = 0;
		 
		 delayMicroseconds(SPACING/2);
		 
		 // read 7 data bits
		 for (uint8_t i=0x1; i<0x80; i <<= 1)
		 {
			 delayMicroseconds(SPACING);
			 uint8_t noti = ~i;
			 if (!digitalRead(_dataPin))
			 {
				 newChar |= i;
			 }
			 else
			 {
				 newChar &= noti;
			 }
		 }
		 
		 // skip parity adn stop bit
		 delayMicroseconds(SPACING); // XXX Should we check parity?
		 delayMicroseconds(SPACING);
		 
		 // Overflow? If not, proceed.
		 if ((_rxBufferTail + 1) % _BUFFER_SIZE == _rxBufferHead)
		 {
			 _bufferOverflow = true;
		 }
		 else
		 {
			 // Save char, advance tail.
			 _rxBuffer[_rxBufferTail] = newChar;
			 _rxBufferTail = (_rxBufferTail + 1) % _BUFFER_SIZE;
			 j++;
		 }
	 }
 }
 
 
 // Constructors ////////////////////////////////////////////////////////////////
 
 /*	Constructor: 	Sets the mode of the digital pins and initializes them
 */
 WaspSDI12::WaspSDI12(uint8_t dataPin)
 {
 	 _dataPin = dataPin;
	 setState(HOLDING);
	 _bufferOverflow = false;
	 
 }
 
 
 // Public Methods //////////////////////////////////////////////////////////////
 
 /*	setState: Manages data pin states
 *	Parameters: uint8_t status: state of the data line
 *  Return:	void
 */
 void WaspSDI12::setState(uint8_t status)
 {
	 switch (status)
	 {
		 case HOLDING:				pinMode(_dataPin,OUTPUT);
		 							digitalWrite(_dataPin,LOW);
		 							break;
		 
		 case TRANSMITTING: 		pinMode(_dataPin,OUTPUT);
		 							break;
		 
		 case LISTENING:			digitalWrite(_dataPin,LOW);
		 							pinMode(_dataPin,INPUT);
		 							break;
		 
		 case DISABLED: 			digitalWrite(_dataPin,LOW);
		 							pinMode(_dataPin,INPUT);
		 							break;
		 
		 default: 					break;
	 }
 }
 /*	sendCommand: sends a SDI12 command
 *	Parameters: char* cmd: command to be sent
 * 				uint8_t length: length of the command
 *  Return:	void
 */
 void WaspSDI12::sendCommand(char* cmd, uint8_t length)
 {
	 #if SDI12DEBUG
	 PRINT_SDI12(F("[TX]"));
	 USB.println(cmd);
	 #endif
	 
	 flush();
	 wakeSensors();
	 
	 for (uint8_t i = 0; i < length; i++)
	 {
		 writeChar(cmd[i]);
	 }
	 
	 setState(LISTENING);
 }
 
 /*	available: number of characters available in the buffer
 *	Parameters: void
 *  Return:	number of bytes available
 */
 int WaspSDI12::available()
 {
	 if(_bufferOverflow) return -1;
	 return (_rxBufferTail + _BUFFER_SIZE - _rxBufferHead) % _BUFFER_SIZE;
 }
 
 /*	read: reads in the next character from the buffer (and moves the index ahead)
 *	Parameters: void
 *  Return:	next character on the buffer
 */
 int WaspSDI12::read()
 {
	 //reading makes room in the buffer
	 _bufferOverflow = false;
	 // Empty buffer? If yes, -1
	 if (_rxBufferHead == _rxBufferTail) return -1;
	 // Otherwise, grab char at head
	 uint8_t nextChar = _rxBuffer[_rxBufferHead];
	 // increment head
	 _rxBufferHead = (_rxBufferHead + 1) % _BUFFER_SIZE;
	 // return the char
	 return nextChar;
 }
 
 /*	readCommandAnswer: waits till sensor answer a command.
 *	Parameters: uint8_t length: legth of expected answer
 * 				unsigned long timeout: time to be waiting for the answer (milliseconds)
 *  Return:	void
 */
 void WaspSDI12::readCommandAnswer(unsigned long timeout)
 {
 	 uint8_t state = 0;

	 j = 0;
	 unsigned long previous = millis();
	 while (millis() - previous < timeout)
	 {
		 receiveChar();

		 if (state == 0)
		 {
		 	if (peek() == '\r')
			{
				state = 1;
			}
		 }
		 else if (state == 1)
		 {
		 	if (peek() == '\n')
			{
				break;
			}
		 }
		 
		 // Condition to avoid an overflow (DO NOT REMOVE)
		 if( millis() < previous ) previous=millis();
		 
	 }
	 #if SDI12DEBUG
	 PRINT_SDI12(F("[RX]"));
	 for (int i = _rxBufferHead; i < _rxBufferTail; i++)
	 {
		 USB.print(_rxBuffer[i]);		// print without macro
	 }
	 USB.println();
	 #endif
	 
 }


/*
 * From the University of Oslo
 */

/*
 * Reads line from internal buffer with some processing: skips garbage at the
 * beginning, does not include the end-of-line.
 */
const char* WaspSDI12::readline()
{
    int c;
    char* p = buffer;
    uint8_t state = 0; // 0: start, 1: reading, 2: eol found

    while ((c = read()) != -1)
    {
        // Skip garbage at the beginning, expect address (0-9, A-Z, a-z)
        if (state == 0 && ! ('0' <= c <= 'z'))
        {
	    warn(F("sdi1-12 readline garbage skipped: %d"), c);
            continue;
        }
        state = 1;

        // Stop condition, check only for \r
        if (c == '\r')
        {
	    state = 2;
            break;
        }

        *p++ = (char) c;
    }

    *p = '\0';

    if (state != 2)
    {
        warn(F("sdi-12 readline eol not found"));
    }

    return buffer;
}

/*
 * Higher level interface: sends command, reads answer, processes and returns
 * it. Exemple: sendCommand("0M!")
 */
const char* WaspSDI12::sendCommand(const char* cmd)
{
    debug(F("sdi-12 sendCommand(%s)"), cmd);
    sendCommand((char*)cmd, strlen(cmd));
    readCommandAnswer();
    readline();
    debug(F("sdi-12 sendCommand(%s): '%s'"), cmd, buffer);
    return buffer;
}

/*
 * Sends command to address. Example: sendCommand(0, "M")
 */
const char* WaspSDI12::sendCommand(uint8_t address, const char* cmd)
{
    char aux[5];
    size_t max = sizeof(aux);
    int n;

    if (address > 9)
    {
        return NULL;
    }

    n = snprintf(aux, max, "%d%s!", address, cmd);
    if (n < 0 || n >= max)
    {
        return NULL;
    }

    sendCommand(aux);

    // Check address
    if (aux[0] != buffer[0])
    {
        return NULL;
    }

    return buffer;
}

/* Sends identity command to address. */
const char* WaspSDI12::identify(uint8_t address)
{
    return sendCommand(address, "I");
}

/* Sends measure command to address. Returns the number of seconds to wait for
 * the data to be available; or -1 if error. */
int WaspSDI12::measure(uint8_t address)
{
    if (sendCommand(address, "M"))
    {
      return -1;
    }

    return atoi(buffer+1);
}

/* Sends data command to address. Always to the buffer 0 (TODO Specify buffer
 * in parameter) */
const char* WaspSDI12::data(uint8_t address)
{
    return sendCommand(address, "D0");
}

/* Sends the query address command. Returns the address. */
char WaspSDI12::read_address()
{
    sendCommand("?!");
    return buffer[0];
}

/* Changes the address of the sensor. */
uint8_t WaspSDI12::set_address(char current_address, char new_address)
{
    char aux[5];

    snprintf(aux, sizeof(aux), "%cA%c!", current_address, new_address);
    sendCommand(aux);

    if (buffer == NULL)
    {
        return 1;
    }

    if (buffer[0] != new_address)
    {
        return 1;
    }

    return 0;
}
