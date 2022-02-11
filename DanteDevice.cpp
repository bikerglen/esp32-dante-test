#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <memory.h>

#include <WiFi.h>
#include <AsyncUDP.h>

#include <Arduino.h>

#include "DanteDevice.h"

DanteDevice::DanteDevice (String server, IPAddress address, uint32_t timeToLive)
{
	uint32_t now = millis ();

	Serial.printf ("  adding new device\n\r");
	Serial.printf ("    server:     %s\n\r", server.c_str());
	Serial.printf ("    ip address: %d.%d.%d.%d\n\r",
										address[0], address[1], address[2], address[3]);
	Serial.printf ("    ttl:        %d\n\r", timeToLive);
	Serial.printf ("    now:        %d\n\r", now);
		
	this->server = server;
	this->address = address;
	this->timeToLive = timeToLive;
	this->updated = now;
	this->missing = false;
	this->isNew = true;
}

DanteDevice::~DanteDevice (void)
{
}

void DanteDevice::updateAddress (IPAddress address, uint32_t timeToLive)
{
	uint32_t now = millis ();

	Serial.printf ("  updating device\n\r");
	Serial.printf ("    server:      %s\n\r", this->server.c_str());
	Serial.printf ("    ip address:  %d.%d.%d.%d\n\r",
										address[0], address[1], address[2], address[3]);
	Serial.printf ("    ttl:         %d\n\r", timeToLive);
	Serial.printf ("    now:         %d\n\r", now);

	Serial.printf ("    was missing: %s\n\r", this->missing ? "yes" : "no");

	this->address = address;
	this->timeToLive = timeToLive;
	this->updated = now;
	this->missing = false;
}

void DanteDevice::setMissing (void)
{
	this->missing = true;
}

bool DanteDevice::getMissing (void)
{
	return this->missing;
}

String DanteDevice::getServer (void)
{	
	return server;
}

String DanteDevice::getName (void)
{
	// retrieve device name from device if needed
	if (this->name == "") {
		this->commandGetDeviceName ();
	}

	return this->name;
}


int DanteDevice::command (uint16_t port, uint8_t seq1, uint16_t cmdLen, uint16_t seq2,
		uint16_t cmd, int argLen, uint8_t *args, int maxRespLen, uint8_t *resp)
{
	int cmdbuflen;
	uint8_t cmdbuf[256];

	uint32_t startTime;
	volatile bool packetReceived = false;
	int respLen = 0;

	cmdbuf[0] = 0x27;
	cmdbuf[1] = seq1;
	cmdbuf[2] = cmdLen >> 8;
	cmdbuf[3] = cmdLen;
	cmdbuf[4] = seq2 >> 8;
	cmdbuf[5] = seq2;
	cmdbuf[6] = cmd >> 8;
	cmdbuf[7] = cmd;
	memcpy (&cmdbuf[8], args, argLen);
	cmdbuflen = 8 + argLen;

/*
	for (int i = 0; i < cmdbuflen; i++) {
		Serial.printf ("%02x ", cmdbuf[i]);
	}
	Serial.printf ("\n\r");
*/

	// save start time so that we can time out in 1 second if no response received
	startTime = millis ();
	
	// packet not received
	packetReceived = false;

	// connect
	if (udp.connect (this->address, port)) {
		// parse response
		udp.onPacket([&packetReceived, &respLen, resp](AsyncUDPPacket packet) {
/*
            Serial.print("UDP Packet Type: ");
            Serial.print(
				packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Length: ");
            Serial.print(packet.length());
            Serial.print(", Data: ");
			for (int i = 0; i < packet.length(); i++) {
				Serial.printf ("%02x ", packet.data()[i]);
			}
            Serial.printf ("\n\r");
*/
			// copy packet data to response
			memcpy (resp, packet.data(), packet.length());

			// set response length
			respLen = packet.length();

			// exit loop
			packetReceived = true;
		});
		// send command
		udp.write (cmdbuf, cmdbuflen);
	}

	// wait for response or timeout
	while (!packetReceived && (millis() - startTime < 1000)) {
	}

	// close socket
	udp.close();

	return respLen;
}


bool DanteDevice::commandGetDeviceName (void)
{
	bool success = false;
	uint8_t args[2] = { 0x00, 0x00 };
	int respLen;
	uint8_t response[256];
	
	respLen = this->command (
		4440, 0xff, 0x000a, 0xffff, 0x1002, 2, args, sizeof (response), response);

	if (respLen) {
		this->name = (char *)&response[10];
	} else {
		Serial.printf ("DanteDevice::commandGetDeviceName failed\n\r");
	}

	return success;
}
