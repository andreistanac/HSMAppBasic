/*
 * util.h
 *
 *  Created on: Nov 6, 2022
 *      Author: worker
 */

#ifndef INC_UTIL_H_
#define INC_UTIL_H_

#include <stdint.h>
#include "hfsm.h"

#define NO_EVENT 	((uint16_t)255)

#define DIGIT_n		0x15

void Digit_Init(void);
void Digit_Number(uint16_t number);
void Digit_NumberPos(uint8_t number, uint8_t pos);
void Digit_SymolPos(uint8_t value, uint8_t pos);

QEvent Event_Update(void);

#endif /* INC_UTIL_H_ */
