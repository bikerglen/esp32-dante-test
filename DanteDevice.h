class DanteDevice
{
	public:

		DanteDevice (uint8_t *name, IPAddress address, uint32_t timeToLive);
		~DanteDevice (void);

		void updateAddress (IPAddress address, uint32_t timeToLive);
	
		// connect a channel on this receive device to a a channel on a transmit device
		// TODO void connect (int rxchan, DanteDevice *txdevice, int txchan);

	public:

		uint8_t *name;			// fully qualified name, e.g. AVIO123.local
		IPAddress address;		// ip address
		uint32_t updated;		// update time in milliseconds
		uint32_t timeToLive;	// time to live in seconds
};
