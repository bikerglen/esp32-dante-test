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


static const uint8_t _netaudio_arc[] = {
	0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x0d,0x5f,0x6e,0x65,
    0x74,0x61,0x75,0x64,0x69,0x6f,0x2d,0x61,0x72,0x63,0x04,0x5f,0x75,0x64,0x70,0x05,
    0x6c,0x6f,0x63,0x61,0x6c,0x00,0x00,0x0c,0x80,0x01
};

static const uint8_t _netaudio_dbc[] = {
    0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x0d,0x5f,0x6e,0x65,
    0x74,0x61,0x75,0x64,0x69,0x6f,0x2d,0x64,0x62,0x63,0x04,0x5f,0x75,0x64,0x70,0x05,
    0x6c,0x6f,0x63,0x61,0x6c,0x00,0x00,0x0c,0x80,0x01
};

static const uint8_t _netaudio_cmc[] = {
    0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x0d,0x5f,0x6e,0x65,
    0x74,0x61,0x75,0x64,0x69,0x6f,0x2d,0x63,0x6d,0x63,0x04,0x5f,0x75,0x64,0x70,0x05,
    0x6c,0x6f,0x63,0x61,0x6c,0x00,0x00,0x0c,0x80,0x01
};

void DanteDeviceList::scan (void)
{
	udp.write (_netaudio_arc, sizeof (_netaudio_arc));
	// udp.write (_netaudio_dbc, sizeof (_netaudio_dbc));
	// udp.write (_netaudio_cmc, sizeof (_netaudio_cmc));
}


void DanteDeviceList::parsePacket (AsyncUDPPacket _packet)
{
	IPAddress localAddr = _packet.localIP ();
	IPAddress remoteAddr = _packet.remoteIP ();

	Serial.printf ("mdns packet: local: %d.%d.%d.%d:%d, remote: %d.%d.%d.%d:%d\n\r", 
		localAddr[0], localAddr[1], localAddr[2], localAddr[3], _packet.localPort (),
		remoteAddr[0], remoteAddr[1], remoteAddr[2], remoteAddr[3], _packet.remotePort ());

	uint8_t *packet = _packet.data ();

/*
	if (remoteAddr[3] == 124) {
		for (int i = 0; i < _packet.length(); i++) {
			Serial.printf ("%02x ", packet[i]);
		}
		Serial.printf ("\n\r");
	}
*/

	// TODO -- assume well-formed packets for now, add checking for errors later

	int index = 0;

	uint16_t tid         = (packet[index++] << 8) | packet[index++];
    uint16_t flags       = (packet[index++] << 8) | packet[index++];
    uint16_t questions   = (packet[index++] << 8) | packet[index++];
    uint16_t answers     = (packet[index++] << 8) | packet[index++];
    uint16_t authorities = (packet[index++] << 8) | packet[index++];
    uint16_t additional  = (packet[index++] << 8) | packet[index++];

    // Serial.printf ("  tid:       %04x\n\r", tid);
    // Serial.printf ("  flags:     %04x\n\r", flags);
    // Serial.printf ("  questions: %04x\n\r", questions);
    // Serial.printf ("  answers:   %04x\n\r", answers);
    // Serial.printf ("  auths:     %04x\n\r", authorities);
    // Serial.printf ("  addits:    %04x\n\r", additional);

    int records = questions + answers + authorities + additional;

	bool interesting = false;
	bool aRecordFound = false; // DNS "A" record found in response
	uint8_t aRecordName[256];
	IPAddress aRecordAddr;
	uint32_t aRecordTTL;

    for (int record = 0; record < records; record++) {

        uint8_t name[256];
        uint8_t data[256];

        index = parseDnsName (packet, index, name, false);
		// Serial.printf ("  name:      %s\n\r", name);

		if (!memcmp (name, "_netaudio-arc._udp.local", 25)) {
			interesting = true;
		}

        uint16_t type   = (packet[index++] << 8)  | packet[index++];
        uint16_t pclass = (packet[index++] << 8)  | packet[index++]; 
        bool flush = (pclass & 0x8000) ? 1 : 0;
        pclass &= 0x7fff;

		// Serial.printf ("    type:    %d\n\r", type);
		// Serial.printf ("    class:   %d\n\r", pclass);
		// Serial.printf ("    flush:   %d\n\r", flush);

		if (record >= questions) {
			uint32_t ttl    = (packet[index++] << 24) | (packet[index++] << 16) |
							 (packet[index++] <<  8) | (packet[index++] <<  0);
			uint16_t dlen   = (packet[index++] << 8)  | packet[index++];

			// Serial.printf ("    ttl:     %d\n\r", ttl);
			// Serial.printf ("    dlen:    %d\n\r", dlen);

			if (type == 12) {
				parseDnsName (packet, index, data, false);
				// Serial.printf ("      PTR:     %s\n\r", data);
			} else if (type == 1) {
/*
				Serial.printf ("      IP Addr: %d.%d.%d.%d\n\r", 
					packet[index+0], packet[index+1],
					packet[index+2], packet[index+3]);
*/
				// found the "A" record, save name, ip, and ttl
				aRecordFound = true;
				memcpy (aRecordName, name, strlen ((char *)name)+1);
				aRecordAddr = IPAddress (
					packet[index+0], packet[index+1],
					packet[index+2], packet[index+3]);
				aRecordTTL = ttl;
			}

			index += dlen;
		}
    }

	if (interesting && aRecordFound) {
		Serial.printf ("  name: %s\n\r  addr: %d.%d.%d.%d\n\r",
			aRecordName, aRecordAddr[0], aRecordAddr[1], aRecordAddr[2], aRecordAddr[3]);
		
		// see if device already exists or not
		DanteDevice *device = this->searchfqn (aRecordName);
		if (device) {
			device->updateAddress (aRecordAddr, aRecordTTL);
		} else {
			DanteDevice *device = new DanteDevice (aRecordName, aRecordAddr, aRecordTTL);
			devices.push_back (device);
		}
	}
}


int DanteDeviceList::parseDnsName (uint8_t *packet, int index, uint8_t *name, bool reference)
{
    int length = 0;
    int nlength = 0;

    while ((length = packet[index++]) != 0) {
        if ((length & 0xc0) == 0xc0) {
            int new_index = ((length & 0x3F) << 8) | packet[index++];
            nlength += parseDnsName (packet, new_index, &name[nlength], true);
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


DanteDevice *DanteDeviceList::searchfqn (uint8_t *name)
{
	std::vector<DanteDevice *>::iterator it;

	for (it = devices.begin(); it != devices.end(); it++) {
		if (!strcmp ((char *)(*it)->name, (char *)name)) {
			return *it;
		}
	}
		
	return NULL;
}
