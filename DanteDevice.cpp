#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <memory.h>
#include <vector>

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

	this->subscriptionChanges = 0;

	this->name = "";
	this->chanCountsValid = false;
	this->numTxChannels = 0;
	this->numRxChannels = 0;
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

	if (this->missing) {
		// no longer missing, mark as new so name, channel counts, 
		// and subscriptions get updated
		this->isNew = true;
	}

	this->address = address;
	this->timeToLive = timeToLive;
	this->updated = now;
	this->missing = false;
}

bool DanteDevice::populateDeviceInfo (void)
{
	bool success = true;

	this->commandGetDeviceName ();
	this->commandGetChannelCounts ();
	this->commandGetSubscriptions ();
	this->setNew (false);

	return success;
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

void DanteDevice::getChannelCounts (int *tx, int *rx)
{
	if (this->chanCountsValid == false) {
		this->commandGetChannelCounts ();
	}

	*tx = this->numTxChannels;
	*rx = this->numRxChannels;
}

String DanteDevice::getSubscriptions (String prefix, String suffix)
{
	String list = "";

	std::vector<DanteRxChannel *>::iterator it;
	for (it = this->rxChannels.begin(); it != this->rxChannels.end(); it++) {
		list += prefix;
		list += String ((*it)->rxChanNum) + ": ";
		list += this->name + "." + (*it)->rxChanName + " <= ";
		if ((*it)->txDevName == "" || (*it)->txChanName == "") {
			list += "<none>";
		} else {
			list += (*it)->txDevName + "." + (*it)->txChanName;
		}
		list += suffix;
	}

	return list;
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


bool DanteDevice::commandGetChannelCounts (void)
{
	bool success = false;
	uint8_t args[2] = { 0x00, 0x00 };
	int respLen;
	uint8_t response[256];
	
	respLen = this->command (
		4440, 0xff, 0x000a, 0xffff, 0x1000, 2, args, sizeof (response), response);

	if (respLen) {
		this->chanCountsValid = true;
		this->numTxChannels = (response[12] << 8) | response[13];
		this->numRxChannels = (response[14] << 8) | response[15];
	} else {
		Serial.printf ("DanteDevice::commandGetChannelCounts failed\n\r");
	}

	return success;
}


bool DanteDevice::commandGetSubscriptions (void)
{
	bool success = false;
	uint8_t args[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int respLen;
	uint8_t response[1536];

	if (this->name == "") {
		success = this->commandGetDeviceName ();
		if (!success) {
			Serial.printf ("DanteDevice::commandGetSubscriptions: failed\n\r");
			Serial.printf ("  unknown receiver name and get device name failed\n\r");
			return success;
		}
	}

	if (this->chanCountsValid == false) {
		success = this->commandGetChannelCounts ();
		if (!success) {
			Serial.printf ("DanteDevice::commandGetSubscriptions: failed\n\r");
			Serial.printf ("  channel counts not valid and get channel counts failed\n\r");
			return success;
		}
	}

	for (int page = 0; page < (this->numRxChannels+15)/16; page++) {
		args[0] = 0x00;
		args[1] = 0x00;
		args[2] = 0x00;
		args[3] = 0x01;
		args[4] = 0x00;
		args[5] = (page << 4) | 0x01;
		args[6] = 0x00;
		args[7] = 0x00;
		respLen = this->command (
			4440, 0xff, 0x0010, 0xffff, 0x3000, 8, args, sizeof (response), response);

		if (respLen) {

			for (int i = 0; i < respLen; i++) {
				Serial.printf ("%02x ", response[i]);
			}
			Serial.printf ("\n\r");

			int pktSlotCount = this->numRxChannels - page*16;
			if (pktSlotCount > 16) {
				pktSlotCount = 16;
			}

			for (int slot = 0; slot < pktSlotCount; slot++) {
				uint16_t chanNumber    = response[12 + 20*slot +  0] << 8; // rx channel number
				         chanNumber   |= response[12 + 20*slot +  1];
				uint16_t txChanOffset  = response[12 + 20*slot +  6] << 8; // tx chan offset
				         txChanOffset |= response[12 + 20*slot +  7];
				uint16_t txDevOffset   = response[12 + 20*slot +  8] << 8; // tx device offset
				         txDevOffset  |= response[12 + 20*slot +  9];
				uint16_t rxChanOffset  = response[12 + 20*slot + 10] << 8; // rx chan offset
				         rxChanOffset |= response[12 + 20*slot + 11];
				uint16_t status1       = response[12 + 20*slot + 12] << 8; // status 1
				         status1      |= response[12 + 20*slot + 13];
				uint16_t status2       = response[12 + 20*slot + 14] << 8; // status 2
				         status2      |= response[12 + 20*slot + 15];

				Serial.printf ("%04x %04x %04x %04x %04x %04x\n\r",
					chanNumber, txChanOffset, txDevOffset, rxChanOffset, status1, status2);

				String rxChannelName = (char *)&response[rxChanOffset];
				String txDeviceName = "";
				String txChannelName = "";

				if (txDevOffset != 0) {
					txDeviceName = (char *)&response[txDevOffset];
					if (txDeviceName == ".") {
						txDeviceName = this->name;
					}
				} else {
					txDeviceName = "";
				}

				if (txChanOffset != 0) {
					txChannelName = (char *)&response[txChanOffset];
				} else {
					txChannelName = "";
				}

				Serial.printf ("  %s.%s <= ", this->name.c_str(), rxChannelName.c_str());
				if (txDeviceName == "" || txChannelName == "") {
					Serial.printf ("<none>\n\r");
				} else {
					Serial.printf ("%s.%s\n\r", txDeviceName.c_str(), txChannelName.c_str());
				}

				// search for existing rx channel with this channel number
				std::vector<DanteRxChannel *>::iterator it;
				DanteRxChannel *rxChannel = NULL;
				for (it = this->rxChannels.begin(); it != this->rxChannels.end(); it++) {
					if ((*it)->rxChanNum == chanNumber) {
						rxChannel = *it;
						break;
					}
				}
				
				// not found so add new rx channel
				if (rxChannel == NULL) {
					Serial.printf ("adding new rx channel: %d\n\r", chanNumber);
					rxChannel = new DanteRxChannel ();
					rxChannel->rxChanNum = chanNumber;
					this->rxChannels.push_back (rxChannel);
				}

				// update channel information
				rxChannel->rxChanName = rxChannelName;
				rxChannel->txDevName =  txDeviceName;
				rxChannel->txChanName = txChannelName;
			}

			success = true;
		} else {
			Serial.printf ("DanteDevice::commandGetSubscriptions failed\n\r");
			break;
		}
	}

	return success;
}
