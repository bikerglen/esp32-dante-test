#define BP4_NUM_BUTTONS           6

// PCA9685 defines
#define BP4_PCA9685_I2C_ADDRESS   0x40
#define BP4_MODE1                 0x00
#define BP4_MODE2                 0x01
#define BP4_LED0_ON_L             0x06
#define BP4_PRESCALE              0xFE

// PCA9535 defines
#define BP4_PCA9535_I2C_ADDRESS   0x20

// button pins
// TODO

// LED pins
#define BP4_SW1_PWM_RED_CH         2
#define BP4_SW1_PWM_GRN_CH         1
#define BP4_SW1_PWM_BLU_CH         3

#define BP4_SW2_PWM_RED_CH         6
#define BP4_SW2_PWM_GRN_CH         5
#define BP4_SW2_PWM_BLU_CH         7

#define BP4_SW3_PWM_RED_CH         9
#define BP4_SW3_PWM_GRN_CH        10
#define BP4_SW3_PWM_BLU_CH        11

#define BP4_SW4_PWM_RED_CH        14
#define BP4_SW4_PWM_GRN_CH        13
#define BP4_SW4_PWM_BLU_CH        15

class ButtonPadFour
{
	public:

		ButtonPadFour (void);
		~ButtonPadFour(void);
		void begin (TwoWire *wire);
		uint8_t tick (void);
		void setButtonColor (uint8_t which, uint8_t r, uint8_t g, uint8_t b);

	private:

		void initPCA9685 (uint8_t address);
		void initPCA9535 (uint8_t address);
		void DebounceButtons (uint8_t address);
		void SetPWM (uint8_t address, uint8_t channel, uint16_t v);
		void SetPWMInv (uint8_t address, uint8_t channel, uint16_t v);
		void SetPWMRaw (uint8_t address, uint8_t channel, uint16_t on, uint16_t off);

		TwoWire *wire;
		uint8_t buttonStates[BP4_NUM_BUTTONS];
		bool    buttonDownEvents[BP4_NUM_BUTTONS];
};
