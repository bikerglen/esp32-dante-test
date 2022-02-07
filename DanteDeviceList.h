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

		// returns pointer to the device with the fully qualified name
		// e.g. AVIO-123.local
		// TODO -- use something safer like an Arduino String instead of c strings
		DanteDevice *searchfqn (uint8_t *name);

		// TODO -- returns pointer to the device with the short name
		// e.g. AVIO-123
		DanteDevice *search (String name);

		// returns the number of known devices
		int getDeviceCount (void);

	private:

		// returns true if any TTL's have expired
		bool checkUpdateNeeded (void);

		// check for missing devices and set them to missing
		void checkMissingDevices (void);

		// parse received mDNS packet
		void parsePacket (AsyncUDPPacket _packet);

		// parse mDNS domain name
		int parseDnsName (uint8_t *packet, int index, uint8_t *name, bool reference);

		AsyncUDP udp;
		std::vector<DanteDevice *> devices;

		uint32_t lastScanTime;
		uint32_t scanState;
		uint32_t lastScanCheckTime;
		uint32_t randomScanDelay;
};
