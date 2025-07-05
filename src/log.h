#ifndef MAQUE_LOG_H
#define MAQUE_LOG_H

#define MAQUE_DEBUG 0
#define MAQUE_VERBOSE 1
#define MAQUE_NOTICE 2
#define MAQUE_WARNING 3

extern char* MAQUE_LOG_FILE_NAME; // Log file name
extern int MAQUE_LOG_VERBOSITY; // Log verbosity level
extern int MAQUE_MAX_LOGMSG_LEN;

void maqueLog(int level, const char *fmt, ...);

#endif // MAQUE_LOG_H