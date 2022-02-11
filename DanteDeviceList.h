class DanteDeviceList
{
	public:

		DanteDeviceList (void);
		~DanteDeviceList (void);
		
		// create mDNS listener on port 5353 and capture _netaudio_arc responses
		bool begin (void);

		// call this once from setup once network connection is established
		// send a _netaudio_arc mDNS request
		void scan (void);

		// call this repetitively from loop
		// check if time to live has expired and perform a scan if needed
		void scanIfNeeded (void);

		// call this repetitively from loop
		// populates name, channel counts, and subscription info for any new devices
		void populateNewDevices (void);

		// returns pointer to the device with the server name
		DanteDevice *searchServer (String server);

		// returns the number of known devices
		int getDeviceCount (void);

		// list known devices to Serial
		void listDevices (void);

	private:

		// parse received mDNS packet
		void parsePacket (AsyncUDPPacket _packet);

		// parse mDNS domain name
		int parseDnsName (uint8_t *packet, int index, uint8_t *name, bool reference);

		// returns true if any TTL's have expired
		bool checkUpdateNeeded (void);

		// check for missing devices and set them to missing
		void checkMissingDevices (void);

		AsyncUDP udp;
		std::vector<DanteDevice *> devices;

		uint32_t lastScanTime;
		uint32_t scanState;
		uint32_t lastScanCheckTime;
		uint32_t randomScanDelay;
};
