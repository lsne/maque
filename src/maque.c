


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

void version() {
    printf("Redis server version %s (%s:%d)\n", REDIS_VERSION,
        redisGitSHA1(), atoi(redisGitDirty()) > 0);
    exit(0);
}

void usage() {
    fprintf(stderr,"Usage: ./redis-server [/path/to/redis.conf]\n");
    fprintf(stderr,"       ./redis-server - (read config from stdin)\n");
    exit(1);
}

int main(int argc, char **argv) {
    time_t start;                                                   // 创建一个时间类型的变量名称

    initServerConfig();
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0 ||
            strcmp(argv[1], "--version") == 0) version();
        if (strcmp(argv[1], "--help") == 0) usage();
        resetServerSaveParams();                                    // 如果指定了配置文件, 则将设置的默认 save 快照保存策略重置清理
        loadServerConfig(argv[1]);                        // 根据配置文件配置, 设置相关参数
    } else if ((argc > 2)) {                                        // 命令行只能有 程序本身 + 配置文件名称, 如果出现多余的参数, 直接报错
        usage();
    } else {                                                       // 不指定配置文件时,会以默认参数启动。这里只是输出一个警告的提示, 不做其他动作
        redisLog(REDIS_WARNING,"Warning: no config file specified, using the default config. In order to specify a config file use 'redis-server /path/to/redis.conf'");
    }
    if (server.daemonize) daemonize();                             // 守护进程方式启动
    initServer();                                                  // 初始化服务器
    if (server.daemonize) createPidFile();
    redisLog(REDIS_NOTICE,"Server started, Redis version " REDIS_VERSION);
#ifdef __linux__
    linuxOvercommitMemoryWarning();                               // 如果是 linux 系统, 则会检测内核参数 overcommit_memory 是否设置为1, 没有设置为1则会输出一个警告信息
#endif
    start = time(NULL);                                    // 获取当前时间
    if (server.appendonly) {
        if (loadAppendOnlyFile(server.appendfilename) == REDIS_OK)
            redisLog(REDIS_NOTICE,"DB loaded from append only file: %ld seconds",time(NULL)-start);
    } else {
        if (rdbLoad(server.dbfilename) == REDIS_OK)
            redisLog(REDIS_NOTICE,"DB loaded from disk: %ld seconds",time(NULL)-start);
    }
    if (server.ipfd > 0)
        redisLog(REDIS_NOTICE,"The server is now ready to accept connections on port %d", server.port);
    if (server.sofd > 0)
        redisLog(REDIS_NOTICE,"The server is now ready to accept connections at %s", server.unixsocket);
    aeSetBeforeSleepProc(server.el,beforeSleep);
    aeMain(server.el);
    aeDeleteEventLoop(server.el);
    return 0;
}
