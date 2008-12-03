#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>

#include <adicpp/fbapi/ibase.h>

int main(int argc, char * argv[])
{
#if HAVE_PTHREAD_ATTR_GETSTACK
  pthread_attr_t  attr;
  pthread_attr_init(&attr);
  void *  stackaddr;
  size_t  stacksize;
  pthread_attr_getstack(&attr, &stackaddr, &stacksize);
  printf("stackaddr = %p, stacksize = %u\n", stackaddr, stacksize);
  pthread_attr_destroy(&attr);
#endif

  /*  sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGHUP);
    sigaddset(&signal_set, SIGUSR1);
    sigaddset(&signal_set, SIGUSR2);
    sigaddset(&signal_set, SIGTERM);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &signal_set, NULL);*/

  void *  fbclient  = dlopen("libgds.so", 0);
  printf("fbclient = %p\n", fbclient);

  ISC_STATUS  status[20];

  union {
      ISC_STATUS ISC_EXPORT (* isc_dsql_execute_immediate)(ISC_STATUS *,
                                                           isc_db_handle *,
                                                           isc_tr_handle *,
                                                           unsigned short,
                                                           char *,
                                                           unsigned short,
                                                           XSQLDA *);
      void *  p0;
  };
  p0 = dlsym(fbclient, "isc_dsql_execute_immediate");
  if( p0 != NULL ){
    isc_db_handle dbHandle  = NULL;
    isc_tr_handle trHandle  = NULL;
    long          r         = isc_dsql_execute_immediate(status, &dbHandle, &trHandle, 0, "CREATE DATABASE 'localhost:/usr/local/firebird/MACROSCOPE.FDB' user 'sysdba' password 'masterkey'", 3, NULL);
    printf("isc_dsql_execute_immediate = %ld, dbHandle = %p, trHandle = %p\n", r, dbHandle, trHandle);

    union {
        ISC_STATUS ISC_EXPORT (* isc_detach_database)(ISC_STATUS * status,
                                                      isc_db_handle * handle);
        void *  p2;
    };
    p2 = dlsym(fbclient, "isc_detach_database");
    r = isc_detach_database(status, &dbHandle);
    printf("isc_detach_database = %ld, dbHandle = %p\n", r, dbHandle);

    union {
        ISC_STATUS ISC_EXPORT (* isc_attach_database)(ISC_STATUS * status,
                                                      short dbNameLen,
                                                      const char * dbName,
                                                      isc_db_handle * handle,
                                                      short dpbLen,
                                                      const char * dpb);
        void *  p1;
    };
    p1 = dlsym(fbclient, "isc_attach_database");
    printf("isc_attach_database = %p\n", p1);

    char  dpb[] = {
      isc_dpb_version1, isc_dpb_user_name, 6, 'S', 'Y', 'S', 'D', 'B', 'A', isc_dpb_password, 9, 'm', 'a', 's', 't', 'e', 'r', 'k', 'e', 'y'
    };
    r = isc_attach_database(status, 0, "localhost:/usr/local/firebird/MACROSCOPE.FDB", &dbHandle, sizeof(dpb), dpb);
    printf("isc_attach_database = %ld, dbHandle = %p\n", r, dbHandle);
    r = isc_detach_database(status, &dbHandle);
    printf("isc_detach_database = %ld, dbHandle = %p\n", r, dbHandle);
  }
  dlclose(fbclient);
  return 0;
}
