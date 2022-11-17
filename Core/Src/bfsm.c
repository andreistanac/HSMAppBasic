/*
 * bfsm.c
 *
 *  Created on: Oct 27, 2022
 *      Author: worker
 */
#include "hfsm.h"
#include "bfsm.h"
#include "util.h"
#include "main.h"

static bFSM_t bFSM;

QHsm * const pbFSM = &bFSM.super;

static QState bFSM_initial(bFSM_t * const me, QEvent const * const e);

static QState bFSM_idle(bFSM_t * const me, QEvent const * const e);
static QState bFSM_counter0(bFSM_t * const me, QEvent const * const e);
static QState bFSM_counter1(bFSM_t * const me, QEvent const * const e);
static QState bFSM_timer(bFSM_t * const me, QEvent const * const e);
static QState bFSM_cnt_tmr(bFSM_t * const me, QEvent const * const e);

void bFSM_ctor(void) {
	bFSM_t *me = &bFSM;
    QHsm_ctor(&me->super, bFSM_initial);
}

QState bFSM_initial(bFSM_t * const me, QEvent const * const e) {
	Digit_Init();

	me->timer = 0;
	me->counter = 0;
	me->mode =0;

	return Q_TRAN(bFSM_idle);
}

QState bFSM_idle(bFSM_t * const me, QEvent const * const e) {
	int ret = Q_RET_IGNORED;
	switch(e->sig) {
		case Q_ENTRY_SIG:
			HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
			Digit_SymolPos(DIGIT_n, 1);
			Digit_NumberPos(me->mode, 0);
			ret = Q_RET_HANDLED;
			break;
		case K0_SIG:
			if(me->mode) {
				ret = Q_TRAN(bFSM_timer);
			} else {
				ret = Q_TRAN(bFSM_counter0);
			}
			break;
		case K1_SIG:
			me->mode ^= 1;
			Digit_NumberPos(me->mode, 0);
			ret = Q_RET_HANDLED;
			break;
		default:
			ret = Q_SUPER(QHsm_top);
			break;
	}
	return ret;
}

QState bFSM_counter0(bFSM_t * const me, QEvent const * const e) {
	int ret = Q_RET_IGNORED;
	switch(e->sig) {
		case Q_ENTRY_SIG:
			Digit_Number(me->counter);
			ret = Q_RET_HANDLED;
			break;
		case K2_SIG:
			ret = Q_TRAN(bFSM_counter1);
			break;
		case ROT_UP_SIG:
			me->counter++;
			Digit_Number(me->counter);
			ret = Q_RET_HANDLED;
			break;
		case ROT_DN_SIG:
			if (me->counter) me->counter--;
			Digit_Number(me->counter);
			ret = Q_RET_HANDLED;
			break;
		default:
			ret = Q_SUPER(bFSM_cnt_tmr);
			break;
	}
	return ret;
}

QState bFSM_counter1(bFSM_t * const me, QEvent const * const e) {
	int ret = Q_RET_IGNORED;
	switch(e->sig) {
		case K3_SIG:
			me->counter++;
			ret = Q_TRAN(bFSM_counter0);
			break;
		default:
			ret = Q_SUPER(bFSM_cnt_tmr);
			break;
	}
	return ret;
}

QState bFSM_timer(bFSM_t * const me, QEvent const * const e) {
	int ret = Q_RET_IGNORED;
	switch(e->sig) {
		case Q_ENTRY_SIG:
			// HAL_TIM_PWM_ConfigChannel(&htim4, sConfig, TIM_CHANNEL_3);

			TIM4->ARR = (me->counter % 2 == 0) ? 4000 : 5333;
			TIM4->CCR3 = ((me->counter % 2 == 0) ? 4000 : 5333 ) / 2;

			HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
			Digit_Number(me->timer);
			ret = Q_RET_HANDLED;
			break;
		case TIMER_SIG:
			me->timer++;
			Digit_Number(me->timer);
			ret = Q_RET_HANDLED;
			break;
		default:
			ret = Q_SUPER(bFSM_cnt_tmr);
			break;
	}
	return ret;
}

QState bFSM_cnt_tmr(bFSM_t * const me, QEvent const * const e) {
	int ret = Q_RET_IGNORED;
	switch(e->sig) {
		case K0_SIG:
			ret = Q_TRAN(bFSM_idle);
			break;
		default:
			ret = Q_SUPER(QHsm_top);
			break;
	}
	return ret;
}
