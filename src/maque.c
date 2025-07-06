#include "maque.h"
#include "config.h"
#include "log.h"
#include "signal_handlers.h"
#include "test.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

maqueServer server;

void oom(const char *msg) {
    maqueLog(MAQUE_WARNING, "%s: Out of memory\n",msg);
    sleep(1);
    abort();
}

int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData) {
    // 每 1000 毫秒执行一次的定时任务

    MAQUE_NOTUSED(eventLoop);
    MAQUE_NOTUSED(id);
    MAQUE_NOTUSED(clientData);

    server.unixtime = time(NULL); // 更新服务器的当前时间
    maqueLog(MAQUE_NOTICE, "测试定时任务: 当前时间戳: %ld",
             server.unixtime); // 记录当前时间戳到日志
    // 每次执行定时任务时, 返回 1000 毫秒, 以便下次继续执行
    return 1000;
}

void beforeSleep(struct aeEventLoop *eventLoop) {
    MAQUE_NOTUSED(eventLoop);
    // 在事件循环之前执行的操作
    maqueLog(MAQUE_NOTICE, "Before sleep: 服务器当前时间戳: %ld",
             server.unixtime);
}

void initServer() {
  int j;
  setupSignalHandlers();

  server.mainthread = pthread_self(); // 获取当前线程的 ID （主线程）
  server.el = aeCreateEventLoop(); // IO 多路复用机制, 创建 epoll 事件循环

  if (config->port != 0) {
    server.ipfd = anetTcpServer(server.neterr, config->port,
                                config->bindaddr); // socket 监听
    if (server.ipfd == ANET_ERR) {
      maqueLog(MAQUE_WARNING, "Opening port: %s", server.neterr);
      exit(1);
    }
  }
  if (config->unixsocket != NULL) {
    unlink(config->unixsocket); /* don't care if this fails */
    server.sofd =
        anetUnixServer(server.neterr, config->unixsocket); // unix socket 监听
    if (server.sofd == ANET_ERR) {
      maqueLog(MAQUE_WARNING, "Opening socket: %s", server.neterr);
      exit(1);
    }
  }
  if (server.ipfd < 0 && server.sofd < 0) {
    maqueLog(MAQUE_WARNING, "Configured to not listen anywhere, exiting.");
    exit(1);
  }

  server.unixtime = time(NULL);
  aeCreateTimeEvent(
      server.el, 1000, serverCron, NULL,
      NULL); // 定时事件的生成， 每 1000 毫秒执行一次 serverCron 函数
  if (server.ipfd > 0 && aeCreateFileEvent(server.el, server.ipfd, AE_READABLE,
                                           acceptTcpHandler, NULL) == AE_ERR)
    oom("creating file event");
  if (server.sofd > 0 && aeCreateFileEvent(server.el, server.sofd, AE_READABLE,
                                           acceptUnixHandler, NULL) == AE_ERR)
    oom("creating file event");

  srand(time(NULL) ^ getpid());
}

char *getMaqueInfoString(void) {
  char *info = (char *)malloc(256);
  info = "this is a test info string";
  if (info == NULL) {
    maqueLog(MAQUE_WARNING, "Failed to allocate memory for info string.");
    return NULL;
  }
  return info;
}

void daemonize(void) {
  int fd;

  /* parent exits */
  // fork() 创建子进程, 然后把父进程(主进程)退出。
  // 新开的子进程的父进程号就会变成 1。
  // 这是实现守护进程的实现方式
  if (fork() != 0)
    exit(0);

  setsid(); /* create a new session */

  // 如果打开文件操作成功。 这里因为打开的是特殊文件 /dev/null, 所以权限给的是 0
  if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
    dup2(fd, STDIN_FILENO);  // 将标准输入重定向到 /dev/null
    dup2(fd, STDOUT_FILENO); // 将标准输出重定向到 /dev/null
    dup2(fd, STDERR_FILENO); // 将标准错误输出重定向到 /dev/null
    if (fd > STDERR_FILENO)
      close(fd);
  }
}

void createPidFile(void) {
  FILE *fp = fopen(config->pidfile, "w");
  if (fp) { // 将当前进程的 pid 写入到 pid 文件中
    fprintf(fp, "%d\n", (int)getpid());
    fclose(fp);
  }
}

void version() {
  printf("MaQue server version %s (%s:%d)\n", MAQUE_VERSION, "md5-xxxxx",
         atoi("1") > 0);
  exit(0);
}

void usage() {
  fprintf(stderr, "Usage: ./maque [/path/to/maque.conf]\n");
  fprintf(stderr, "       ./maque - (read config from stdin)\n");
  exit(1);
}

int main(int argc, char **argv) {

  time_t start;                    // 创建一个时间类型的变量名称
  initMaqueConfigDefaults(config); // 初始化配置

  if (argc == 2) {
    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
      version();
    if (strcmp(argv[1], "--help") == 0)
      usage();
    loadMaqueConfigFile(argv[1], config);
  } else if ((argc > 2)) {
    // 命令行只能有 程序本身 + 配置文件名称, 如果出现多余的参数, 直接报错
    usage();
  } else {
    // 不指定配置文件时,会以默认参数启动。这里只是输出一个警告的提示,
    // 不做其他动作
    maqueLog(MAQUE_WARNING,
             "Warning: no config file specified, using the default config. In "
             "order to specify a config file use 'maque /path/to/maque.conf'");
  }

  test_config();
  test_log();

  // 守护进程方式启动
  if (config->daemonize)
    daemonize();

  // 初始化 server
  initServer();

  // 写入 pid 文件
  if (config->daemonize)
    createPidFile();

  maqueLog(MAQUE_NOTICE, "Server started, MaQue version " MAQUE_VERSION);
  start = time(NULL);
  if (server.ipfd > 0)
        maqueLog(MAQUE_NOTICE,"The server is now ready to accept connections on port %d", config->port);
    if (server.sofd > 0)
        maqueLog(MAQUE_NOTICE,"The server is now ready to accept connections at %s", config->unixsocket);
    aeSetBeforeSleepProc(server.el,beforeSleep);
    aeMain(server.el);
    aeDeleteEventLoop(server.el);
  return 0;
}
