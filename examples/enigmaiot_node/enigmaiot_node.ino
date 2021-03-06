/**
  * @file enigmaiot_node.ino
  * @version 0.8.2
  * @date 03/03/2020
  * @author German Martin
  * @brief Node based on EnigmaIoT over ESP-NOW
  *
  * Sensor reading code is mocked on this example. You can implement any other code you need for your specific need
  */

#ifndef ESP8266
#error Node only supports ESP8266 platform
#endif

#include <Arduino.h>
#include <EnigmaIOTNode.h>
#include <espnow_hal.h>
#include <CayenneLPP.h>

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Curve25519.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <ESPAsyncTCP.h>
#include <Hash.h>
#include <DNSServer.h>

#define BLUE_LED LED_BUILTIN
constexpr auto RESET_PIN = 13;

ADC_MODE (ADC_VCC);

void connectEventHandler () {
	Serial.println ("Registered");
}

void disconnectEventHandler () {
	Serial.println ("Unregistered");
}

void processRxData (const uint8_t* mac, const uint8_t* buffer, uint8_t length, nodeMessageType_t command) {
	char macstr[18];
	String commandStr;

	mac2str (mac, macstr);
	Serial.println ();
	Serial.printf ("Data from %s\n", macstr);
	if (command == nodeMessageType_t::DOWNSTREAM_DATA_GET)
		commandStr = "GET";
	else if (command == nodeMessageType_t::DOWNSTREAM_DATA_SET)
		commandStr = "SET";
	else
		return;

	Serial.printf ("Command %s\n", commandStr.c_str ());
	Serial.printf ("Data: %s\n", printHexBuffer (buffer, length));

	//for (int i = 0; i < length; i++) {
	//	Serial.print ((char)buffer[i]);
	//}
	//Serial.println ();
	//Serial.println ();
}

void setup () {

	Serial.begin (115200); Serial.println (); Serial.println ();
	time_t start = millis ();
	
	EnigmaIOTNode.setLed (BLUE_LED);
	EnigmaIOTNode.setResetPin (RESET_PIN);
	EnigmaIOTNode.onConnected (connectEventHandler);
	EnigmaIOTNode.onDisconnected (disconnectEventHandler);
	EnigmaIOTNode.onDataRx (processRxData);

	EnigmaIOTNode.begin (&Espnow_hal);
	
	// Put here your code to read sensor and compose buffer
    CayenneLPP msg (MAX_DATA_PAYLOAD_SIZE);

	msg.addAnalogInput (0, (float)(ESP.getVcc ()) / 1000);
    msg.addTemperature (1, 20.34);

	Serial.printf ("Vcc: %f\n", (float)(ESP.getVcc ()) / 1000);
	// End of user code

	Serial.printf ("Trying to send: %s\n", printHexBuffer (msg.getBuffer (), msg.getSize ()));

    // Send buffer data
	if (!EnigmaIOTNode.sendData (msg.getBuffer (), msg.getSize ())) {
		Serial.println ("---- Error sending data");
	} else {
		Serial.println ("---- Data sent");
	}
	Serial.printf ("Total time: %d ms\n", millis() - start);

    // Go to sleep
	EnigmaIOTNode.sleep ();
}

void loop () {

	EnigmaIOTNode.handle ();

}
