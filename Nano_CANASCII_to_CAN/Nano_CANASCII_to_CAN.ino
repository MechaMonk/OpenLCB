/*
 * Nano_CANASCII_to_CAN.ino
 * 
 * Copyright 2017 Otto Schreibke <oschreibke@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * The license can be found at https://www.gnu.org/licenses/gpl-3.0.txt
 * 
 * 
 */


//
// Bridge CAN-ASCII (produced by JMRI-OpenLCB) from a serial input (from ESP8266) to a physical CAN adapter
//
/*
   From: https://gridconnect.com/media/documentation/grid_connect/CAN_USB232_UM.pdf

   Normal CAN Message Syntax

   : <S | X > <IDENTIFIER> <N> <Data0> <Data1> ... <Data7> ;

   RTR CAN Message Syntax

   : <S | X > <IDENTIFIER> <R> <Length> ;


*/
/* Example CAN-ASCII Messagesfrom the JMRI OpenLCB tool

  Bytes Available: 12
  :X170002B7N;
  Bytes Available: 60
  :X160002B7N;:X150002B7N;:X140002B7N;:X107002B7N;:X194902B7N;
  Bytes Available: 18
  :X00000123N120456;
  Bytes Available: 18
  :X00000123N120456;
  Bytes Available: 18
  :X00000123N120456;
  Bytes Available: 26
  :X1A2B72B7N20610000000008;
  Bytes Available: 16
  :X199702B7N02B7;
  Bytes Available: 20
  :X00000123N12345678;
*/

// handling of the CAN adapter format is performed by the MCP2515 library (mcp_can)

// https://github.com/coryjfowler/MCP_CAN_lib.git
#include <mcp_can.h>

#include <SPI.h>

#include "CANCommon.h"
#include "CanAscii2Can.h"
#include "Can2CanAscii.h"

//#define SHOWLCD
#define SHOWSERIAL

// Arduino SPI Pins
//
// CLK  D13
// MISO D12
// MOSI D11
// CS   D10
//
// CAN Interrupt D2
//

#define CANINTPIN   2
//#define GPIO12 12
//#define GPIO13 13
//#define GPIO14 14
//#define GPIO15 15

#ifdef SHOWSERIAL
#include <SoftwareSerial.h>

SoftwareSerial mySerial(5, 4); // RX, TX
#endif

#ifdef SHOWLCD
#include <LiquidCrystal_I2C.h>
/*
  Pins: SDA   A4
        SCK   A5
*/

#define I2C_ADDR    0x3f
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7


LiquidCrystal_I2C  lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
#endif

bool initialised = false;

MCP_CAN CAN(10); // Set CS to pin 10

//volatile boolean can_msg_received = false;

// send and receive buffers for the can packets
// CAN => received from the CAN interface; WiFi => received over WiFi
uint32_t CANIdentifier = 0;
uint8_t CANDataLen = 0;
uint8_t CANData[8]; // = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

char CANAscii[CANASCII_SIZE]; // array to store the ASCII-encoded message
CAN_message_type  WiFiMessageType = Standard;
uint32_t WiFiIdentifier = 0;
uint8_t WiFiDataLen = 0;
uint8_t WiFiData[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

char msgString[128];                        // Array to store serial string


//void MCP2515_ISR()
//{
//  can_msg_received = true;
//}

//void CAN_Send(unsigned char cantxValue1, unsigned char cantxValue2, unsigned char cantxValue3, unsigned char cantxValue4, unsigned char cantxValue5, unsigned char cantxValue6, unsigned char cantxValue7, unsigned char cantxValue8) {
//  unsigned char canMsg[8] = {cantxValue1, cantxValue2, cantxValue3, cantxValue4, cantxValue5, cantxValue6, cantxValue7, cantxValue8};
//  CAN.sendMsgBuf(I2C_ADDR, 0, 8, canMsg);
//}



void setup() {
  Serial.begin(115200);
#ifdef SHOWSERIAL
  mySerial.begin(115200);
  mySerial.println("Starting...");
#endif  

#ifdef SHOWLCD
  lcd.begin (16, 2); //  <<----- My LCD was 16x2
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.clear();
#endif

  pinMode(CANINTPIN, INPUT); 
  
  //start UART and the server
  //Serial.begin(115200);

  initialised = (CAN.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ) == CAN_OK);
  if (initialised) {
#ifdef SHOWLCD
    lcd.print("can init ok");
#endif
#ifdef SHOWSERIAL
    mySerial.println("can init ok");
#endif
    CAN.setMode(MCP_NORMAL);

    //attachInterrupt(CANINTPIN, MCP2515_ISR, FALLING); // start interrupt
    
//    Serial.println("can init ok");
  }
#ifdef SHOWLCD
  else
    lcd.print("Can init fail");
#endif
#ifdef SHOWSERIAL
  else
    mySerial.println("can init fail");
#endif
//for (int x = 0; x < 16; x++){
//  Serial.print(hexDigits[x]); Serial.print(": "); Serial.println(Hex2Int(hexDigits[x])); 
//}
}

