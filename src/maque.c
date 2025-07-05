#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> 
#include <fcntl.h>
#include "maque.h"
#include "config.h"
#include "log.h"

maqueServer server;

void initServer() {
    server.ipfd = -1;
    server.sofd = -1;
}

void daemonize(void) {
    int fd;

    if (fork() != 0) exit(0); /* parent exits */           // // fork() 创建子进程, 然后把父进程(主进程)退出, 新开的子进程的父进程号就会变成 1。 这是实现守护进程的实现方式
    setsid(); /* create a new session */

    /* Every output goes to /dev/null. If Redis is daemonized but
     * the 'logfile' is set to 'stdout' in the configuration file
     * it will not log at all. */
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {  // 如果打开文件操作成功。 这里因为打开的是特殊文件 /dev/null, 所以权限给的是 0
        dup2(fd, STDIN_FILENO);                              // 将标准输入重定向到 /dev/null
        dup2(fd, STDOUT_FILENO);                             // 将标准输出重定向到 /dev/null
        dup2(fd, STDERR_FILENO);                             // 将标准错误输出重定向到 /dev/null
        if (fd > STDERR_FILENO) close(fd);
    }
}

void createPidFile(void) {
    /* Try to write the pid file in a best-effort way. */
    FILE *fp = fopen(config->pidfile,"w");
    if (fp) {
        fprintf(fp,"%d\n",(int)getpid());  // 将当前进程的 pid 写入到 pid 文件中
        fclose(fp);
    }
}

void version() {
    // printf("Redis server version %s (%s:%d)\n", REDIS_VERSION,
    //     redisGitSHA1(), atoi(redisGitDirty()) > 0);
    printf("MaQue server version %s (%s:%d)\n", MAQUE_VERSION,
        "md5-xxxxx", atoi("1") > 0);
    exit(0);
}

void usage() {
    fprintf(stderr,"Usage: ./maque [/path/to/maque.conf]\n");
    fprintf(stderr,"       ./maque - (read config from stdin)\n");
    exit(1);
}

int main(int argc, char **argv) {
    time_t start;                                                   // 创建一个时间类型的变量名称
    initMaqueConfigDefaults(config);                          // 初始化配置
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0 ||
            strcmp(argv[1], "--version") == 0) version();
        if (strcmp(argv[1], "--help") == 0) usage();
        loadMaqueConfigFile(argv[1], config);
    } else if ((argc > 2)) {                                        // 命令行只能有 程序本身 + 配置文件名称, 如果出现多余的参数, 直接报错
        usage();
    } else {                                                       // 不指定配置文件时,会以默认参数启动。这里只是输出一个警告的提示, 不做其他动作
        maqueLog(MAQUE_WARNING,"Warning: no config file specified, using the default config. In order to specify a config file use 'maque /path/to/maque.conf'");
    }
    if (config->daemonize) daemonize();                             // 守护进程方式启动
    initServer();                                                  // 初始化服务器
    if (config->daemonize) createPidFile();
    maqueLog(MAQUE_NOTICE,"Server started, Maque version " MAQUE_VERSION);
    initServer();
    printf("test .... config file args: bindaddr - %s", config->bindaddr ? config->bindaddr : "NULL");
    printf("test .... config file args: port -  %d\n", config->port);
    printf("test .... config file args: dbnum - %d\n", config->dbnum);
    printf("test .... config file args: logfile - %s\n", config->logfile ? config->logfile : "NULL");
    printf("test .... config file args: verbosity - %d\n", config->verbosity);
    printf("test .... config file args: daemonize - %d \n", config->daemonize);
    printf("test .... config file args: appendonly - %d \n", config->appendonly);
    printf("test .... config file args: pidfile - %s \n", config->pidfile ? config->pidfile : "NULL");
    start = time(NULL);
    maqueLog(MAQUE_NOTICE,"Server started, Maque version %s, pid %d, running on %s, started at %s",
        MAQUE_VERSION, (int)getpid(),
        config->bindaddr ? config->bindaddr : "NULL",
        ctime(&start));  // ctime() 将时间戳转换为字符串格式
    // 测试 maqueLog 函数
    maqueLog(MAQUE_NOTICE, "This is a test log message at NOTICE level.");
    maqueLog(MAQUE_WARNING, "This is a test log message at WARNING level.");
    maqueLog(MAQUE_DEBUG, "This is a test log message at DEBUG level.");
    maqueLog(MAQUE_VERBOSE, "This is a test log message at VERBOSE level.");
    return 0;
}
