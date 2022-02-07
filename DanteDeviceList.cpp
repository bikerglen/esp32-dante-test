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
#include "RingBuf.h"

#include <vector>

#include "DanteDevice.h"
#include "DanteDeviceList.h"

DanteDeviceList::DanteDeviceList (void)
{
}


DanteDeviceList::~DanteDeviceList (void)
{
}


bool DanteDeviceList::begin (void)
{
	bool success = false; // assume we will fail

	// mDNS multicast IP address
	IPAddress address = IPAddress (224, 0, 0, 251);
	
	// mDNS multicast port
	if (udp.listenMulticast (address, 5353)) {
		ip4_addr_t ifaddr;
		ip4_addr_t multicast_addr;

		ifaddr.addr = static_cast<uint32_t>(WiFi.localIP());
		multicast_addr.addr = static_cast<uint32_t>(IPAddress (224, 0, 0, 251));
		igmp_joingroup (&ifaddr, &multicast_addr);
		udp.onPacket(std::bind(&DanteDeviceList::parsePacket, this,
		std::placeholders::_1));

		success = true;
	}

	return success;
}


/*
void DanteDeviceList::pollForDevices (void)
{
	0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x0d,0x5f,0x6e,0x65,
    0x74,0x61,0x75,0x64,0x69,0x6f,0x2d,0x61,0x72,0x63,0x04,0x5f,0x75,0x64,0x70,0x05,
    0x6c,0x6f,0x63,0x61,0x6c,0x00,0x00,0x0c,0x80,0x01

}
*/


void DanteDeviceList::parsePacket (AsyncUDPPacket _packet)
{
	IPAddress localAddr = _packet.localIP ();
	IPAddress remoteAddr = _packet.remoteIP ();

	Serial.printf ("packet received!\n\r");
	Serial.printf ("  local:  %3d.%3d.%3d.%3d:%d\n\r", 
		localAddr[0], localAddr[1],
		localAddr[2], localAddr[3], _packet.localPort ());
	Serial.printf ("  remote: %3d.%3d.%3d.%3d:%d\n\r", 
		remoteAddr[0], remoteAddr[1],
		remoteAddr[2], remoteAddr[3], _packet.remotePort ());

	uint8_t *packet = _packet.data ();

	// TODO -- assume well-formed packets for now, add checking for errors later

	int index = 0;

	uint16_t tid         = (packet[index++] << 8) | packet[index++];
    uint16_t flags       = (packet[index++] << 8) | packet[index++];
    uint16_t questions   = (packet[index++] << 8) | packet[index++];
    uint16_t answers     = (packet[index++] << 8) | packet[index++];
    uint16_t authorities = (packet[index++] << 8) | packet[index++];
    uint16_t additional  = (packet[index++] << 8) | packet[index++];

    Serial.printf ("  tid:       %04x\n\r", tid);
    Serial.printf ("  flags:     %04x\n\r", flags);
    Serial.printf ("  questions: %04x\n\r", questions);
    Serial.printf ("  answers:   %04x\n\r", answers);
    Serial.printf ("  auths:     %04x\n\r", authorities);
    Serial.printf ("  addits:    %04x\n\r", additional);

    int records = questions + answers + authorities + additional;

    for (int record = 0; record < records; record++) {

        uint8_t name[256];
        uint8_t data[256];

        index = parseDnsName (packet, index, name, false);

        Serial.printf ("  name:      %s\n\r", name);

        uint16_t type   = (packet[index++] << 8)  | packet[index++];
        uint16_t pclass = (packet[index++] << 8)  | packet[index++]; 
        bool flush = (pclass & 0x8000) ? 1 : 0;
        pclass &= 0x7fff;
        Serial.printf ("    type:    %d\n\r", type);
        Serial.printf ("    class:   %d\n\r", pclass);
        Serial.printf ("    flush:   %d\n\r", flush);

		if (record >= questions) {
			uint32_t ttl    = (packet[index++] << 24) | (packet[index++] << 16) |
							 (packet[index++] <<  8) | (packet[index++] <<  0);
			uint16_t dlen   = (packet[index++] << 8)  | packet[index++];

			Serial.printf ("    ttl:     %d\n\r", ttl);
			Serial.printf ("    dlen:    %d\n\r", dlen);

			if (type == 12) {
				parseDnsName (packet, index, data, false);
				Serial.printf ("      PTR:     %s\n\r", data);
			} else if (type == 1) {
				Serial.printf ("      IP Addr: %3d.%3d.%3d.%3d\n\r", 
					packet[index+0], packet[index+1],
					packet[index+2], packet[index+3]);
			}
			index += dlen;
		}
    }
}


int DanteDeviceList::parseDnsName (uint8_t *packet, int index, uint8_t *name, bool reference)
{
    int length = 0;
    int nlength = 0;

    while ((length = packet[index++]) != 0) {
        if (length == 0xc0) {
            nlength += parseDnsName (packet, packet[index++], &name[nlength], true);
            break;
        } else {
            if (reference || (nlength != 0)) {
                name[nlength++] = '.';
            }
            memcpy (&name[nlength], &packet[index], length);
            index += length;
            nlength += length;
        }
    }

    if (reference) {
        return nlength;
    }

    name[nlength++] = 0;

    return index;
}
