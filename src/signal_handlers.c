#include "signal_handlers.h"
#include "config.h"
#include "log.h"
#include "maque.h"
#include "version.h"
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>

static void *getMcontextEip(ucontext_t *uc) {
#if defined(__FreeBSD__)
  return (void *)uc->uc_mcontext.mc_eip;
#elif defined(__dietlibc__)
  return (void *)uc->uc_mcontext.eip;
#elif defined(__APPLE__) && !defined(MAC_OS_X_VERSION_10_6)
#if __x86_64__
  return (void *)uc->uc_mcontext->__ss.__rip;
#else
  return (void *)uc->uc_mcontext->__ss.__eip;
#endif
#elif defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)
#if defined(_STRUCT_X86_THREAD_STATE64) && !defined(__i386__)
  return (void *)uc->uc_mcontext->__ss.__rip;
#else
  return (void *)uc->uc_mcontext->__ss.__eip;
#endif
#elif defined(__i386__)
  return (void *)uc->uc_mcontext.gregs[14]; /* Linux 32 */
#elif defined(__X86_64__) || defined(__x86_64__)
  return (void *)uc->uc_mcontext.gregs[16]; /* Linux 64 */
#elif defined(__ia64__) /* Linux IA64 */
  return (void *)uc->uc_mcontext.sc_ip;
#else
  return NULL;
#endif
}

// 收到程序崩溃信号时, 如: SIGSEGV, SIGBUS, SIGFPE, SIGILL 等,
// 生成一个堆栈跟踪, 并打印出相关信息,
// 然后退出程序, 以便于调试和分析问题
static void sigsegvHandler(int sig, siginfo_t *info, void *secret) {
  printf("进入到了 act.sa_sigaction 函数, 收到信号: %d\n", sig);
  void *trace[100];
  char **messages = NULL;
  int i, trace_size = 0;
  ucontext_t *uc = (ucontext_t *)secret;
  char *infostring;
  struct sigaction act;
  MAQUE_NOTUSED(info);

  maqueLog(MAQUE_WARNING,
           "======= Ooops! MaQue %s got signal: -%d- =======", MAQUE_VERSION,
           sig);
  infostring = getMaqueInfoString();
  maqueLog(MAQUE_WARNING, "%s", infostring);
  /* It's not safe to sdsfree() the returned string under memory
   * corruption conditions. Let it leak as we are going to abort */

  trace_size = backtrace(trace, 100);
  /* overwrite sigaction with caller's address */
  if (getMcontextEip(uc) != NULL) {
    trace[1] = getMcontextEip(uc);
  }
  messages = backtrace_symbols(trace, trace_size);

  for (i = 1; i < trace_size; ++i)
    maqueLog(MAQUE_WARNING, "%s", messages[i]);

  /* free(messages); Don't call free() with possibly corrupted memory. */
  if (config->daemonize)
    unlink(config->pidfile);

  /* Make sure we exit with the right signal at the end. So for instance
   * the core will be dumped if enabled. */
  sigemptyset(&act.sa_mask);
  /* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction
   * is used. Otherwise, sa_handler is used */
  act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
  act.sa_handler = SIG_DFL;
  sigaction(sig, &act, NULL);
  kill(getpid(), sig);
}

static void sigtermHandler(int sig) {
  printf("进入到了 act.sa_handler 函数, 收到信号: %d\n", sig);

  MAQUE_NOTUSED(sig);

  maqueLog(MAQUE_WARNING, "Received SIGTERM, scheduling shutdown...");
  config->shutdown_asap = 1;
}

void setupSignalHandlers(void) {

  signal(SIGHUP, SIG_IGN);  // 忽略信号: SIGHUP, 即无视: kill -1
  signal(SIGPIPE, SIG_IGN); // 忽略信号: SIGPIPE, 即无视: kill -13

  // 处理其他信号, 如: kill -2, kill -11 等
  struct sigaction act;

  /* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction is used.
   * Otherwise, sa_handler is used. */
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
  act.sa_handler = sigtermHandler;
  // 下面这行代码设置 SIGTERM 信号的处理方式。
  // 由于 act.sa_flags 还没有设置 SA_SIGINFO 标志, 所以 SIGTERM 信号使用
  // act.sa_handler 函数处理
  sigaction(SIGTERM, &act, NULL);  // 处理 SIGTERM 信号, 即: kill -15

  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND | SA_SIGINFO;
  act.sa_sigaction = sigsegvHandler;

  // 下面这 4 行代码设置 SIGSEGV, SIGBUS, SIGFPE, SIGILL 信号的处理方式。 由于
  // act.sa_flags 设置了 SA_SIGINFO 标志, 所以这些信号使用 act.sa_sigaction
  // 函数处理
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGFPE, &act, NULL);
  sigaction(SIGILL, &act, NULL);

  return;
}