#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/endian.h>
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

    KQueue &  testConnect();
    KQueue &  testRegularFiles();
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
  fprintf(stderr, "kqueue = %d\n", kqueue_);
}

KQueue & KQueue::testConnect()
{
  struct kevent kev;
  int           s;

  s = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
  if( s == -1 ){
    perror(NULL);
    abort();
  }
  if( fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK) != 0 ){
    perror(NULL);
    abort();
  }
  EV_SET(&kev, s, EVFILT_READ | EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, 0);
  int evc = kevent(kqueue_, &kev, 1, NULL, 0, NULL);
  fprintf(stderr,"evc == %d\n",evc);
  if( evc == -1 ){
    perror(NULL);
    abort();
  }
  struct sockaddr_in  addr;
  addr.sin_len = sizeof(addr);
  addr.sin_family = PF_INET;
  addr.sin_addr.s_addr = inet_addr("192.168.201.31");//INADDR_LOOPBACK;
  addr.sin_port = htons(2121);
  fprintf(stderr,"sizeof(addr) == %u\n",sizeof(addr));
  if( connect(s, (const struct sockaddr *) &addr, sizeof(addr)) != 0 && errno != EINPROGRESS ){
    perror(NULL);
    abort();
  }
  int kcount;
  struct timespec timeout = { 0, 0 }, * pto = NULL;
  char b[1];
  for(;;){
    kcount = kevent(kqueue_, NULL, 0, &kev, 1, NULL);
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
      // if connect failed
      // ident = 7, filter = -1, flags = 32789, fflags = 60, data = 0, udata = 0x0
      errno = kev.fflags;
      perror(NULL);
      abort();
    }

    fprintf(stderr, "%d\n", kev.filter);
    if( kev.filter == EVFILT_READ ){
      fprintf(stderr, "EVFILT_READ\n");
    }
    if( kev.filter == EVFILT_WRITE ){
      fprintf(stderr, "EVFILT_WRITE\n");
    }
    ssize_t r;
    while( (r = recv(s, b, sizeof(b), 0)) > 0 ) fprintf(stderr, "%c", b[0]);
    if( r <= 0 ){
      if( errno != EAGAIN ){
        perror(NULL);
        abort();
      }
      static char cmd[] = "USER korvin\n";
      r = send(s, cmd, sizeof(cmd) - 1, 0);
      if( r <= 0 ){
        perror(NULL);
        abort();
      }
    }
    pto = &timeout;
  }

  if( shutdown(s, SHUT_RDWR) != 0 ){
    perror(NULL);
    abort();
  }
  if( close(s) != 0 ){
    perror(NULL);
    abort();
  }
  return *this;
}

KQueue & KQueue::testRegularFiles()
{
  struct kevent kev;
  int           f;

  f = open("qwert", O_RDWR | O_CREAT/* | O_NONBLOCK*/);
  if( f == -1 ){
    perror(NULL);
    abort();
  }
  if( fcntl(f, F_SETFL, fcntl(f, F_GETFL, 0) | O_NONBLOCK) != 0 ){
    perror(NULL);
    abort();
  }
  char          b[1 * 1024 * 1024];
  struct aiocb  iocb;
  memset(&iocb, 0, sizeof(iocb));
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
  for( ; ; ){
    kcount = kevent(kqueue_, NULL, 0, &kev, 1, pto);
    if( kcount == -1 ){
      perror(NULL);
      abort();
    }
    if( kcount == 0 )
      break;
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
      fprintf(stderr, "EVFILT_READ\n");
    }
    if( kev.filter == EVFILT_WRITE ){
      fprintf(stderr, "EVFILT_READ\n");
    }
    if( kev.filter == EVFILT_AIO ){
      fprintf(stderr, "EVFILT_AIO\n");
      kev.flags = EV_DELETE;
      if( kevent(kqueue_, &kev, 1, NULL, 0, NULL) == -1 ){
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

int main(int argc, char ** argv)
{
//  be32enc(NULL,0);

  KQueue kqueue;
  kqueue.testConnect();
  //  kqueue.testRegularFiles();
  return 0;
}
