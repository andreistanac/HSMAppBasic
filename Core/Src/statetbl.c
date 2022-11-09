#include "statetbl.h"
#include <assert.h>

void StateTable_ctor(StateTable *me,
					Tran const *table, uint8_t n_states, uint8_t n_signals,
					Tran initial) {
	me->state_table = table;
	me->n_states = n_states;
	me->n_signals = n_signals;
	me->initial = initial;
	me->state = n_states; /* initialize state out of range */
}

void StateTable_init(StateTable *me) {
	(*me->initial)(me, (Event *)0); /* top-most initial transition */
	assert(me->state < me->n_states); /* the initial tran. must change state */
}

void StateTable_dispatch(StateTable *me, Event const *e) {
	Tran t;
	assert(e->sig < me->n_signals); /* require the signal in range */
	t = me->state_table[me->state*me->n_signals + e->sig];
	(*t)(me, e); /* execute the transition function */
	assert(me->state < me->n_states); /* ensure that state stays in range */
}

void StateTable_empty(StateTable *me, Event const *e) {
	(void)me; /* void compiler warning about unused parameter */
	(void)e; /* void compiler warning about unused parameter */
}
