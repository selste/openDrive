#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "timespec.h"
#include "motor.h"

typedef enum { FALSE = 0, TRUE } boolean_t;
typedef signed long long int position_t;
typedef signed long int speed_t;

typedef struct position {
  struct time_spec time;
  position_t pos;
  speed_t spd;
} position;

typedef union callback_data {
  enum { NONE } event;
  void *private;
} callback_data;

typedef void (*callback_fn)(void *, union callback_data *);

typedef struct motor_driver {
  /* mandatory elements */
  void             (*getPosition)     (struct motor *,struct position *);
  struct time_spec (*moveToPosition)  (struct motor *, 
				       struct position *src, 
				       struct position *tgt, 
				       boolean_t simul);
  struct time_spec (*accelerateTo)    (struct motor *,speed_t src, 
				       speed_t tgt, boolean_t simul);
  int              (*setSpeed)        (struct motor *,speed_t);
  /* optional element */
  int              (*registerCallback)(struct motor *, callback_fn);
} motor_driver;

#endif /* MOTOR_DRIVER_H */
