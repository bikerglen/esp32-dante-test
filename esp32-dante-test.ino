#ifdef ARDUINO_ESP32_GATEWAY_F
  #pragma message "Compiling for Olimex ESP32 Gateway Rev F"
  #define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
  #define ETH_PHY_POWER 5
#else
  #pragma message "Compiling for wireless ESP32 boards"
#endif

#ifdef ARDUINO_ESP32_GATEWAY_F
#include <ETH.h>
#else
#include <WiFi.h>
#endif

#include <vector>
#include <arduino-timer.h>

#include "AsyncUDP.h"
#include "DanteDevice.h"
#include "DanteDeviceList.h"

#include "secrets.h"

// switch defines
#define SW1 (digitalRead (32) ? 0 : 1)
#define SW2 (digitalRead (33) ? 0 : 1)
#define SW3 (digitalRead (34) ? 0 : 1)
#define SW4 (digitalRead (35) ? 0 : 1)
#define SW5 (digitalRead (36) ? 0 : 1)
#define SW6 (digitalRead (39) ? 0 : 1)

static volatile bool eth_connected = false;

DanteDeviceList devices;

auto loopTimer = timer_create_default(); 
bool onLoopTimer (void *);

uint8_t buttonStates[6];
uint8_t thisPressed = 0;
uint8_t lastPressed = 0;

void setup (void)
{
	// set up serial
	Serial.begin(115200);
	delay (100);
	Serial.printf ("Hello, world\n\r");

	// set up GPIO's
	pinMode (32, INPUT);
	pinMode (33, INPUT);
	pinMode (34, INPUT);
	pinMode (35, INPUT);
	pinMode (36, INPUT);
	pinMode (39, INPUT);

	// clear button states
	for (int i = 0; i < 6; i++) {
		buttonStates[i] = 0;
	}

#ifdef ARDUINO_ESP32_GATEWAY_F
	// set up ethernet
	WiFi.onEvent(WiFiEvent);
	ETH.begin();
	Serial.printf ("Returned from ETH.connected.\n\r");

	// wait for ethernet to connect
	while (!eth_connected) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("Ethernet connected!");
	Serial.print("IP address: ");
	Serial.println(ETH.localIP()); 
#else
	// set up wireless
	WiFi.begin (ssid, password);

	// wait for wireless to connect
	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected!");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP()); 
#endif

	// set up Dante mDNS listener
	devices.begin ();

	// seed random number generator
	randomSeed ((uint32_t)IPAddress (224, 0, 0, 251));

	// call once in setup after connected to network to poll for _netaudio_arc services
	Serial.printf ("scanning...\n\r");
	devices.scan ();

	// wait a second for scan to complete
	delay (1000);

	// every 20 ms = 50 Hz
	loopTimer.every (25, onLoopTimer);
}


void loop (void)
{
	// call repetively in loop to poll for _netaudio_arc services
	devices.scanIfNeeded ();

	// loop timer tick
	loopTimer.tick ();
}

 
bool onLoopTimer (void *)
{
	uint8_t newPresses;

	// check buttons
	thisPressed  = ProcessButton (0, SW1);
	thisPressed |= ProcessButton (1, SW2);
	thisPressed |= ProcessButton (2, SW3);
	thisPressed |= ProcessButton (3, SW4);
	thisPressed |= ProcessButton (4, SW5);
	thisPressed |= ProcessButton (5, SW6);
	if (thisPressed != lastPressed) {
		Serial.printf ("buttons are now %02x\n\r", thisPressed);
	}
	newPresses = thisPressed & (~lastPressed);
	if (newPresses & 0x01) {
		Serial.printf ("button 1 pressed!\n\r");
	} 
	if (newPresses & 0x02) {
		Serial.printf ("button 2 pressed!\n\r");
	} 
	if (newPresses & 0x04) {
		Serial.printf ("button 3 pressed!\n\r");
	} 
	if (newPresses & 0x08) {
		Serial.printf ("button 4 pressed!\n\r");
	} 
	if (newPresses & 0x10) {
		Serial.printf ("button 5 pressed!\n\r");
	} 
	if (newPresses & 0x20) {
		Serial.printf ("button 6 pressed!\n\r");
	}
	lastPressed = thisPressed;

	// return true to keep timer armed
	return true;
}


uint8_t ProcessButton (uint8_t which, uint8_t sw)
{
	uint8_t state;

	state = buttonStates[which];

	switch (state) {
		case 0: state = sw ? 1 : 0; break;
		case 1: state = sw ? 2 : 0; break;
		case 2: state = sw ? 2 : 3; break;
		case 3: state = sw ? 2 : 0; break;
	}

	buttonStates[which] = state;

	return (state & 2) ? (1 << which) : 0;
}


#ifdef ARDUINO_ESP32_GATEWAY_F
void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
    
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}
#endif
