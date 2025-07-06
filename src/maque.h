
#include "version.h"
#include "config.h"

#if defined(__sun)
#include "solarisfixes.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>

typedef struct {
    // pthread_t mainthread;
    int ipfd;
    int sofd;
} maqueServer;

extern maqueServer server;

extern char *getMaqueInfoString(void);