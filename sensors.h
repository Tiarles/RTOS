#ifndef __SENSORS_H__
#define __SENSORS_H__

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
//#include <pthread.h>
#include <unistd.h>

static void *readTemperatureSensor (void *arg);
static void *readGeolocalizationSensor (void *arg);
static void *readPressionSensor (void *arg);

#endif