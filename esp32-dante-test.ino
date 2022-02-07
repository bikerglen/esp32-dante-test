#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 5

#include <ETH.h>
#include <vector>

#include "AsyncUDP.h"
#include "DanteDevice.h"
#include "DanteDeviceList.h"

static volatile bool eth_connected = false;

DanteDeviceList devices;

uint32_t lastScanTime;
uint32_t scanState;
uint32_t lastScanCheckTime;
uint32_t randomScanDelay;

void setup (void)
{
	// set up serial
	Serial.begin(115200);
	delay (100);
	Serial.printf ("Hello, world\n\r");

	// set up ethernet
	WiFi.onEvent(WiFiEvent);
	ETH.begin();
	Serial.printf ("Returned from ETH.connected.\n\r");

	// set up Dante mDNS listener
	devices.begin ();

	// wait for ethernet to connect
	while (!eth_connected) {
	}

	// seed random number generator
	randomSeed ((uint32_t)IPAddress (224, 0, 0, 251));

	// start a scan
	Serial.printf ("scanning...\n\r");
	devices.scan ();
	lastScanTime = millis ();
	scanState = 0;
	lastScanCheckTime = 0;
	randomScanDelay = 0;

	// wait a second for scan to complete
	delay (1000);
}


void loop (void)
{
	// do we need to do another check for devices and updated IP addresses
	bool doScan = false;

	// if no known devices, run a scan every 30 seconds 
	if (devices.getDeviceCount () == 0) {
		if (millis () - lastScanTime > 1000 * 30) {
			Serial.printf ("doScan: no known devices and 30 seconds since last scan\n\r");
			doScan = true;
		}
	} else {
		// devices known, see if any need updates then update 50ms to 5s later
		// can't just repeat every two minutes since another device may have already
		// requested a scan and caused everything to already be updated
		if (scanState == 0) {
			// no scan scheduled, see if we need to do a scan
			if (devices.checkUpdateNeeded ()) {
				// schedule the scan from 50ms to 5s from now
				Serial.printf ("doScan: updates needed\n\r");
				lastScanCheckTime = millis ();
				randomScanDelay = random (50,5000); // 50 ms to 5 seconds holdoff
				scanState = 1;
			}
		} else if (scanState == 1) {
			// scan scheduled. once the random delay has elapsed, do one more check. 
			// if scan still needed, run the scan
			if (millis () - lastScanCheckTime > randomScanDelay) {
				Serial.printf ("doScan: random holdoff timer expired\n\r");
				if (devices.checkUpdateNeeded ()) {
					Serial.printf ("doScan: updates still needed after timer expiration\n\r");
					doScan = true;
					scanState = 2;
					lastScanCheckTime = millis ();
				} else {
					Serial.printf ("doScan: updates not needed aftr timer expiration\n\r");
					scanState = 0;
					lastScanCheckTime = 0;
					randomScanDelay = 0;
				}
			}
		} else if (scanState == 2) {
			// don't allow another scan for 1 second after scan started
			if (millis () - lastScanCheckTime > 1000) {
				Serial.printf ("doScan: post update timer expired\n\r");
				scanState = 0;
				lastScanCheckTime = 0;
				randomScanDelay = 0;
			}
		}
	}

	if (doScan) {
		Serial.printf ("doScan: scan started!\n\r");
		devices.scan ();
		lastScanTime = millis ();
	}
}


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
