#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 5

#include <ETH.h>
#include <vector>

#include "AsyncUDP.h"
#include "DanteDevice.h"
#include "DanteDeviceList.h"

static volatile bool eth_connected = false;

DanteDeviceList devices;

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

	// call once in setup to poll for _netaudio_arc services
	Serial.printf ("scanning...\n\r");
	devices.scan ();

	// wait a second for scan to complete
	delay (1000);
}


void loop (void)
{
	// call repetively in loop to poll for _netaudio_arc services if a poll is needed
	devices.scanIfNeeded ();
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
