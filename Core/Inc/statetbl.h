
#include "stdint.h"

typedef struct EventTag {
	uint16_t sig; /* signal of the event */
	/* add event parameters by derivation from the Event structure */
} Event;

struct StateTableTag; /* forward declaration */

typedef void (*Tran)(struct StateTableTag *me, Event const *e);

typedef struct StateTableTag {
	Tran const *state_table; /* the State-Table */
	uint8_t n_states; /* number of states */
	uint8_t n_signals; /* number of signals */
	uint8_t state; /* the current active state */
	Tran initial; /* the initial transition */
} StateTable;

void StateTable_ctor(StateTable *me,
					Tran const *table, uint8_t n_states, uint8_t n_signals,
					Tran initial);

void StateTable_init(StateTable *me); /* init method */
void StateTable_dispatch(StateTable *me, Event const *e); /* dispatch method */
void StateTable_empty(StateTable *me, Event const *e); /* empty action */

/* macro for taking a state transition inside a transition function */
#define TRAN(target_) (((StateTable *)me)->state = (uint8_t)(target_))
