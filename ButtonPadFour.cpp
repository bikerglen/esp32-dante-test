#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <Wire.h>

#include "ButtonPadFour.h"

// in=[0:255]; round((in/255).^2.8*4095) 
static const uint16_t led_gamma_12b_2p8[256] = {
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     1,     1,     1,     1,     1,
       2,     2,     2,     3,     3,     4,     4,     5,     5,     6,     7,     8,     8,     9,    10,    11,
      12,    13,    15,    16,    17,    18,    20,    21,    23,    25,    26,    28,    30,    32,    34,    36,    
      38,    40,    43,    45,    48,    50,    53,    56,    59,    62,    65,    68,    71,    75,    78,    82,    
      85,    89,    93,    97,   101,   105,   110,   114,   119,   123,   128,   133,   138,   143,   149,   154,   
     159,   165,   171,   177,   183,   189,   195,   202,   208,   215,   222,   229,   236,   243,   250,   258,   
     266,   273,   281,   290,   298,   306,   315,   324,   332,   341,   351,   360,   369,   379,   389,   399,   
     409,   419,   430,   440,   451,   462,   473,   485,   496,   508,   520,   532,   544,   556,   569,   582,   
     594,   608,   621,   634,   648,   662,   676,   690,   704,   719,   734,   749,   764,   779,   795,   811,   
     827,   843,   859,   876,   893,   910,   927,   944,   962,   980,   998,  1016,  1034,  1053,  1072,  1091,  
    1110,  1130,  1150,  1170,  1190,  1210,  1231,  1252,  1273,  1294,  1316,  1338,  1360,  1382,  1404,  1427,  
    1450,  1473,  1497,  1520,  1544,  1568,  1593,  1617,  1642,  1667,  1693,  1718,  1744,  1770,  1797,  1823,  
    1850,  1877,  1905,  1932,  1960,  1988,  2017,  2045,  2074,  2103,  2133,  2162,  2192,  2223,  2253,  2284,  
    2315,  2346,  2378,  2410,  2442,  2474,  2507,  2540,  2573,  2606,  2640,  2674,  2708,  2743,  2778,  2813,  
    2849,  2884,  2920,  2957,  2993,  3030,  3067,  3105,  3143,  3181,  3219,  3258,  3297,  3336,  3376,  3416,  
    3456,  3496,  3537,  3578,  3619,  3661,  3703,  3745,  3788,  3831,  3874,  3918,  3962,  4006,  4050,  4095
};


ButtonPadFour::ButtonPadFour (void)
{
}


ButtonPadFour::~ButtonPadFour (void)
{
}


void ButtonPadFour::begin (TwoWire *wire, uint8_t numButtons)
{
	// save number of buttons
	this->numButtons = numButtons;

	// save i2c interface
	this->wire = wire;

	// configure PCA9685 PWM driver #1
	if (numButtons > 0) {
		this->initPCA9685 (BP48_PCA9685_I2C_ADDRESS_0);
	}

	// configure PCA9685 PWM driver #2
	if (numButtons > 4) {
		this->initPCA9685 (BP48_PCA9685_I2C_ADDRESS_1);
	}

	// configure PCA9535 I2C IO expander
	this->initPCA9535 (BP48_PCA9535_I2C_ADDRESS);

	// initialize button masks
	if (numButtons == 4) {
		this->buttonMasks[0] = 0x0800;
		this->buttonMasks[1] = 0x0400;
		this->buttonMasks[2] = 0x0200;
		this->buttonMasks[3] = 0x0100;
	} else if (numButtons == 8) {
		this->buttonMasks[0] = 0x0400;
		this->buttonMasks[1] = 0x0200;
		this->buttonMasks[2] = 0x0800;
		this->buttonMasks[3] = 0x0100;
		this->buttonMasks[4] = 0x0010;
		this->buttonMasks[5] = 0x0080;
		this->buttonMasks[6] = 0x0040;
		this->buttonMasks[7] = 0x0020;
	}

	// initialize button states
	for (int i = 0; i < BP48_MAX_BUTTONS; i++) {
		this->buttonStates[i] = 0;
		this->buttonDownEvents[i] = false;
	}
}



uint8_t ButtonPadFour::tick (void)
{
	uint8_t retval = 0;

    // debounce pushbuttons
    this->DebounceButtons (BP48_PCA9535_I2C_ADDRESS);

    // return button presses as a single byte with bits set to '1' for any new presses
    for (int i = 0; i < this->numButtons; i++) {
        if (buttonDownEvents[i]) {
            buttonDownEvents[i] = false;
			retval |= (1 << i);
        }
    }

	return retval;
}


