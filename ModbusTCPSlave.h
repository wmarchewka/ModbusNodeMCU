/*
    ModbusTCPSlave.h - an Arduino library for a Modbus TCP slave.

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

// Note: The Arduino IDE does not respect conditional included
// header files in the main sketch so you have to select your
// here.
//
#ifndef ModbusTCPSlave_h
#define ModbusTCPSlave_h

#define strDate __DATE__
#define strTime __TIME__
 

#define MB_PORT 502  

#define MB_ESP8266

#define MBDebug     //serial debug enable

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "flasher.h"
#include <EEPROM.h>



#define maxInputRegister    20
#define maxHoldingRegister    20
#define maxCoilRegister    20



//
// MODBUS Function Codes
//
#define MB_FC_NONE 0
#define MB_FC_READ_COILS 1
#define MB_FC_READ_DISCRETE_INPUT 2
#define MB_FC_READ_REGISTERS 3              //implemented
#define MB_FC_READ_INPUT_REGISTERS 4        //implemented
#define MB_FC_WRITE_COIL 5
#define MB_FC_WRITE_REGISTER 6              //implemented
#define MB_FC_WRITE_MULTIPLE_COILS 15
#define MB_FC_WRITE_MULTIPLE_REGISTERS 16   //implemented
//
// MODBUS Error Codes
//
#define MB_EC_NONE 0
#define MB_EC_ILLEGAL_FUNCTION 1
#define MB_EC_ILLEGAL_DATA_ADDRESS 2
#define MB_EC_ILLEGAL_DATA_VALUE 3
#define MB_EC_SLAVE_DEVICE_FAILURE 4
//
// MODBUS MBAP offsets
//
#define MB_TCP_TID          0
#define MB_TCP_PID          2
#define MB_TCP_LEN          4
#define MB_TCP_UID          6
#define MB_TCP_FUNC         7
#define MB_TCP_REGISTER_START         8
#define MB_TCP_REGISTER_NUMBER         10

class ModbusTCPSlave
{
public:
  ModbusTCPSlave();
    void begin(const char *ssid, const char *key);
    void begin(const char *ssid, const char *key,uint8_t ip[4],uint8_t gateway[4],uint8_t subnet[4]);
    
    void Run();
    unsigned int  MBInputRegister[maxInputRegister];
    unsigned int  MBHoldingRegister[maxHoldingRegister];
    unsigned int  MBCoilRegister[maxCoilRegister];
    int AwaitConnectCounter;
    int AwaitConnectTime;
    int timer2;
    int rssi;
  

    int calcRSSI(int rssiVal){
        if (rssiVal <= -100)
    quality = 0;
    else if (rssiVal >= -50)
    quality = 100;
    else
    rssiVal = rssiVal + 100;
    quality = (rssiVal * 2);

    return quality;
    }
    
private: 
    byte ByteArray[260];
    bool ledPinStatus = LOW;
    WiFiServer MBServer;
      int quality;


 };
#endif
