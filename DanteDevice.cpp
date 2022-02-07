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

DanteDevice::DanteDevice (uint8_t *name, IPAddress address, uint32_t timeToLive)
{
	uint32_t now = millis ();

	Serial.printf ("  adding new device\n\r");
	Serial.printf ("    name:       %s\n\r", name);
	Serial.printf ("    ip address: %d.%d.%d.%d\n\r",
										address[0], address[1], address[2], address[3]);
	Serial.printf ("    ttl:        %d\n\r", timeToLive);
	Serial.printf ("    now:        %d\n\r", now);
		
	this->name = (uint8_t *)malloc (strlen ((char *)name) + 1);
	memcpy (this->name, name, strlen ((char *)name) + 1);
	this->address = address;
	this->timeToLive = timeToLive;
	this->updated = now;
	this->missing = false;
}

DanteDevice::~DanteDevice (void)
{
}

void DanteDevice::updateAddress (IPAddress address, uint32_t timeToLive)
{
	uint32_t now = millis ();

	Serial.printf ("  updating device\n\r");
	Serial.printf ("    name:        %s\n\r", this->name);
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

char *DanteDevice::getName (void)
{	
	return (char *)name;
}
