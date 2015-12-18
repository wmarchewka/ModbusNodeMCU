#include <Arduino.h>
#include "ModbusTCPSlave.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include "TimeLib.h"

//WIFI Settings
byte ip[] = { 10, 0, 0, 202 };
byte gateway[] = { 10, 0, 0, 100 };
byte subnet[] = { 255, 255, 255, 0 };

ModbusTCPSlave Mb;
Flasher led3(16, 100, 100);

ADC_MODE(ADC_VCC);


unsigned long timer;
unsigned long checkRSSIMillis;
bool UpdateMode;
char AppVersion[] = "1.0.16";
int count = 0;
unsigned long previousMillis = 0;
int OTAsecondsCounter;
bool OTAupdateStarted = false;



////********************************************************************************************
void OTA() {

	Serial.println("Booting");
	Serial.println("Update mode running");
	WiFi.mode(WIFI_STA);
	WiFi.begin("WALTERMARCHEWKA", "Molly11315110");
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}

	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	// ArduinoOTA.setHostname("myesp8266");

	// No authentication by default
	//ArduinoOTA.setPassword((const char *)"123");

	ArduinoOTA.onStart([]() {
		Serial.println("Start");
		led3.Update();
		OTAupdateStarted = true;
	});
	ArduinoOTA.onEnd([]() {
		led3.Off();
		Serial.println("End");
		Serial.println("Restarting...");
		EEPROM.write(0, 0);
		EEPROM.commit();
		ESP.restart();
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		led3.Update();
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		led3.Update();
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
		ESP.restart();
	});
	ArduinoOTA.begin();
	Serial.println("Ready for OTA update");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}
//*******************************************************************************
void setup() {

	pinMode(16, OUTPUT);
	digitalWrite(16, LOW);
	int softwareVersion;


#ifdef MBDebug
	Serial.begin(115200);
#endif

	delay(5000);

	EEPROM.begin(10);
	//eeprom mapping
	//0 = update mode 0/1
	//1 = 
	//2 = 
	//3 = savedEpoch byte 1
	//4 = saved epoch byte 2
	//5 = saved epoch byte 3
	//6 = saved epoch byte 4
	//7 = software Version


	UpdateMode = EEPROM.read(0);
	softwareVersion = CheckVersion();
	Serial.print("\n\r");
	Serial.print("App version ");
	Serial.println(softwareVersion);
	Serial.print("Chip ID-");
	Serial.println(ESP.getChipId());


	if (UpdateMode) {
		OTA();
	}
	else {
		Mb.begin("WALTERMARCHEWKA", "Molly11315110", ip, gateway, subnet);
		for (int x = 0; x < 20; x++) {
			Mb.MBHoldingRegister[x] = x;
		}
	}

}

////********************************************************************************************
void loop() {

	
	long currentMillis = millis();


	
	if (UpdateMode) {
		ArduinoOTA.handle();
		led3.Update();
		delay(20);
		if ( (currentMillis - previousMillis >= 1000) & !OTAupdateStarted ) {
			previousMillis = currentMillis;
			OTAsecondsCounter++;
			Serial.print("Waiting for update for ");
			Serial.print(OTAsecondsCounter);
			Serial.println(" seconds");
			if (OTAsecondsCounter >= 60) {
				Serial.println("Rebooting due to no update");
				EEPROM.write(0, 0);   //write 0 to startup in run mode
				EEPROM.commit();
				ESP.restart(); 
			}
		}
	}
	else {
		Mb.Run();

	}
	//allow yield to esp for wifi tasks to run
	delay(20);
}

////********************************************************************************************
//This function will write a 4 byte (32bit) long to the eeprom at
//the specified address to address + 3.
void EEPROMWritelong(int address, long value)
{
	//Decomposition from a long to 4 bytes by using bitshift.
	//One = Most significant -> Four = Least significant byte
	byte four = (value & 0xFF);
	byte three = ((value >> 8) & 0xFF);
	byte two = ((value >> 16) & 0xFF);
	byte one = ((value >> 24) & 0xFF);

	//Write the 4 bytes into the eeprom memory.
	EEPROM.write(address, four);
	EEPROM.write(address + 1, three);
	EEPROM.write(address + 2, two);
	EEPROM.write(address + 3, one);
	EEPROM.commit();
}
////********************************************************************************************
long EEPROMReadlong(long address)
{
	//Read the 4 bytes from the eeprom memory.
	long four = EEPROM.read(address);
	long three = EEPROM.read(address + 1);
	long two = EEPROM.read(address + 2);
	long one = EEPROM.read(address + 3);

	//Return the recomposed long by using bitshift.
	return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
////********************************************************************************************      
long CheckVersion() {

	//Dec 15 2015
	//09:47:17

	double epochTime;
	String month;
	String day;
	String year;
	String hour;
	String minute;
	String sec;
	int intMonth;
	long savedEpoch = EEPROMReadlong(3);
	int softwareVersion = EEPROM.read(7);
	int newSoftwareVersion;
	String newDate = strDate;
	String newTime = strTime;
	long timeSnapshot;

	Serial.println(strDate);
	Serial.println(strTime);

	month = newDate.substring(0, 3);
	day = newDate.substring(4, 6);
	year = newDate.substring(7, 11);
	hour = newTime.substring(0, 2);
	minute = newTime.substring(3, 5);
	sec = newTime.substring(6, 8);

	//convert month string into integer
	if (month == "Jan") intMonth = 1;
	else if (month == "Feb") intMonth = 2;
	else if (month == "Mar") intMonth = 3;
	else if (month == "Apr") intMonth = 4;
	else if (month == "May") intMonth = 5;
	else if (month == "Jun") intMonth = 6;
	else if (month == "Jul") intMonth = 7;
	else if (month == "Aug") intMonth = 8;
	else if (month == "Sep") intMonth = 9;
	else if (month == "Oct") intMonth = 10;
	else if (month == "Nov") intMonth = 11;
	else if (month == "Dec") intMonth = 12;

	//hr, min, sec, day, mon, year
	setTime(hour.toInt(), minute.toInt(), sec.toInt(), day.toInt(), intMonth, year.toInt());

	timeSnapshot = now();

	Serial.print("time is epoch ");
	Serial.println(timeSnapshot);

	Serial.print("saved epoch ");
	Serial.println(savedEpoch);

	Serial.print("Old software version ");
	Serial.println(softwareVersion);


	if (timeSnapshot > savedEpoch) {
		newSoftwareVersion = softwareVersion;
		++newSoftwareVersion;
		EEPROMWritelong(3, timeSnapshot);
		EEPROM.write(7, newSoftwareVersion);
		EEPROM.commit();
		Serial.print("New version ");
		Serial.println(newSoftwareVersion);
	}
	else {

		newSoftwareVersion = softwareVersion;
	}

	return newSoftwareVersion;
}
////********************************************************************************************

