#define NUM_BUTTONS           6

// PCA9685 defines
#define PCA9685_I2C_ADDRESS_0 0x40
#define PCA9685_I2C_ADDRESS_1 0x41
#define MODE1                 0x00
#define MODE2                 0x01
#define LED0_ON_L             0x06
#define PRESCALE              0xFE

// SW2, SW4, SW6 are on U1 / PCA9685_I2C_ADDRESS_0
#define SW2_PWM_RED_CH        13
#define SW2_PWM_GRN_CH        12
#define SW2_PWM_BLU_CH        14

#define SW4_PWM_RED_CH         9
#define SW4_PWM_GRN_CH         8
#define SW4_PWM_BLU_CH        10

#define SW6_PWM_RED_CH         7
#define SW6_PWM_GRN_CH         5
#define SW6_PWM_BLU_CH         6

// SW1, SW3, SW5 are on U2 / PCA9685_I2C_ADDRESS_1
#define SW1_PWM_RED_CH         0
#define SW1_PWM_GRN_CH         1
#define SW1_PWM_BLU_CH         2

#define SW3_PWM_RED_CH         4
#define SW3_PWM_GRN_CH         5
#define SW3_PWM_BLU_CH         6

#define SW5_PWM_RED_CH         8
#define SW5_PWM_GRN_CH         9
#define SW5_PWM_BLU_CH        10


class ButtonPadSix
{
	public:

		ButtonPadSix (void);
		~ButtonPadSix(void);
		void begin (
			int sw1pin, int sw2pin, int sw3pin, int sw4pin, int sw5pin, int sw6pin, 
			TwoWire *wire);
		uint8_t tick (void);
		void setButtonColor (uint8_t which, uint8_t r, uint8_t g, uint8_t b);

	private:

		void initPCA9685 (uint8_t address);
		void DebounceButtons (void);
		void SetPWM (uint8_t address, uint8_t channel, uint16_t v);
		void SetPWMInv (uint8_t address, uint8_t channel, uint16_t v);
		void SetPWMRaw (uint8_t address, uint8_t channel, uint16_t on, uint16_t off);

		TwoWire *wire;
		uint8_t buttonPins[NUM_BUTTONS];
		uint8_t buttonStates[NUM_BUTTONS];
		bool    buttonDownEvents[NUM_BUTTONS];
};