void loop() {
	char buf[50];

  // Send incoming message from CAN to WiFi (if available)
  if (!digitalRead(CANINTPIN)) {
//  if (can_msg_received) {
#ifdef SHOWSERIAL
	mySerial.println("CAN Message received.");
#endif	  
    //can_msg_received = false;

    CAN.readMsgBuf(&CANIdentifier, &CANDataLen, CANData);      // Read data: len = data length, buf = data byte(s)

#ifdef SHOWSERIAL
    mySerial.print("CANIdentifier- "); mySerial.println(CANIdentifier, HEX);
    mySerial.print("CANDataLen- "); mySerial.println(CANDataLen, HEX);
    if (CANDataLen > 0){
        mySerial.print("CANData- "); 
        for (uint8_t i = 0; i < CANDataLen; i++){
            if (CANData[i] < 0x10){
		        mySerial.print(F("0"));
	    	}
	        mySerial.print(CANData[i], HEX);	
		}
        mySerial.println("");
    }
#endif       
    
    Can2CanAscii(&CANIdentifier, &CANDataLen, CANData, CANAscii);

    Serial.println((String)CANAscii);               // Transmit the CAN-Ascii message to the client
      
#ifdef SHOWLCD
      lcd.clear();
      lcd.print("Sending to WiFi client- ");
      lcd.println((String)CANAscii);
#endif
#ifdef SHOWSERIAL
      mySerial.print("Sending to WiFi client: ");
      mySerial.println((String)CANAscii);
      mySerial.println();
#endif
    }


  // send incoming message from WiFi to CAN  (if available)
  if (Serial.available()) {
#ifdef SHOWSERIAL
	mySerial.println("WiFi Message received.");
#endif
		  
    //get data from the telnet client
    while (Serial.available()) {
      char ch = Serial.read();

#ifdef SHOWLCD
      lcd.print(ch); // push it to the UART (=> Serial Monitor)
#endif
#ifdef SHOWSERIAL
      mySerial.print(ch); // push it to the UART (=> Serial Monitor)
#endif

    
     if (CanAscii2Can(&WiFiIdentifier, &WiFiMessageType, &WiFiDataLen, WiFiData, &ch)){
#ifdef SHOWSERIAL
        mySerial.println();
        mySerial.print("WiFiIdentifier- "); mySerial.println(WiFiIdentifier, HEX);
	    mySerial.print("WiFiDataLen- "); mySerial.println(WiFiDataLen, HEX);
		  if (WiFiDataLen > 0){
			  mySerial.print("WiFiData- "); 
			  for (uint8_t i = 0; i < WiFiDataLen; i++){
				  if (WiFiData[i] < 0x10){
					  mySerial.print(F("0"));
				  }
				  mySerial.print(WiFiData[i], HEX);	
		     }
            mySerial.println("");
         }				
#endif		  
		  
            byte sndStat = CAN.sendMsgBuf(WiFiIdentifier, (int) WiFiMessageType, WiFiDataLen, WiFiData);
#ifdef SHOWLCD
            if (sndStat == CAN_OK) {
              lcd.println("Message Sent Successfully!");
            } else {
              lcd.println("Error Sending Message...");
            }
#endif
#ifdef SHOWSERIAL
            if (sndStat == CAN_OK) {
              mySerial.println("CAN Message Sent Successfully!");
            } else {
              mySerial.println("Error Sending CAN Message...");
            }
#endif            
         }   		  
	  }
    }
}


