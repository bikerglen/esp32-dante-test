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
#include <Wire.h>
#include <RingBuf.h>

#include "AsyncUDP.h"
#include "DanteDevice.h"
#include "DanteDeviceList.h"
#include "DanteDeviceMonitor.h"

#include "secrets.h"

#include "ButtonPadFour.h"
// #include "ButtonPadSix.h"

static volatile bool eth_connected = false;

DanteDeviceList devices;
DanteDeviceMonitor deviceMonitor;

auto loopTimer = timer_create_default(); 
bool onLoopTimer (void *);

TwoWire wire = TwoWire (0);
ButtonPadFour buttonPad = ButtonPadFour ();
// ButtonPadSix buttonPad = ButtonPadSix ();

void setup (void)
{
	// set up serial
	Serial.begin(115200);
	delay (100);
	Serial.printf ("Hello, world\n\r");

#ifdef ARDUINO_ESP32_GATEWAY_F
	wire.begin (15, 14);
	buttonPad.begin (32, 33, 34, 35, 36, 39, &wire);
#else
	wire.begin (26, 27);
	buttonPad.begin (&wire);
	// buttonPad.begin (34, 35, 32, 33, 25, 14, &wire);
#endif

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

	// seed random number generator from IP address
	randomSeed ((uint32_t)ETH.localIP());
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

	// seed random number generator from IP address
	randomSeed ((uint32_t)WiFi.localIP());
#endif

	// set up Dante mDNS listener
	devices.begin ();

	// set up Dante config change monitor
	deviceMonitor.begin ();

	// call once in setup after connected to network to poll for _netaudio_arc services
	Serial.printf ("scanning...\n\r");
	devices.scan ();

	// wait a second for scan to complete
	delay (1000);

	// every 20 ms = 50 Hz
	loopTimer.every (20, onLoopTimer);
}


void loop (void)
{
	uint8_t key;
	DanteDevice *device;

	// call repetively in loop to poll for _netaudio_arc services
	devices.scanIfNeeded ();

	// update any new devices, update any changed subscriptions
	devices.populateNewDevices ();

	// check for configuration changes
	device = deviceMonitor.changed (&devices);
	if (device) {
		// The device monitor detects the subscription change packets from the
		// dante receiver and returns a pointer to the correspond device from devices. 
		// We then increment the device's subscription change count, and when subscription 
		// change count is non-zero, device list populate new devices will retrieve the 
		// udpated subscription information from the dante receiver and set the subscription 
		// updated flag for use by the MQTT publisher and/or local button lights.
		// receive change packet -> query device for updated info -> send MQTT request
		Serial.printf ("device changed\n\r");
		device->incrementSubscriptionChanges ();
	}

	// loop timer tick
	loopTimer.tick ();

	if (Serial.available ()) {
		key = Serial.read ();
		if (key == 'l') {
			// list devices
			devices.listDevices ();
		} else if (key == 's') {
			// force an mDNS scan
			devices.scan ();
		}
	}
}

 
bool onLoopTimer (void *)
{
	uint8_t newPresses;
	DanteDevice *receiver;

	// check buttons
	newPresses = buttonPad.tick ();

	// display pressed buttons
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
	if (newPresses & 0x40) {
		Serial.printf ("button 6 pressed!\n\r");
	} 
	if (newPresses & 0x80) {
		Serial.printf ("button 7 pressed!\n\r");
	}

	// TODO -- add some sort of flag that can be checked to indicate a 
	// TODO -- subscription update then cleared when done
	// TODO -- add mqtt publish of all device subscription status changes
	// TODO -- subscribe to all mqtt subscription change commands
	receiver = devices.searchName (LIVING_ROOM);
	if (receiver) {
		DanteRxChannel *rxChan1 = receiver->searchRxChannelName ("CH1");
		DanteRxChannel *rxChan2 = receiver->searchRxChannelName ("CH2");
		if (rxChan1 && rxChan2) {
			if (rxChan1->isConnected (USB_MUSIC_SERVER, "Left") &&
				rxChan2->isConnected (USB_MUSIC_SERVER, "Right")) {
				// button 1 red, button 2 off
				buttonPad.setButtonColor (0, 0xff, 0x00, 0x00);
				buttonPad.setButtonColor (1, 0x00, 0x00, 0x00);
			} else if (rxChan1->isConnected (BT_IPAD_OR_PHONE, "Left") &&
					   rxChan2->isConnected (BT_IPAD_OR_PHONE, "Right")) {
				// button 1 off, button 2 red
				buttonPad.setButtonColor (0, 0x00, 0x00, 0x00);
				buttonPad.setButtonColor (1, 0xff, 0x00, 0x00);
			} else {
				// unknown combination or not connected
				// both buttons yellow
				buttonPad.setButtonColor (0, 0x80, 0x80, 0x00);
				buttonPad.setButtonColor (1, 0x80, 0x80, 0x00);
			}
		} else {
			// both channel names not found; set both buttons red
			buttonPad.setButtonColor (0, 0xff, 0x00, 0x00);
			buttonPad.setButtonColor (1, 0xff, 0x00, 0x00);
		}
	}

	// buttonPad.setButtonColor (0, 0xff, 0x00, 0x00);
	// buttonPad.setButtonColor (1, 0x80, 0x80, 0x00);
	// buttonPad.setButtonColor (2, 0x00, 0xff, 0x00);
	// buttonPad.setButtonColor (3, 0x00, 0x80, 0x80);
	// buttonPad.setButtonColor (4, 0x00, 0x00, 0xff);
	// buttonPad.setButtonColor (5, 0x80, 0x00, 0x80);

	// return true to keep timer armed
	return true;
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
