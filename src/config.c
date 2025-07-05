#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "config.h"
#include "log.h"
#include "../deps/iniparser/src/iniparser.h"

#define MAQUE_CONFIG_LINE_MAX 1024
#define MAQUE_DEFAULT_SERVERPORT 8866    /* TCP port */
#define MAQUE_DEFAULT_DBNUM 16           /* 数据库数量 */
#define MAQUE_DEFAULT_PIDFILE "/var/run/maque.pid"    /* PID file */

MaqueConfig* config; // 全局配置变量

void initMaqueConfigDefaults(MaqueConfig* config) {
    config->port = MAQUE_DEFAULT_SERVERPORT;
    config->bindaddr = NULL;
    config->unixsocket = NULL;
    config->dbnum = MAQUE_DEFAULT_DBNUM;
    config->daemonize = 0;
    config->appendonly = 0;
    config->pidfile = MAQUE_DEFAULT_PIDFILE;
    config->logfile = NULL; /* NULL = log on standard output */
    config->verbosity = MAQUE_VERBOSE;
}

int loadMaqueConfigFile(const char* filename, MaqueConfig* config) {
    // FILE *fp;

    dictionary *configfile = NULL;
    char buf[MAQUE_CONFIG_LINE_MAX+1], *err = NULL;
    int linenum = 0;
    int really_use_vm = 0;

    if ((configfile = iniparser_load(filename)) == NULL) {
        maqueLog(MAQUE_WARNING, "Fatal error, can't parse config file '%s'", filename);
         exit(1);
    }
    config->port = iniparser_getint(configfile, "default:port", MAQUE_DEFAULT_SERVERPORT);
    config->bindaddr = iniparser_getstring(configfile, "default:bindaddr", NULL);
    config->unixsocket = iniparser_getstring(configfile, "default:unixsocket", NULL);
    config->pidfile = iniparser_getstring(configfile, "default:pidfile", MAQUE_DEFAULT_PIDFILE);
    config->logfile = iniparser_getstring(configfile, "default:logfile", NULL);
    config->verbosity = iniparser_getint(configfile, "default:verbosity", MAQUE_VERBOSE);
    config->daemonize = iniparser_getboolean(configfile, "default:daemonize", 0);
    config->appendonly = iniparser_getboolean(configfile, "default:appendonly", 0);
    config->dbnum = iniparser_getint(configfile, "default:dbnum", MAQUE_DEFAULT_DBNUM);
    if (config->port < 0 || config->port > 65535) {
        maqueLog(MAQUE_WARNING, "Invalid port number %d in config file '%s'", config->port, filename);
        iniparser_freedict(configfile);
        return -1;
    }
    return 0;
}