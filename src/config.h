#ifndef MAQUE_CONFIG_H
#define MAQUE_CONFIG_H



typedef struct {
    int port;
    char *bindaddr;
    char *unixsocket;
    int dbnum;
    int daemonize;
    int appendonly;
    const char *pidfile;
    char *logfile;
    int verbosity;
    // int syslog_enabled;
    // char *syslog_ident;
    // int syslog_facility;
} MaqueConfig;

extern MaqueConfig config; // 全局配置变量

extern void initMaqueConfigDefaults(MaqueConfig* config);

// 函数声明
extern int loadMaqueConfigFile(const char* filename, MaqueConfig* config);

#endif // MAQUE_CONFIG_H