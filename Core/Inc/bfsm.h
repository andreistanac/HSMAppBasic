/*
 * bfsm.h
 *
 *  Created on: Oct 27, 2022
 *      Author: worker
 */

#ifndef INC_BFSM_H_
#define INC_BFSM_H_

#include "hfsm.h"
#include "bfsm.h"

enum {
    K0_SIG = Q_USER_SIG,
	K1_SIG,
	K2_SIG,
	K3_SIG,
	TIMER_SIG
};

typedef struct {

    QHsm super;

    uint16_t counter;
    uint16_t timer;

    uint8_t mode;
    // ...
} bFSM_t;

void bFSM_ctor(void);

extern QHsm * const pbFSM;

#endif /* INC_BFSM_H_ */
