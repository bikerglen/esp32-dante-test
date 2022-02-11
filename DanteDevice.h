class DanteDevice
{
	public:

		DanteDevice (String server, IPAddress address, uint32_t timeToLive);
		~DanteDevice (void);

		void updateAddress (IPAddress address, uint32_t timeToLive);

		void setMissing (void);
		bool getMissing (void);

		String getServer (void);
	
		// connect a channel on this receive device to a a channel on a transmit device
		// TODO void connect (int rxchan, DanteDevice *txdevice, int txchan);

	public:

		String    server;		// server name from A record
		IPAddress address;		// ip address from A record
		uint32_t  updated;		// update time in milliseconds
		uint32_t  timeToLive;	// time to live in seconds
		bool      missing;      // device disappeared on a scan
};
