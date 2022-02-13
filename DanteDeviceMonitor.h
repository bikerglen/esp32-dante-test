class DanteDeviceMonitor
{
	public:

		DanteDeviceMonitor (void);
		~DanteDeviceMonitor (void);
		
		// create listener for multicast packets on address 224.0.0.231 to port 8702
		bool begin (void);

		// call this from main loop to get dante devices that have had their
		// configuration changed
		DanteDevice *changed (DanteDeviceList *devices);

	private:

		// parse received mDNS packet
		void parsePacket (AsyncUDPPacket _packet);

		AsyncUDP udp;
		RingBuf *pbuff;
};
