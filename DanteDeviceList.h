class DanteDeviceList
{
	public:

		DanteDeviceList (void);
		~DanteDeviceList (void);
		
		bool begin (void);

	private:

		void parsePacket (AsyncUDPPacket _packet);
		int parseDnsName (uint8_t *packet, int index, uint8_t *name, bool reference);


		AsyncUDP udp;
		std::vector<DanteDevice *> devices;

};
