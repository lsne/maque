#include <stdio.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include "log.h"
#include "config.h"

/* 实际定义全局变量并初始化 */
char* MAQUE_LOG_FILE_NAME = NULL;      // 默认输出到 stdout
int MAQUE_LOG_VERBOSITY = MAQUE_NOTICE; // 默认日志级别
int MAQUE_MAX_LOGMSG_LEN = 1024;       // 默认消息长度

/**
 * maqueLog - 日志记录函数
 * @level: 日志级别
 * @fmt: 格式化字符串
 * @...: 可变参数列表
 *
 * 根据日志级别记录日志到指定文件或标准输出。
 */

void maqueLog(int level, const char *fmt, ...) {
    if (level < MAQUE_DEBUG || level > MAQUE_WARNING) return;
    if (level < config->verbosity) return;

    FILE *fp;
    const char *c = ".-*#";
    time_t now = time(NULL);
    va_list ap;
    char buf[64];
    char msg[MAQUE_MAX_LOGMSG_LEN];

    fp = (config->logfile == NULL) ? stdout : fopen(config->logfile,"a");
    if (!fp) return;

    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    strftime(buf,sizeof(buf),"%d %b %H:%M:%S",localtime(&now));
    fprintf(fp,"[%d] %s %c %s\n",(int)getpid(),buf,c[level],msg);
    fflush(fp);

    if (fp) fclose(fp);
}