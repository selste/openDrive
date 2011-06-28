#ifndef TIMESPEC_H
#define TIMESPEC_H

typedef struct time_spec {
	char epoch;
	long usecs;
} time_spec;

static inline void
timespec_sub(struct time_spec *tgt,
	     struct time_spec *op1,
	     struct time_spec *op2) {
  tgt->epoch = 0;
  tgt->usecs = op1->usecs - op2->usecs;
  while (tgt->usecs < 0) {
    tgt->usecs += 1000000;
    tgt->epoch -=1;
  } 
  tgt->epoch += (op1->epoch - op2->epoch);
}
#endif /* TIMESPEC_H */
