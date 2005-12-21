#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <aio.h>
	  	       
class KQueue {
  public:
    ~KQueue();
    KQueue();
    
    KQueue & testRegularFiles();
  protected:
  private:
    int kqueue_;
};

KQueue::~KQueue()
{
  if( close(kqueue_) != 0 ){
    perror(NULL);
    abort();
  }
}

KQueue::KQueue()
{
  kqueue_ = kqueue();
  if( kqueue_ == -1 ){
    perror(NULL);
    abort();
  }
}

KQueue & KQueue::testRegularFiles()
{
  struct kevent kev;
  int f;
  
  f = open("qwert",O_RDWR | O_CREAT/* | O_NONBLOCK*/);
  if( f == -1 ){
    perror(NULL);
    abort();
  }
  if( fcntl(f,F_SETFL,fcntl(f,F_GETFL,0) | O_NONBLOCK) != 0 ){
    perror(NULL);
    abort();
  }
  char b[1 * 1024 * 1024];
  struct aiocb iocb;
  memset(&iocb,0,sizeof(iocb));
  iocb.aio_fildes = f;
  iocb.aio_nbytes = sizeof(b);
  iocb.aio_buf = b;
  iocb.aio_offset = 0;
  iocb.aio_sigevent.sigev_notify_kqueue = kqueue_;
  iocb.aio_sigevent.sigev_notify = SIGEV_KEVENT;
  if( aio_write(&iocb) != 0 ){
    perror(NULL);
    abort();
  }
  int kcount;
  struct timespec timeout = { 0, 0 }, * pto = NULL;
  for(;;){
    kcount = kevent(kqueue_,NULL,0,&kev,1,pto);
    if( kcount == -1 ){
      perror(NULL);
      abort();
    }
    if( kcount == 0 ) break;
    if( kev.flags & EV_ERROR ){
      errno = kev.data;
      perror(NULL);
      abort();
    }
    if( kev.flags & EV_EOF ){
      if( (errno = kev.fflags) != 0 ){
        perror(NULL);
        abort();
      }
    }

    if( kev.filter == EVFILT_READ ){
      fprintf(stderr,"EVFILT_READ\n");
    }
    if( kev.filter == EVFILT_WRITE ){
      fprintf(stderr,"EVFILT_READ\n");
    }
    if( kev.filter == EVFILT_AIO ){
      fprintf(stderr,"EVFILT_AIO\n");
      kev.ident = (uintptr_t) &iocb;
      kev.filter = EVFILT_AIO;
//      kev.flags = EV_DELETE;
// reboot after this call
      if( kevent(kqueue_,&kev,1,NULL,0,NULL) == -1 ){
        perror(NULL);
        abort();
      }
    }
    pto = &timeout;
  }
  if( close(f) != 0 ){
    perror(NULL);
    abort();
  }
  return *this;
}

int main(int argc,char ** argv)
{
  KQueue kqueue;
  kqueue.testRegularFiles();
  return 0;
}
