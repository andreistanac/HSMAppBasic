/*
 * util.c
 *
 *  Created on: Nov 6, 2022
 *      Author: worker
 */

#include <stdlib.h>
#include "util.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "bfsm.h"

static void Digit_Update(uint16_t data);

static const uint8_t digits[] = {
		0x7E, 0x50, 0x6D, 0x79, 0x53, 0x3B, 0x3F, 0x70, 0x7F, 0x7B
};

static uint16_t digit_state = 0x00;

uint32_t timer_key2;
uint32_t timer_key3;

unsigned int timer_enc = 0;

void Digit_Init(void) {
	Digit_Update(digit_state);
}

void Digit_Number(uint16_t number) {
	digit_state = ((uint16_t)digits[(number/10)%10]<<8) | digits[number % 10];
	Digit_Update(digit_state);
}

void Digit_NumberPos(uint8_t number, uint8_t pos) {
	switch(pos) {
		case 0:
			digit_state &= 0xFF00;
			digit_state |= digits[number%10];
			break;
		case 1:
			digit_state &= 0x00FF;
			digit_state |= (((uint16_t)digits[number%10])<<8);
			break;
	}

	Digit_Update(digit_state);
}

void Digit_SymolPos(uint8_t value, uint8_t pos) {
	switch(pos) {
		case 0:
			digit_state &= 0xFF00;
			digit_state |= value;
			break;
		case 1:
			digit_state &= 0x00FF;
			digit_state |= ((uint16_t)value)<<8;
			break;
	}
	Digit_Update(digit_state);
}

static void Digit_Update(uint16_t data) {
	uint32_t  j = 16;

	// negative reset pulse
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);

	while (j--) { // while(data) -- BAD!!!!

		  // set data bit
		  if (data & 0x1) {
			  HAL_GPIO_WritePin(SDI_GPIO_Port, SDI_Pin, GPIO_PIN_SET);
		  } else {
			  HAL_GPIO_WritePin(SDI_GPIO_Port, SDI_Pin, GPIO_PIN_RESET);
		  }

		  // clock pulse
		  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);

		  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);

		  data >>= 1;
	}

	// positive update (write) pulse
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

QEvent Event_Update(void) {

	static uint8_t key0 = 1, key1 = 1, key2 = 1, key3 = 1;
	static uint8_t key0_prev = 1, key1_prev = 1 , key2_prev = 1, key3_prev = 1;

	extern uint32_t timer;

	QEvent e;

	e.sig = NO_EVENT;

	key0 = HAL_GPIO_ReadPin(K0_GPIO_Port, K0_Pin);
	key1 = HAL_GPIO_ReadPin(K1_GPIO_Port, K1_Pin);
	// key2 = HAL_GPIO_ReadPin(K2_GPIO_Port, K2_Pin);
	// key3 = HAL_GPIO_ReadPin(K3_GPIO_Port, K3_Pin);

	key2 = HAL_GPIO_ReadPin(K2_GPIO_Port, K2_Pin);
	key3 = HAL_GPIO_ReadPin(K3_GPIO_Port, K3_Pin);

	if(key2 ^ key2_prev) {
		// if (key2 == GPIO_PIN_SET) {
			timer_key2 = timer_enc;
		// }
	} else if(key3 ^ key3_prev) {
		// if (key3 == GPIO_PIN_SET) {
			timer_key3 = timer_enc;
		// }
	}

	key2_prev = key2;
	key3_prev = key3;

	if(key0 ^ key0_prev) {
		if (key0 == GPIO_PIN_SET) {
			e.sig = K0_SIG;
		}
	} else if(key1 ^ key1_prev) {
		if (key1 == GPIO_PIN_SET) {
			e.sig = K1_SIG;
		}
	} /* else if(key2 ^ key2_prev) {
		if (key2 == GPIO_PIN_SET) {
			e.sig = K2_SIG;
		}
	} else if(key3 ^ key3_prev) {
		if (key3 == GPIO_PIN_SET) {
			e.sig = K3_SIG;
		}
	} */ else if (timer > 1000) {

		timer = 0;
		e.sig = TIMER_SIG;
	} else if (timer_key2 != 0 && abs(timer_key3 - timer_key2) < 22) {
		e.sig = (timer_key3 > timer_key2) + ROT_UP_SIG;
		/*
		if (timer_key3 > timer_key2) {
			e.sig = ROT_UP_SIG;
		} else {
			e.sig = ROT_DN_SIG;
		}
		*/
		timer_key2 = 0;
		timer_key3 = 0;
	}

	key0_prev = key0;
	key1_prev = key1;
	// key2_prev = key2;
	// key3_prev = key3;

	return e;
}
