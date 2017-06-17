#ifndef __PERIODIC_H__
#define __PERIODIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netdb.h> 
//#include <arpa/inet.h>		// inet_aton
//#include <pthread.h>

#include <signal.h>
#include <errno.h>
#include <time.h>

struct periodic_info
{
	int sig;
	sigset_t alarm_sig;
};

static int make_periodic (int unsigned period, struct periodic_info *info);
static void wait_period (struct periodic_info *info);

#endif
