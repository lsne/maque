/* Log levels */
const int MAQUE_DEBUG = 0;
const int MAQUE_VERBOSE = 1;
const int MAQUE_NOTICE = 2;
const int MAQUE_WARNING = 3;

extern char* MAQUE_LOG_FILE_NAME; // Log file name
extern int MAQUE_LOG_VERBOSITY; // Log verbosity level
extern int MAQUE_MAX_LOGMSG_LEN;

void maqueLog(int level, const char *fmt, ...);