#ifndef __PERIODIC_h__
#define __PERIODIC_h__

#include <stdio.h>
#include <signal.h>
//#include <errno.h>
#include <time.h>
//#include <pthread.h>
//#include <unistd.h>

static int make_periodic (int unsigned period, struct periodic_info *info);
static void wait_period (struct periodic_info *info);

#endif