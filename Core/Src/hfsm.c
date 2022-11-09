/*
 * hfsm.c
 *
 *  Created on: Oct 27, 2022
 *      Author: worker
 */
#include "hfsm.h"

QEvent const QEP_reservedEvt_[] = {
		{ (QSignal)QEP_EMPTY_SIG_, (uint8_t)0 },
		{ (QSignal)Q_ENTRY_SIG, (uint8_t)0 },
		{ (QSignal)Q_EXIT_SIG, (uint8_t)0 },
		{ (QSignal)Q_INIT_SIG, (uint8_t)0 }
};

QState QHsm_top(QHsm *me, QEvent const *e) {
	(void)me; /* avoid the compiler warning about unused parameter */
	(void)e; /* avoid the compiler warning about unused parameter */
	return Q_IGNORED(); /* the top state ignores all events */
}

void QHsm_init(QHsm *me, QEvent const *e) {
	QStateHandler t;

	/* the top-most initial transition must be taken */
	Q_ALLEGE((*me->state)(me, e) == Q_RET_TRAN);

	t = (QStateHandler)&QHsm_top; /* HSM starts in the top state */
	do { /* drill into the target... */
		QStateHandler path [QEP_MAX_NEST_DEPTH_];
		int8_t ip = (int8_t)0; /* transition entry path index */
		path [0] = me->state; /* save the target of the initial transition */
		(void)QEP_TRIG_(me->state, QEP_EMPTY_SIG_);
		while (me->state != t) {
			path [++ip] = me->state;
			(void)QEP_TRIG_(me->state, QEP_EMPTY_SIG_);
		}
		me->state = path [0]; /* restore the target of the initial tran. */
		/* entry path must not overflow */
		Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH_);
		do { /* retrace the entry path in reverse (desired) order... */
			QEP_ENTER_(path [ip]); /* enter path [ip] */
		} while ((--ip) >= (int8_t)0);
		t = path [0]; /* current state becomes the new source */
	} while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN);
	me->state = t;
}

void QHsm_dispatch(QHsm *me, QEvent const *e) {

	QStateHandler path [QEP_MAX_NEST_DEPTH_];
	QStateHandler s;
	QStateHandler t;
	QState r;

	t = me->state; /* save the current state */
	do { /* process the event hierarchically... */
		s = me->state;
		r = (*s)(me, e); /* invoke state handler s */
	} while (r == Q_RET_SUPER);

	if (r == Q_RET_TRAN) { /* transition taken? */

		int8_t ip = (int8_t)(-1); /* transition entry path index */
		int8_t iq; /* helper transition entry path index */

		path [0] = me->state; /* save the target of the transition */
		path [1] = t;
		while (t != s) { /* exit current state to transition source s... */
			if (QEP_TRIG_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {/*exit handled? */
				(void)QEP_TRIG_(t, QEP_EMPTY_SIG_); /* find superstate of t */
			}
			t = me->state; /* me->state holds the superstate */
		}


		t = path [0]; /* target of the transition */
		if (s == t) { /* (a) check source¼¼target (transition to self) */
			QEP_EXIT_(s) /* exit the source */
			ip = (int8_t)0; /* enter the target */
		} else {
			(void)QEP_TRIG_(t, QEP_EMPTY_SIG_); /* superstate of target */
			t = me->state;
			if (s == t) { /* (b) check source¼¼target->super */
				ip = (int8_t)0; /* enter the target */
			} else {
				(void)QEP_TRIG_(s, QEP_EMPTY_SIG_); /* superstate of src */
				/* (c) check source->super¼¼target->super */
				if (me->state == t) {
						QEP_EXIT_(s) /* exit the source */
						ip = (int8_t)0; /* enter the target */
				} else {
					/* (d) check source->super¼¼target */
					if (me->state == path[0]) {
						QEP_EXIT_(s) /* exit the source */
					} else {
						/* (e) check rest of source¼¼target->super->super..
						* and store the entry path along the way
						*/
						iq = (int8_t)0; /* indicate that LCA not found */
						ip = (int8_t)1; /* enter target and its superstate */
						path [1] = t; /* save the superstate of target */
						t = me->state; /* save source->super */
						/* find target->super->super */
						r = QEP_TRIG_(path [1], QEP_EMPTY_SIG_);
						while (r == Q_RET_SUPER) {
							path [++ip] = me->state; /* store the entry path */
							if (me->state == s) { /* is it the source? */
								iq = (int8_t)1; /* indicate that LCA found */
												/* entry path must not overflow */
								Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH_);

								--ip; /* do not enter the source */
								r = Q_RET_HANDLED; /* terminate the loop */
							} else { /* it is not the source, keep going up */
								r = QEP_TRIG_(me->state, QEP_EMPTY_SIG_);
							}
						}
						if (iq == (int8_t)0) { /* the LCA not found yet? */
								/* entry path must not overflow */
							Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH_);
							QEP_EXIT_(s) /* exit the source */
								/* (f) check the rest of source->super
								* ¼¼ target->super->super...
								*/
							iq = ip;
							r = Q_RET_IGNORED; /* indicate LCA NOT found */
							do {
								if (t == path [iq]) { /* is this the LCA? */
									r = Q_RET_HANDLED; /* indicate LCA found */
									ip = (int8_t)(iq - 1); /*do not enter LCA*/
									iq = (int8_t)(-1); /* terminate the loop */
								} else {
									--iq; /* try lower superstate of target */
								}
							} while (iq >= (int8_t)0);
							if (r != Q_RET_HANDLED) { /* LCA not found yet? */
								/* (g) check each source->super->...
								* for each target->super...
								*/
								r = Q_RET_IGNORED; /* keep looping */
								do {
									/* exit t unhandled? */
									if (QEP_TRIG_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {
										(void)QEP_TRIG_(t, QEP_EMPTY_SIG_);
									}
									t = me->state; /* set to super of t */
									iq = ip;
									do {
										if (t == path [iq]) { /* is this LCA? */
											/* do not enter LCA */
											ip = (int8_t)(iq - 1);
											iq = (int8_t)(-1); /*break inner */
											r = Q_RET_HANDLED; /*break outer */
										} else {
											--iq;
										}
									} while (iq >= (int8_t)0);
								} while (r != Q_RET_HANDLED);
							}
						}
					}
				}
			}
		}

		/* retrace the entry path in reverse (desired) order... */
		for (; ip >= (int8_t)0; --ip) {
			QEP_ENTER_(path [ip]) /* enter path [ip] */
		}
		t = path [0]; /* stick the target into register */

		me->state = t; /* update the current state */

		while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN) {
			ip = (int8_t)0;
			path [0] = me->state;
			(void)QEP_TRIG_(me->state, QEP_EMPTY_SIG_); /* find superstate */
			while (me->state != t) {
				path [++ip] = me->state;
				(void)QEP_TRIG_(me->state, QEP_EMPTY_SIG_); /* find superstate */
			}
			me->state = path [0];
			/* entry path must not overflow */
			Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH_);
			do { /* retrace the entry path in reverse (correct) order... */
				QEP_ENTER_(path [ip]) /* enter path [ip] */
			} while ((--ip) >= (int8_t)0);
			t = path [0];
		}
	}

	me->state = t; /* set new state or restore the current state */
}
