#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <memory.h>

#include <WiFi.h>
#include <AsyncUDP.h>

#include <lwip/ip_addr.h>
#include <lwip/igmp.h>
#include <Arduino.h>
#include <RingBuf.h>

#include <vector>

#include "DanteDevice.h"
#include "DanteDeviceList.h"
#include "DanteDeviceMonitor.h"

DanteDeviceMonitor::DanteDeviceMonitor (void)
{
	// allocate a buffer for received packets' IP addresses
	pbuff = RingBuf_new (sizeof (IPAddress), 32);
}


DanteDeviceMonitor::~DanteDeviceMonitor (void)
{
}


bool DanteDeviceMonitor::begin (void)
{
	bool success = false; // assume we will fail

	// mDNS multicast IP address
	IPAddress address = IPAddress (224, 0, 0, 231);
	
	// mDNS multicast port
	if (udp.listenMulticast (address, 8702)) {
		ip4_addr_t ifaddr;
		ip4_addr_t multicast_addr;

		ifaddr.addr = static_cast<uint32_t>(WiFi.localIP());
		multicast_addr.addr = static_cast<uint32_t>(IPAddress (224, 0, 0, 231));
		igmp_joingroup (&ifaddr, &multicast_addr);
		udp.onPacket(std::bind(&DanteDeviceMonitor::parsePacket, this,
		std::placeholders::_1));

		success = true;
	}

	return success;
}


void DanteDeviceMonitor::parsePacket (AsyncUDPPacket _packet)
{
	IPAddress localAddr = _packet.localIP ();
	IPAddress remoteAddr = _packet.remoteIP ();

	Serial.printf ("224.0.0.231:8702 packet: local: %d.%d.%d.%d:%d, remote: %d.%d.%d.%d:%d\n\r", 
		localAddr[0], localAddr[1], localAddr[2], localAddr[3], _packet.localPort (),
		remoteAddr[0], remoteAddr[1], remoteAddr[2], remoteAddr[3], _packet.remotePort ());

	uint8_t *packet = _packet.data ();

	if ((packet[0] == 0xff) &&	// 0xff
	    (packet[1] == 0xff) &&	// 0xff
	    (packet[2] == 0x00) &&	// 0x00 length hi
	    (packet[3] == 0x23) &&	// 0x23 length lo
		(packet[16] == 'A') &&
		(packet[17] == 'u') &&
		(packet[18] == 'd') &&
		(packet[19] == 'i') &&
		(packet[20] == 'n') &&
		(packet[21] == 'a') &&
		(packet[22] == 't') &&
		(packet[23] == 'e') &&
		(packet[26] == 0x01) &&
		(packet[27] == 0x02)) {
		Serial.printf ("  receiver changed subscriptions on channel %d\n\r", packet[34]);

		// push IP addresses of received packets into ring buffer
		this->pbuff->add (this->pbuff, &remoteAddr);
	}
}


DanteDevice *DanteDeviceMonitor::changed (DanteDeviceList *devices)
{
	IPAddress address;
	DanteDevice *device = NULL;

	// pop IP address off ring buffer
	if (!this->pbuff->isEmpty(this->pbuff)) {
		this->pbuff->pull (this->pbuff, &address);

		Serial.printf ("  popped changed IP address %d.%d.%d.%d\n\r", 
			address[0], address[1], address[2], address[3]);
		
		// search DanteDeviceList for IP address of changed device
		device = devices->searchAddress (address);
	}
	
	// return device or null if none
	return device;
}

/*
change subscription on 2 channels:
      lengt -seq-       aa bb cc       dd ee ff A  u  d  i  n  a  t  e           xx                   yy
ff ff 00 23 f1 aa 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 02 00 00 00 00 00 01 02
ff ff 00 23 f1 ab 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 05 00 00 00 00 00 01 03
ff ff 00 23 f1 ac 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 02 00 00 00 00 00 01 02
ff ff 00 23 f1 ad 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 05 00 00 00 00 00 01 01

unsubscrbe left channel:
      lengt -seq-       aa bb cc       dd ee ff A  u  d  i  n  a  t  e           xx                   yy
ff ff 00 23 f2 b3 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 02 00 00 00 00 00 01 01
ff ff 00 23 f2 b4 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 05 00 00 00 00 00 01 01

subscribe left channel:
      lengt -seq-       aa bb cc       dd ee ff A  u  d  i  n  a  t  e           xx                   yy
ff ff 00 23 f5 ca 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 02 00 00 00 00 00 01 01
ff ff 00 23 f5 cb 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 05 00 00 00 00 00 01 01

unsubscribe then subscribe right channel:
      lengt -seq-       aa bb cc       dd ee ff A  u  d  i  n  a  t  e           xx                   yy
ff ff 00 23 f6 26 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 02 00 00 00 00 00 01 02
ff ff 00 23 f6 27 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 05 00 00 00 00 00 01 01
ff ff 00 23 f6 2a 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 02 00 00 00 00 00 01 02
ff ff 00 23 f6 2b 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 01 05 00 00 00 00 00 01 01

random packet when starting controller app
      lengt -seq-       aa bb cc       dd ee ff A  u  d  i  n  a  t  e           xx                   yy
ff ff 00 ec f6 e0 00 00 00 1d c1 ff fe 51 18 28 41 75 64 69 6e 61 74 65 07 31 00 60 00 00 00 00 04 01 00 
09 04 01 00 09 01 00 00 00 44 41 4f 32 00 00 00 00 0d 50 50 db 00 00 00 64 00 00 00 00 00 00 00 02 00 00
00 01 01 00 00 00 00 00 00 01 00 00 03 00 00 00 00 00 44 41 4f 32 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 1f 00 00 00 0d 00 00 00 03 00 00 00 03 00 00 00 00
*/
