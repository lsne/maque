
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

#include "ae.h"     /* Event driven programming library */
#include "sds.h"    /* Dynamic safe strings */
#include "dict.h"   /* Hash tables */
#include "adlist.h" /* Linked lists */
#include "zmalloc.h" /* total memory usage aware version of malloc/free */
#include "anet.h"   /* Networking the easy way */
#include "zipmap.h" /* Compact string -> string data structure */
#include "ziplist.h" /* Compact list data structure */
#include "intset.h" /* Compact integer set structure */
#include "version.h"

struct maqueServer {
    // pthread_t mainthread;
    int port;
    char *bindaddr;
    char *unixsocket;
    int ipfd;
    int sofd;
    int dbnum;
    int daemonize;
    int appendonly;
    int appendfd;
    int appendseldb;
    char *pidfile;
    pid_t bgsavechildpid;
    pid_t bgrewritechildpid;
    char *logfile;
    int syslog_enabled;
    char *syslog_ident;
    int syslog_facility;
};