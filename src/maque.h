
#include "version.h"
#include "config.h"
#define MAQUE_NOTUSED(V) ((void)V)
#define MAQUE_IOBUF_LEN 1024

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
#include "ae.h"
#include "anet.h"

typedef struct {
    pthread_t mainthread;
    aeEventLoop *el;
    char neterr[ANET_ERR_LEN];
    int ipfd;
    int sofd;
    time_t unixtime;
    int maxclients;
} maqueServer;

typedef struct maqueClient {
    int fd;
    int dictid;
    int argc;
    struct maqueCommand *cmd;
    int reqtype;
    int multibulklen;       /* number of multi bulk arguments left to read */
    long bulklen;           /* length of bulk argument in multi bulk request */
    int sentlen;
    time_t lastinteraction; /* time of the last interaction, used for timeout */
    int flags;              /* MAQUE_SLAVE | MAQUE_MONITOR | MAQUE_MULTI ... */
    int slaveseldb;         /* slave selected db, if this client is a slave */
    int authenticated;      /* when requirepass is non-NULL */
    int replstate;          /* replication state if this is a slave */
    int repldbfd;           /* replication DB file descriptor */
    long repldboff;         /* replication DB file offset */
    off_t repldbsize;       /* replication DB file size */

    /* Response buffer */
    int bufpos;
} maqueClient;

extern maqueServer server;

extern char *getMaqueInfoString(void);
void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask);
void acceptUnixHandler(aeEventLoop *el, int fd, void *privdata, int mask);
void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask);