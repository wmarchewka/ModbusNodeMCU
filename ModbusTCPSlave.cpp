#include "ModbusTCPSlave.h"

 
Flasher led1(16,250,250);
Flasher led2(16,1000,1000);
Flasher led4(16,100,100);

//********************************************************************************************
ModbusTCPSlave::ModbusTCPSlave():

  MBServer(MB_PORT)

{
 
}
//********************************************************************************************
void ModbusTCPSlave::begin(const char *ssid, const char *key, uint8_t ip[4], uint8_t gateway[4], uint8_t subnet[4]) {
 
#ifdef MBDebug
  // Connect to WiFi network
  Serial.println("Starting LIVE mode");
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif

  WiFi.config(IPAddress(ip), IPAddress(gateway), IPAddress(subnet));
  WiFi.begin(ssid, key);
  
  while (WiFi.status() != WL_CONNECTED) {
  
#ifdef MBDebug
    Serial.print(".");
#endif
  }
#ifdef MBDebug
  Serial.println("");
  Serial.println("WiFi connected");
#endif

#ifdef MBDebug
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif

  // Start the server
  MBServer.begin();
  rssi=ModbusTCPSlave::calcRSSI(WiFi.RSSI());
  Serial.print("RSSI ");
  Serial.println(rssi);
    
#ifdef MBDebug
  Serial.print(F("MODBUS TCP Listening on port "));
  Serial.println(MB_PORT);
#endif
}

//********************************************************************************************
void ModbusTCPSlave::Run() {
  boolean flagClientConnected = 0;
  boolean flagConnectionEstablished = 0;

  byte byteFN = MB_FC_NONE;
  int Start;
  int WordDataLength;
  int ByteDataLength;
  int MessageLength;
  int coilValue = 0;
  int timer1 =  0;
  int ConnectedTime = 0;
  int timer3 =  0;
  


  //****************** Read from socket ****************
  WiFiClient client = MBServer.available();

 led1.Update();

#ifdef MBDebug
     if (millis() - timer2 >= 1000) {
    timer2 = millis();
    AwaitConnectTime++;
    Serial.print("Awaiting connection for ");
    Serial.print(AwaitConnectTime);
    Serial.println(" seconds");
    }    
#endif

    while (client.connected()) {

led2.Update(); 

flagClientConnected = 1;   

#ifdef MBDebug
    if (flagConnectionEstablished) {
    if (millis() - timer1 >= 1000) {
    timer1 = millis();
    AwaitConnectTime=0;
    ++ConnectedTime;
    Serial.print("Connected for ");
    Serial.print(ConnectedTime);
    Serial.println(" seconds");
    }
    }
#endif
        
    if ( (flagConnectionEstablished == 0) && (flagClientConnected == 1) ) {
      #ifdef MBDebug
      Serial.println("\r\n");
      Serial.println("Client connected...");
      #endif
      flagConnectionEstablished = 1;
    }

    if (client.available())
    {
      //flagClientConnected = 1;
      int i = 0;
      while (client.available())
      {
        ByteArray[i] = client.read();
        led4.Update();
        i++;
      }
      client.flush();
      byteFN = ByteArray[MB_TCP_FUNC];
      Start = word(ByteArray[MB_TCP_REGISTER_START], ByteArray[MB_TCP_REGISTER_START + 1]);
      WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER], ByteArray[MB_TCP_REGISTER_NUMBER + 1]);
#ifdef MBDebug
      Serial.println();
      Serial.print("RX: ");
      for (byte thisByte = 0; thisByte < 20; thisByte++) {
        Serial.print(ByteArray[thisByte], DEC);
        Serial.print("-");
      }
      Serial.println();
      Serial.print("Received Function: ");
      Serial.println(byteFN);
#endif
    }

    // Handle request
    switch (byteFN) {
      case MB_FC_NONE:
      MBHoldingRegister[0] = (unsigned int) ESP.getVcc();
      MBHoldingRegister[1] = 55;    //RSSIvalue.checkRSSIval(); 
      MBHoldingRegister[2] = ESP.getChipId();
      delay(20);         //yield to OS for wifi
  
      break;

      case MB_FC_READ_REGISTERS:  // 03 Read Holding Registers
        ByteDataLength = WordDataLength * 2;
        ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
        ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).
        for (int i = 0; i < WordDataLength; i++)
        {
          ByteArray[ 9 + i * 2] = highByte(MBHoldingRegister[Start + i]);
          ByteArray[10 + i * 2] =  lowByte(MBHoldingRegister[Start + i]);
        }
        MessageLength = ByteDataLength + 9;
        client.write((const uint8_t *)ByteArray, MessageLength);
#ifdef MBDebug
        Serial.print("TX: ");
        for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
          Serial.print(ByteArray[thisByte], DEC);
          Serial.print("-");
        }
        Serial.println();
