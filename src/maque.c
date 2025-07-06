#include "maque.h"
#include "config.h"
#include "log.h"
#include "test.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "signal_handlers.h"

maqueServer server;

void initServer() {
  int j;
  setupSignalHandlers();
  sleep(1000);
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
  maqueLog(
      MAQUE_NOTICE,
      "Server started, MaQue version %s, pid %d, running on %s, started at %s",
      MAQUE_VERSION, (int)getpid(),
      config->bindaddr ? config->bindaddr : "NULL",
      ctime(&start)); // ctime() 将时间戳转换为字符串格式
  return 0;
}
