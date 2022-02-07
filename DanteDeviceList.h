class DanteDeviceList
{
	public:

		DanteDeviceList (void);
		~DanteDeviceList (void);
		
		// create mDNS listener on port 5353 and capture _netaudio_arc responses
		bool begin (void);

		// send a _netaudio_arc mDNS request
		void scan (void);

		// check if time to live has expired and perform a scan if needed
		// TODO -- move code from main loop to here
		void scanIfNeeded (void);

		// TODO -- make private
		// returns the number of known devices
		int getDeviceCount (void);

		// TODO -- make private
		// returns true if any TTL's have expired
		bool checkUpdateNeeded (void);

		// TODO -- make private
		// check for missing devices and set them to missing
		void checkMissingDevices (void);

		// returns pointer to the device with the fully qualified name
		// e.g. AVIO-123.local
		// TODO -- use something safer like an Arduino String instead of c strings
		DanteDevice *searchfqn (uint8_t *name);

		// TODO -- returns pointer to the device with the short name
		// e.g. AVIO-123
		DanteDevice *search (String name);

	private:

		void parsePacket (AsyncUDPPacket _packet);
		int parseDnsName (uint8_t *packet, int index, uint8_t *name, bool reference);

		AsyncUDP udp;
		std::vector<DanteDevice *> devices;
};