void ButtonPadFour::setButtonColor (uint8_t which, uint8_t r, uint8_t g, uint8_t b)
{
	// TODO
	switch (which) {
		case 0: 
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW1_PWM_RED_CH, led_gamma_12b_2p8[r]);
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW1_PWM_GRN_CH, led_gamma_12b_2p8[g]);
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW1_PWM_BLU_CH, led_gamma_12b_2p8[b]);
			break;
		case 1: 
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW2_PWM_RED_CH, led_gamma_12b_2p8[r]);
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW2_PWM_GRN_CH, led_gamma_12b_2p8[g]);
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW2_PWM_BLU_CH, led_gamma_12b_2p8[b]);
			break;
		case 2: 
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW3_PWM_RED_CH, led_gamma_12b_2p8[r]);
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW3_PWM_GRN_CH, led_gamma_12b_2p8[g]);
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW3_PWM_BLU_CH, led_gamma_12b_2p8[b]);
			break;
		case 3: 
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW4_PWM_RED_CH, led_gamma_12b_2p8[r]);
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW4_PWM_GRN_CH, led_gamma_12b_2p8[g]);
			SetPWMInv (BP48_PCA9685_I2C_ADDRESS_0, BP4_SW4_PWM_BLU_CH, led_gamma_12b_2p8[b]);
			break;
	}
}


void ButtonPadFour::initPCA9685 (uint8_t address)
{
    // place PCA9685 into sleep and enable auto address increment
    this->wire->beginTransmission (address);
    this->wire->write (BP48_MODE1);
    this->wire->write (0x30);
    this->wire->endTransmission (true);

    // update on stop and open drain
    this->wire->beginTransmission (address);
    this->wire->write (BP48_MODE2);
    this->wire->write (0x00);
    this->wire->endTransmission (true);

    // set for maximum PWM frequency
    this->wire->beginTransmission (address);
    this->wire->write (BP48_PRESCALE);
    this->wire->write (0x02);
    this->wire->endTransmission (true);

    // take PCA9685 out of sleep and enable auto address increment
    this->wire->beginTransmission (address);
    this->wire->write (BP48_MODE1);
    this->wire->write (0x20);
    this->wire->endTransmission (true);
    
    // clear all outputs
    for (uint8_t channel = 0; channel < 16; channel++) {
        SetPWMInv (address, channel, 0);
    }
}


void ButtonPadFour::initPCA9535 (uint8_t address)
{
    this->wire->beginTransmission (address);
    this->wire->write (0x04);
    this->wire->write (0x00); // no inversion
    this->wire->endTransmission (true);

    this->wire->beginTransmission (address);
    this->wire->write (0x05);
    this->wire->write (0x00); // no inversion
    this->wire->endTransmission (true);

    this->wire->beginTransmission (address);
    this->wire->write (0x06);
    this->wire->write (0xff); // all inputs
    this->wire->endTransmission (true);

    this->wire->beginTransmission (address);
    this->wire->write (0x06);
    this->wire->write (0xff); // all inputs
    this->wire->endTransmission (true);
}


void ButtonPadFour::DebounceButtons (uint8_t address)
{
	uint8_t tmp;
	uint16_t readData = 0;

	if (this->numButtons > 0) {
		this->wire->beginTransmission (address);
		this->wire->write (0x01); // port 1 input data register, buttons 1 to 4
		this->wire->endTransmission (false);
		this->wire->requestFrom ((uint8_t)address, (size_t)1, (bool)true); 
		tmp = this->wire->read ();
		readData |= tmp << 8; // save in high byte
	}

	if (this->numButtons > 4) {
		this->wire->beginTransmission (address);
		this->wire->write (0x00); // port 0 input data register, buttons 5 to 8
		this->wire->endTransmission (false);
		this->wire->requestFrom ((uint8_t)address, (size_t)1, (bool)true); 
		tmp = this->wire->read ();
		readData |= tmp; // save in low byte
	}

    for (int i = 0; i < this->numButtons; i++) {

		uint8_t buttonDown;

		buttonDown = (readData & this->buttonMasks[i]) ? 0 : 1;

        switch (buttonStates[i]) {
            case 0:
                buttonStates[i] = buttonDown ? 1 : 0;
                break;
            case 1:
                if (buttonDown) {
                    buttonStates[i] = 2;
                    buttonDownEvents[i] = true;
                } else {
                    buttonStates[i] = 0;
                }
                break;
            case 2:
                buttonStates[i] = buttonDown ? 2 : 3;
                break;
            case 3:
                buttonStates[i] = buttonDown ? 2 : 0;
                break;
            default:
                buttonStates[i] = 0;
                break;
        }
    }
}


void ButtonPadFour::SetPWM (uint8_t address, uint8_t channel, uint16_t v)
{
    v &= 0xFFF;
    if (v == 4095) {
        // fully on
        SetPWMRaw (address, channel, 4096, 0);
    } else if (v == 0) {
        // fully off
        SetPWMRaw (address, channel, 0, 4096);
    } else {
        SetPWMRaw (address, channel, 0, v);
    }
}


void ButtonPadFour::SetPWMInv (uint8_t address, uint8_t channel, uint16_t v)
{
    v &= 0xFFF;
    v = 4095 - v;
    if (v == 4095) {
        // fully on
        SetPWMRaw (address, channel, 4096, 0);
    } else if (v == 0) {
        // fully off
        SetPWMRaw (address, channel, 0, 4096);
    } else {
        SetPWMRaw (address, channel, 0, v);
    }
}


void ButtonPadFour::SetPWMRaw (uint8_t address, uint8_t channel, uint16_t on, uint16_t off)
{
    this->wire->beginTransmission (address);
    this->wire->write (BP48_LED0_ON_L + 4 * channel);
    this->wire->write (on);
    this->wire->write (on >> 8);
    this->wire->write (off);
    this->wire->write (off >> 8);
    this->wire->endTransmission (true);
}
