#include "test.h"
#include <stdio.h>
#include "config.h" 
#include "log.h"

void test_config(void){
  printf("test .... config file args: bindaddr - %s\n",
         config->bindaddr ? config->bindaddr : "NULL");
  printf("test .... config file args: port -  %d\n", config->port);
  printf("test .... config file args: dbnum - %d\n", config->dbnum);
  printf("test .... config file args: logfile - %s\n",
         config->logfile ? config->logfile : "NULL");
  printf("test .... config file args: verbosity - %d\n", config->verbosity);
  printf("test .... config file args: daemonize - %d \n", config->daemonize);
  printf("test .... config file args: appendonly - %d \n", config->appendonly);
  printf("test .... config file args: pidfile - %s \n",
         config->pidfile ? config->pidfile : "NULL");
}

void test_log(void) {
  maqueLog(MAQUE_NOTICE, "This is a test log message at NOTICE level.");
  maqueLog(MAQUE_WARNING, "This is a test log message at WARNING level.");
  maqueLog(MAQUE_DEBUG, "This is a test log message at DEBUG level.");
  maqueLog(MAQUE_VERBOSE, "This is a test log message at VERBOSE level.");
}