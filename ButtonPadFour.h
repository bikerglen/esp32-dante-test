// This class supports both my four and eight button button pads.
// Pass 4 or 8 to the numButtons argument of the begin method to
// select which button pad is connected.

// max number of buttons
#define BP48_MAX_BUTTONS           8

// PCA9685 defines
#define BP48_PCA9685_I2C_ADDRESS_0 0x40
#define BP48_PCA9685_I2C_ADDRESS_1 0x41
#define BP48_MODE1                 0x00
#define BP48_MODE2                 0x01
#define BP48_LED0_ON_L             0x06
#define BP48_PRESCALE              0xFE

// PCA9535 defines
#define BP48_PCA9535_I2C_ADDRESS   0x20

// BP4 LED pins
#define BP4_SW1_PWM_RED_CH         2 // always first and only pca9685
#define BP4_SW1_PWM_GRN_CH         1
#define BP4_SW1_PWM_BLU_CH         3

#define BP4_SW2_PWM_RED_CH         6 // always first and only pca9685
#define BP4_SW2_PWM_GRN_CH         5
#define BP4_SW2_PWM_BLU_CH         7

#define BP4_SW3_PWM_RED_CH         9 // always first and only pca9685
#define BP4_SW3_PWM_GRN_CH        10
#define BP4_SW3_PWM_BLU_CH        11

#define BP4_SW4_PWM_RED_CH        14 // always first and only pca9685
#define BP4_SW4_PWM_GRN_CH        13
#define BP4_SW4_PWM_BLU_CH        15

// BP8 LED pins
#define BP8_SW1_PWM_RED_CH         6 // first pca9685
#define BP8_SW1_PWM_GRN_CH         1 
#define BP8_SW1_PWM_BLU_CH         3 
                                        
#define BP8_SW2_PWM_RED_CH        15 // first pca9685
#define BP8_SW2_PWM_GRN_CH         9 
#define BP8_SW2_PWM_BLU_CH        11 
                                        
#define BP8_SW3_PWM_RED_CH        10 // second pca9685
#define BP8_SW3_PWM_GRN_CH        14 
#define BP8_SW3_PWM_BLU_CH        13 
                                        
#define BP8_SW4_PWM_RED_CH         2 // second pca9685
#define BP8_SW4_PWM_GRN_CH         7 
#define BP8_SW4_PWM_BLU_CH         5 
                                        
#define BP8_SW5_PWM_RED_CH         2  // first pca9685
#define BP8_SW5_PWM_GRN_CH         7  
#define BP8_SW5_PWM_BLU_CH         5  
                                         
#define BP8_SW6_PWM_RED_CH        10  // first pca9685
#define BP8_SW6_PWM_GRN_CH        14  
#define BP8_SW6_PWM_BLU_CH        13  
                                         
#define BP8_SW7_PWM_RED_CH        15  // second pca9685
#define BP8_SW7_PWM_GRN_CH         9  
#define BP8_SW7_PWM_BLU_CH        11  
                                         
#define BP8_SW8_PWM_RED_CH         6  // second pca9685
#define BP8_SW8_PWM_GRN_CH         1 
#define BP8_SW8_PWM_BLU_CH         3 

class ButtonPadFour
{
	public:

		ButtonPadFour (void);
		~ButtonPadFour(void);
		void begin (TwoWire *wire, uint8_t numButtons);
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
		uint8_t  numButtons;
		uint16_t buttonMasks[BP48_MAX_BUTTONS];
		uint8_t  buttonStates[BP48_MAX_BUTTONS];
		bool     buttonDownEvents[BP48_MAX_BUTTONS];
};
