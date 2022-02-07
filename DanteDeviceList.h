class DanteDeviceList
{
	public:

		DanteDeviceList (void);
		~DanteDeviceList (void);
		
		bool begin (void);
		void scan (void);

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
