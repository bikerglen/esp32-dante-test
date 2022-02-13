class DanteRxChannel
{
	friend class DanteDevice;

	uint16_t rxChanNum;
	String rxChanName;
	String txDevName;
	String txChanName;
};

class DanteDevice
{
	friend class DanteDeviceList;

	public:

		DanteDevice (String server, IPAddress address, uint32_t timeToLive);
		~DanteDevice (void);

		void updateAddress (IPAddress address, uint32_t timeToLive);

		void setMissing (void) { this->missing = true; };
		bool getMissing (void) { return this->missing; };
		void setNew (bool state) { this->isNew = state; };
		bool getNew (void) { return this->isNew; };

		void incrementSubscriptionChanges (void) { this->subscriptionChanges++; };
		void decrementSubscriptionChanges (void) { this->subscriptionChanges--; };

		bool populateDeviceInfo (void);

		String getServer (void);
		String getName (void);
		void getChannelCounts (int *tx, int *rx);
		String getSubscriptions (String prefix, String suffix);
	
		// connect a channel on this receive device to a a channel on a transmit device
		// TODO void connect (int rxchan, DanteDevice *txdevice, int txchan);

	private:

		int command (uint16_t port, uint8_t seq1, uint16_t cmdLen, uint16_t seq2,
			uint16_t cmd, int argLen, uint8_t *args, int maxRespLen, uint8_t *resp);

		bool commandGetDeviceName (void);
		bool commandGetChannelCounts (void);
		bool commandGetSubscriptions (void);

		String    server;		// server name from A record
		IPAddress address;		// ip address from A record
		uint32_t  updated;		// update time in milliseconds
		uint32_t  timeToLive;	// time to live in seconds
		bool      missing;      // device disappeared on a scan
		bool      isNew;		// new device, needs name, chan counts, subs populated

		// This starts at zero. For every multicast subscription change notififcation received 
		// for the device, this is incremented by 1. When it's non-zero, 
		// commandGetSubscriptions should be called and the value decremented by 1. Counter is 
		// needed instead of a simple flag because a change could cause 4 updates in a row 
		// on 2 channel device, e.g. unsubscribe ch1, subscribe ch1, unsubscribe ch2, 
		// subscribe ch2, in rapid succession and inc/dec functions are likely on separate
		// threads. Probably needs wrapping inc/dec functions and a semaphore to control 
		// access.  
		int       subscriptionChanges;

		String    name;
		bool      chanCountsValid;
		int       numTxChannels;
        int       numRxChannels;
		std::vector<DanteRxChannel *> rxChannels;

		AsyncUDP  udp;
};
