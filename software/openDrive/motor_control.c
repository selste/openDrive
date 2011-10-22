#include <stdio.h>

#include "test.h"

#include "timespec.h"

#define ABS(x) (((x)<0)?(-(x)):(x))

typedef struct wait_queue {
	int wait;
} wait_queue;

typedef struct MotorPosition {
  struct time_spec  time;
  unsigned long int pos;
  long int spd;
} MotorPosition;

struct Motor;

typedef struct MotorDriver {
  int (*getPosition)(struct Motor *,struct MotorPosition *);
  int (*doRamp)(struct Motor *,struct MotorPosition *, struct MotorPosition *);
  int (*setSpeed)(struct Motor *, unsigned long speed);
} MotorDriver;

typedef struct Motor {
	int id;
	struct MotorDriver *driver;
	struct wait_queue wq;
	struct time_spec nxt;	
	struct MotorPosition *curr;
	struct MotorPosition *trgt;
	unsigned long int min_accel; // How much can we change speed in a single step
	unsigned long int inactive:1;
} Motor;

void usleep(int x) { }
int genericDoRamp(struct Motor *motor,
		  struct MotorPosition *from, struct MotorPosition *to) { }

static inline void
wakeUp(struct wait_queue *wq) 
{

}

static inline void
waitForNextUpdate(struct wait_queue *wq, struct time_spec *when)
{
	usleep(1000000);
}

void
motorSetTarget(struct Motor *motor, struct MotorPosition *trgt) {
	wakeUp(&motor->wq);
}

static inline int
updateMotor(struct Motor *motor) {
	int rc = 0;
	struct time_spec now = {0,};
	long int need_speed;
	struct time_spec help;
	/* Lets update our current position */
	if (motor->driver->getPosition) {
		motor->driver->getPosition(motor, motor->curr);
	} else {
		timespec_sub(&help,&now,&motor->curr->time);
		/* No motor driver information: Let's guess */
		motor->curr->pos += motor->curr->spd * help.usecs;
		motor->curr->time = now;
	}

	timespec_sub(&help,&motor->trgt->time,&motor->curr->time);
	need_speed = (motor->trgt->pos - motor->curr->pos) / help.usecs;

	if (motor->trgt == NULL ) {
		// no target, just do, what we are doing
	} else /* okay, we have a target */
		if ( (ABS(need_speed - motor->trgt->spd) < motor->min_accel)
		    && (ABS(need_speed - motor->curr->spd) < motor->min_accel)) {
		// needed speed is not too different from current and target
		// just set it.
		motor->driver->setSpeed(motor,need_speed);
		motor->nxt = motor->trgt->time;
	} else if ( motor->driver->doRamp 
		   && motor->driver->doRamp(motor, motor->curr, motor->trgt) == 0) {
		motor->nxt = motor->trgt->time;
	} else if ( genericDoRamp(motor, motor->curr, motor->trgt) == 0) {
		motor->nxt = motor->trgt->time;
	} else {
		rc = 1;
	}
	return rc;
}

static void
motorTask(struct Motor *motor) {
	do {
		if (updateMotor(motor) == 0) {
			waitForNextUpdate(&motor->wq,&motor->nxt);
		} else {
			printf("Did not update Motor %d - marking it inactive\n",motor->id);
			break;
		}
	} while (!motor->inactive);
	motor->inactive = 1;
	return;
}

#ifdef TEST_MODE
struct time_spec e0u1k = { 0, 1000 };
struct time_spec e0u2k = { 0, 2000 };
struct time_spec e0um1k = { 0, -1000 };

TEST_PREFIX
        RUN_TEST(unequal,{1==0;},0);
        RUN_TEST(must_fail,{1==0;},1);
        RUN_TEST(equal,{1==1;},1);
	RUN_TEST(timespec1,{ struct time_spec help; 
		timespec_sub(&help,&e0u1k,&e0u2k);
		help.epoch != -1 || help.usecs != 999000;},0);
TEST_SUFFIX
#endif
