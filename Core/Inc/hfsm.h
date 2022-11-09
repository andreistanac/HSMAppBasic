/*
 * hfsm.h
 *
 *  Created on: Oct 27, 2022
 *      Author: worker
 */
#ifndef INC_HFSM_H_
#define INC_HFSM_H_

#include "stdint.h"

#define QEP_MAX_NEST_DEPTH_ 16

#define Q_DEFINE_THIS_FILE
#define Q_DEFINE_THIS_MODULE(name_)
#define Q_ASSERT(test_) ((void)0)
#define Q_ALLEGE(test_) ((void)(test_))
#define Q_ERROR() ((void)0)

enum {
	QEP_EMPTY_SIG_,
	Q_ENTRY_SIG, /* signal for coding entry actions */
	Q_EXIT_SIG, /* signal for coding exit actions */
	Q_INIT_SIG, /* signal for coding initial transitions */

	Q_USER_SIG /* first signal that can be used in user code */
};

#ifndef QP_SIGNAL_SIZE
	#define QP_SIGNAL_SIZE 1
#endif

#if (QP_SIGNAL_SIZE == 1)
	typedef uint8_t QSignal;
#elif (QP_SIGNAL_SIZE == 2)
	typedef uint16_t QSignal;
#elif (QP_SIGNAL_SIZE == 4)
	typedef uint32_t QSignal;
#else
	#error "QP_SIGNAL_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

typedef struct QEventTag {
	QSignal sig; /* signal of the event */
	uint8_t dynamic_; /* attributes of a dynamic event (0 for static event) */
	/* add event parameters by derivation from the QEvent structure... */
} QEvent;

extern QEvent const QEP_reservedEvt_[];

typedef uint8_t QState; /* status returned from a state-handler function */

/* pointer to function type definition */
typedef 	QState /* return type */
			(*QStateHandler) /* name of the pointer-to-function type */
			(void *me, QEvent const *e); /* argument list */

typedef struct QHsmTag {
	QStateHandler state; /* current active state (state-variable) */
} QHsm;

#define Q_RET_HANDLED 	((QState)0)
#define Q_RET_IGNORED 	((QState)1)
#define Q_RET_TRAN 		((QState)2)
#define Q_RET_SUPER 	((QState)3)

#define Q_HANDLED() (Q_RET_HANDLED)
#define Q_IGNORED() (Q_RET_IGNORED)

#define Q_TRAN(target_) \
		(((QHsm *)me)->state = (QStateHandler)(target_), Q_RET_TRAN)

#define Q_SUPER(super_) \
		(((QHsm *)me)->state = (QStateHandler)(super_), Q_RET_SUPER)

/** helper macro to trigger reserved event in an HSM */
#define QEP_TRIG_(state_, sig_) \
		((*(state_))(me, &QEP_reservedEvt_[sig_]))

/** helper macro to trigger entry action in an HSM */
#define QEP_EXIT_(state_) \
		if (QEP_TRIG_(state_, Q_EXIT_SIG) == Q_RET_HANDLED) { \
			/* QS software tracing instrumentation for state exit */\
		}

/** helper macro to trigger exit action in an HSM */
#define QEP_ENTER_(state_) \
		if (QEP_TRIG_(state_, Q_ENTRY_SIG) == Q_RET_HANDLED) { \
			/* QS software tracing instrumentation for state entry */\
		}

#define Q_STATE_CAST(s) ((QStateHandler)(s))

#define QHsm_ctor(me_, initial_) ((me_)->state = (initial_))

void QHsm_init (QHsm *me, QEvent const *e);
void QHsm_dispatch(QHsm *me, QEvent const *e);
// uint8_t QHsm_isIn (QHsm *me, QHsmState state);
QState QHsm_top (QHsm *me, QEvent const *e);

#endif