#endif
        byteFN = MB_FC_NONE;
        break;

      case MB_FC_READ_INPUT_REGISTERS:  // 04 Read Input Registers
        Start = word(ByteArray[MB_TCP_REGISTER_START], ByteArray[MB_TCP_REGISTER_START + 1]);
        WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER], ByteArray[MB_TCP_REGISTER_NUMBER + 1]);
        ByteDataLength = WordDataLength * 2;
        ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
        ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).
        for (int i = 0; i < WordDataLength; i++)
        {
          ByteArray[ 9 + i * 2] = highByte(MBInputRegister[Start + i]);
          ByteArray[10 + i * 2] =  lowByte(MBInputRegister[Start + i]);
        }
        MessageLength = ByteDataLength + 9;
        client.write((const uint8_t *)ByteArray, MessageLength);
#ifdef MBDebug
        Serial.print("TX: ");
        for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
          Serial.print(ByteArray[thisByte], DEC);
          Serial.print("-");
        }
        Serial.println();
#endif
        byteFN = MB_FC_NONE;
        break;

      case MB_FC_WRITE_COIL:  // 05 Write Coil
        MBCoilRegister[Start] = word(ByteArray[MB_TCP_REGISTER_NUMBER], ByteArray[MB_TCP_REGISTER_NUMBER + 1]);
        ByteArray[5] = 5; //Number of bytes after this one.
        MessageLength = 12;
        client.write((const uint8_t *)ByteArray, MessageLength);
        coilValue = MBCoilRegister[Start];

        //if command 6 is received causes reboot into OTA mode
        if (Start==6 & coilValue == 65280) {
          Serial.println("Restarting in OTA mode");
          EEPROM.write(0,1);
          EEPROM.commit();
          client.stop();
          ESP.restart();
        }
        
        //if command 6 is received causes reboot into OTA mode
        if  (Start==6 & coilValue == 0) {
          Serial.println("Restarting in RUN mode");
          EEPROM.write(0,0);
          EEPROM.commit();
          ESP.restart();
        }

        if (coilValue == 65280 & Start!=6)  {
          pinMode(Start, OUTPUT);
          digitalWrite(Start, HIGH);
        }
        if (coilValue == 0 & Start !=6) {
            pinMode(Start, OUTPUT);
          digitalWrite(Start, LOW);
         }

#ifdef MBDebug
        Serial.print("TX: ");
        for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
          Serial.print(ByteArray[thisByte], DEC);
          Serial.print("-");
        }
        Serial.println();
        Serial.print("Write Single Coil: ");
        Serial.print(Start);
        if (coilValue == 65280)  {
          Serial.println("--ON");
        }
        if (coilValue == 0) {
          Serial.println("--OFF");
        }
 #endif
        byteFN = MB_FC_NONE;
        break;

      case MB_FC_WRITE_REGISTER:  // 06 Write Holding Register
        MBHoldingRegister[Start] = word(ByteArray[MB_TCP_REGISTER_NUMBER], ByteArray[MB_TCP_REGISTER_NUMBER + 1]);
        ByteArray[5] = 6; //Number of bytes after this one.
        MessageLength = 12;
        client.write((const uint8_t *)ByteArray, MessageLength);
#ifdef MBDebug
        Serial.print("TX: ");
        for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
          Serial.print(ByteArray[thisByte], DEC);
          Serial.print("-");
        }
        Serial.println();
        Serial.print("Write Holding Register: ");
        Serial.print(Start);
        Serial.print("=");
        Serial.println(MBHoldingRegister[Start]);
#endif
        byteFN = MB_FC_NONE;
        break;


      case MB_FC_WRITE_MULTIPLE_REGISTERS:    //16 Write Holding Registers
        ByteDataLength = WordDataLength * 2;
        ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
        for (int i = 0; i < WordDataLength; i++)
        {
          MBHoldingRegister[Start + i] =  word(ByteArray[ 13 + i * 2], ByteArray[14 + i * 2]);
        }
        MessageLength = 12;
        client.write((const uint8_t *)ByteArray, MessageLength);
#ifdef MBDebug
        Serial.print("TX: ");
        for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
          Serial.print(ByteArray[thisByte], DEC);
          Serial.print("-");
        }
        Serial.println();
        Serial.print("Write Holding Registers from: ");
        Serial.print(Start);
        Serial.print("=");
        Serial.println(WordDataLength);
#endif
        byteFN = MB_FC_NONE;
        break;

        default:
        client.flush();
#ifdef MBDebug
        Serial.println("Client flushed");
        byteFN = MB_FC_NONE;
        break;
#endif
                
    }
  }
client.stop();
flagClientConnected == 0;
flagConnectionEstablished = 0;

}
//**********************************************************************







