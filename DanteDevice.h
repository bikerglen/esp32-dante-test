class DanteDevice
{
	friend class DanteDeviceList;

	public:

		DanteDevice (String server, IPAddress address, uint32_t timeToLive);
		~DanteDevice (void);

		void updateAddress (IPAddress address, uint32_t timeToLive);

		void setMissing (void);
		bool getMissing (void);

		String getServer (void);
		String getName (void);
		void getChannelCounts (int *tx, int *rx);
	
		// connect a channel on this receive device to a a channel on a transmit device
		// TODO void connect (int rxchan, DanteDevice *txdevice, int txchan);

	private:

		int command (uint16_t port, uint8_t seq1, uint16_t cmdLen, uint16_t seq2,
			uint16_t cmd, int argLen, uint8_t *args, int maxRespLen, uint8_t *resp);

		bool commandGetDeviceName (void);
		bool commandGetChannelCounts (void);

		String    server;		// server name from A record
		IPAddress address;		// ip address from A record
		uint32_t  updated;		// update time in milliseconds
		uint32_t  timeToLive;	// time to live in seconds
		bool      missing;      // device disappeared on a scan
		bool      isNew;		// TODO -- use this flag to scan each found 
								// TODO -- device for name, num channels, subscriptions

		String    name;
		bool      chanCountsValid;
		int       numTxChannels;
        int       numRxChannels;

		AsyncUDP  udp;
};
