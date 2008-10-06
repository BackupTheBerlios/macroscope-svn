/*-
 * Copyright 2005-2008 Guram Dukashvili
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
//---------------------------------------------------------------------------
#include <adicpp/ksys.h>
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
bool AsyncDescriptor::isSocket() const
{
  return false;
}
//------------------------------------------------------------------------------
#if HAVE_KQUEUE || __linux__
//------------------------------------------------------------------------------
int64_t AsyncDescriptor::read2(void *,uint64_t)
{
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
  return -1;
}
//------------------------------------------------------------------------------
int64_t AsyncDescriptor::write2(const void *,uint64_t)
{
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
  return -1;
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
void AsyncDescriptor::shutdown2()
{
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
void AsyncDescriptor::flush2()
{
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
void AsyncDescriptor::close2()
{
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void BaseThread::initialize()
{
  new (requester_) Requester;
#if defined(__WIN32__) || defined(__WIN64__)
#else
#if HAVE_SIGNAL_H
//  sigset_t sigs;
//	sigemptyset(&sigs);
//	sigaddset(&sigs,SIGIO);
//	sigaddset(&sigs,SIGRTMIN);
//	sigprocmask(SIG_BLOCK,&sigs,NULL);
#endif
#endif
}
//------------------------------------------------------------------------------
void BaseThread::cleanup()
{
  requester().~Requester();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncIoSlave::SocketInitializer::~SocketInitializer()
{
  ksock::api.close();
}
//------------------------------------------------------------------------------
AsyncIoSlave::SocketInitializer::SocketInitializer()
{
  ksock::api.open();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncIoSlave::~AsyncIoSlave()
{
#if defined(__WIN32__) || defined(__WIN64__)
  for( intptr_t i = sizeof(safeEvents_) / sizeof(safeEvents_[0]) - 1; i >= 0; i-- )
    CloseHandle(safeEvents_[i]);
#elif HAVE_KQUEUE
  if( kqueue_ >= 0 && close(kqueue_) != 0 ){
    perror(NULL);
    assert( 0 );
    abort();
  }
#endif
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
AsyncIoSlave::AsyncIoSlave() : maxRequests_(MAXIMUM_WAIT_OBJECTS - 1)
#else
AsyncIoSlave::AsyncIoSlave(bool connect) : connect_(connect), maxRequests_(64)
#endif
{
#if defined(__WIN32__) || defined(__WIN64__)
  intptr_t i;
  int32_t err;
  for( i = sizeof(eReqs_) / sizeof(eReqs_[0]) - 1; i >= 0; i-- ) eReqs_[i] = NULL;
  for( i = sizeof(safeEvents_) / sizeof(safeEvents_[0]) - 1; i >= 0; i-- ) safeEvents_[i] = NULL;
  for( i = sizeof(safeEvents_) / sizeof(safeEvents_[0]) - 1; i >= 0; i-- ){
    if( (safeEvents_[i] = CreateEventA(NULL,TRUE,FALSE,NULL)) == NULL ){
      err = GetLastError() + errorOffset;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
  }
#else
  if( connect_ ){
    rfds_.alloc(sizeof(fd_set));
    wfds_.alloc(sizeof(fd_set));
    FD_ZERO(rfds_);
    FD_ZERO(wfds_);
  }
#if HAVE_KQUEUE
  else {
    kqueue_ = -1;
    kqueue_ = kqueue();
    if( kqueue < 0 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    kevents_.resize(64);
  }
#else // TODO: use epoll under linux
  newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
#endif
}
//---------------------------------------------------------------------------
void AsyncIoSlave::threadBeforeWait()
{
  terminate();
  post();
}
//---------------------------------------------------------------------------
#if HAVE_KQUEUE
//---------------------------------------------------------------------------
void AsyncIoSlave::cancelEvent(const AsyncEvent & request)
{
  if( request.type_ != etConnect ){
    struct kevent ke;
    EV_SET(&ke,request.descriptor_->socket_,EVFILT_READ | EVFILT_WRITE,EV_DELETE,0,0,0);
    if( kevent(kqueue_,&ke,1,NULL,0,NULL) == -1 ){
      perror(NULL);
      assert( 0 );
      abort();
    }
    EV_SET(&ke,1000,EVFILT_TIMER,EV_ADD | EV_ONESHOT,0,0,&request.node(request));
    if( kevent(kqueue_,&ke,1,NULL,0,NULL) == -1 ){
      perror(NULL);
      assert( 0 );
      abort();
    }
  }
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
bool AsyncIoSlave::transplant(AsyncEvent & request)
{
  bool r = false;
#if defined(__WIN32__) || defined(__WIN64__)
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
    if( requests_.count() + newRequests_.count() < maxRequests_ ){
      newRequests_.insToTail(request);
      BOOL es = SetEvent(safeEvents_[MAXIMUM_WAIT_OBJECTS - 1]);
      assert( es != 0 );
      if( requests_.count() == 0 ) post();
      r = true;
    }
  }
#else
  uintptr_t MAX_REQS;
#if HAVE_KQUEUE
  MAX_REQS = 64;
#endif
  if( connect_ ) MAX_REQS = FD_SETSIZE;
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
    if( requests_.count() + newRequests_.count() < MAX_REQS ){
      newRequests_.insToTail(request);
      if( newRequests_.count() < 2 ){
        if( connect_ ){
	}
#if HAVE_KQUEUE
        else {
          struct kevent ev;
          EV_SET(&ev,1000,EVFILT_TIMER,EV_ADD | EV_ONESHOT,0,0,0);
          if( kevent(kqueue_,&ev,1,NULL,0,NULL) == -1 ){
            perror(NULL);
            abort();
          }
	}
#else
        newObjectV1C2<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
#endif
        if( requests_.count() == 0 ) post();
      }
      r = true;
    }
  }
#undef MAX_REQS
#endif
  return r;
}
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
void AsyncIoSlave::threadExecute()
{
//  priority(THREAD_PRIORITY_HIGHEST);
//  priority(THREAD_PRIORITY_LOWEST);
  BOOL rw = FALSE;
  DWORD nb = 0;
  intptr_t sp = -1, ssp = MAXIMUM_WAIT_OBJECTS - 1;
  bool isw9x = isWin9x();
  EventsNode * node;
  AsyncEvent * object = NULL;
  HANDLE events[MAXIMUM_WAIT_OBJECTS];
  for(;;){
    AutoLock<InterlockedMutex> lock(*this);
    for( node = newRequests_.first(); node != NULL; node = newRequests_.first() ){
      object = &AsyncEvent::nodeObject(*node);
      //openAPI(object);
      assert( sp < MAXIMUM_WAIT_OBJECTS - 1 );
      sp++;
      eReqs_[sp] = object;
      memset(&object->overlapped_,0,sizeof(object->overlapped_));
      object->overlapped_.Offset = (DWORD) object->position_;
      object->overlapped_.OffsetHigh = (DWORD) (object->position_ >> 32);
      ssp--;
      events[sp] = safeEvents_[ssp];
      safeEvents_[ssp] = NULL;
      nb = ResetEvent(events[sp]);
      assert( nb != 0 );
      object->overlapped_.hEvent = events[sp];
l1:   SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
      SetLastError(ERROR_SUCCESS);
      switch( object->type_ ){
        case etDirectoryChangeNotification :
          assert( !isw9x && object->directoryChangeNotification_->hDirectory() != INVALID_HANDLE_VALUE );
          if( object->abort_ ){
            rw = 0;
            SetLastError(ERROR_REQUEST_ABORTED);
          }
          else {
            rw = ReadDirectoryChangesW(
              object->directoryChangeNotification_->hDirectory(),
              object->directoryChangeNotification_->buffer(),
              (DWORD) object->directoryChangeNotification_->bufferSize(),
              FALSE,
              FILE_NOTIFY_CHANGE_FILE_NAME,
              &nb,
              &object->overlapped_,
              NULL
            );
          }
          break;
        case etWaitCommEvent :
          rw = WaitCommEvent(
            object->descriptor_->descriptor_,
            &object->evtMask_,
            &object->overlapped_
          );
          break;
        case etLockFile :
          if( object->length_ == 0 ) object->length_ = ~UINT64_C(0);
          switch( object->lockType_ ){
            case AsyncEvent::rdLock    :
              nb = 0;
              break;
            case AsyncEvent::wrLock    :
              nb = LOCKFILE_EXCLUSIVE_LOCK;
              break;
            case AsyncEvent::tryRDLock :
              nb = LOCKFILE_FAIL_IMMEDIATELY;
              break;
            case AsyncEvent::tryWRLock :
              nb = LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK;
              break;
            default :
              assert( 0 );
          }
          rw = LockFileEx(
            object->descriptor_->descriptor_,
            nb,
            0,
            (DWORD) object->length_,
            (DWORD) (object->length_ >> 32),
            isw9x ? NULL : &object->overlapped_
          );
          break;
        case etRead     :
	        if( object->length_ > 0 ){
            if( object->length_ > 0x40000000 ) object->length_ = 0x40000000;
            rw = object->descriptor_->Read(
              object->buffer_,
              (DWORD) object->length_,
              &nb,
              isw9x ? NULL : &object->overlapped_
            );
          }
	        else {
	          errno = EINVAL;
	        }
          break;
        case etWrite  :
	        if( object->length_ > 0 ){
            if( object->length_ > 0x40000000 ) object->length_ = 0x40000000;
            rw = object->descriptor_->Write(
              object->buffer_,
              (DWORD) object->length_,
              &nb,
              isw9x ? NULL : &object->overlapped_
            );
	        }
	        else {
            errno = EINVAL;
	        }
          break;
        case etAccept :
          rw = object->descriptor_->AcceptEx(
            object->socket_,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            isw9x ? NULL : &object->overlapped_
          );
          break;
        case etConnect :
          rw = object->descriptor_->Connect(events[sp],object);
          break;
        default :
          assert( 0 );
      }
      if( rw == 0 && GetLastError() == ERROR_NO_SYSTEM_RESOURCES )
        if( (object->length_ >>= 1) > 0 ) goto l1;
      if( rw == 0 && GetLastError() != ERROR_IO_PENDING && GetLastError() != WSAEWOULDBLOCK ){
        DWORD err = GetLastError();
        //closeAPI(object);

        safeEvents_[ssp++] = events[sp];
        events[sp] = NULL;
        //events[sp + 1] = NULL;
        eReqs_[sp] = NULL;
        sp--;

        newRequests_.remove(*object);
        object->errno_ = err;
        object->count_ = ~uint64_t(0);
        object->fiber_->thread()->postEvent(object);
      }
      else {
        requests_.insToTail(newRequests_.remove(*object));
        SetLastError(ERROR_SUCCESS);
      }
    }
    if( requests_.count() == 0 ){
      if( terminated_ ) break;
      release();
      Semaphore::wait();
      acquire();
    }
    else {
      node = NULL;
      object = NULL;
      DWORD wm0 = WAIT_OBJECT_0, wm;
      if( isw9x ){
        node = requests_.first();
        object = &AsyncEvent::nodeObject(*node);
        wm = ~DWORD(0);
      }
      else {
        uint64_t timeout = ~uint64_t(0);
        for( node = requests_.first(); node != NULL; node = node->next() ){
          object = &AsyncEvent::nodeObject(*node);
          if( object->timeout_ < timeout )
            timeout = object->timeout_;
        }
        DWORD tma = timeout == ~uint64_t(0) ? INFINITE : DWORD(timeout / 1000u);
        events[sp + 1] = safeEvents_[MAXIMUM_WAIT_OBJECTS - 1];
        release();
        timeout = gettimeofday();
        wm = WaitForMultipleObjectsEx(DWORD(sp + 2),events,FALSE,tma,TRUE);
        DWORD err0 = GetLastError();
        acquire();
        SetLastError(err0);
        if( wm >= wm0 && wm < WAIT_OBJECT_0 + sp + 1 ){
          wm -= WAIT_OBJECT_0;
l2:       object = eReqs_[wm];
          assert( object != NULL );
          node = &AsyncEvent::node(*object);
          SetLastError(ERROR_SUCCESS);
          if( object->type_ == etConnect ){
            nb = 0;
            object->descriptor_->WSAEnumNetworkEvents(events[wm],FD_CONNECT_BIT);
          }
          else {
            if( object->type_ == etDirectoryChangeNotification ){
              if( object->abort_ ){
                SetLastError(ERROR_REQUEST_ABORTED);
              }
              else {
                GetOverlappedResult(
                  object->directoryChangeNotification_->hDirectory(),
                  &object->overlapped_,
                  &nb,
                  TRUE
                );
              }
            }
            else {
              object->descriptor_->GetOverlappedResult(
                &object->overlapped_,
                &nb,
                TRUE
              );
            }
          }
        }
        else if( wm >= WAIT_ABANDONED_0 && wm < WAIT_ABANDONED_0 + sp + 1 ){
          wm -= WAIT_ABANDONED_0;
          goto l2;
        }
        else if( wm == WAIT_OBJECT_0 + sp + 1	){
          wm -= WAIT_OBJECT_0;
          nb = ResetEvent(events[wm]);
          assert( nb != 0 );
        }
        else if( wm == WAIT_ABANDONED_0 + sp + 1	){
          wm -= WAIT_ABANDONED_0;
          nb = ResetEvent(events[wm]);
          assert( nb != 0 );
        }
        else if( wm == WAIT_TIMEOUT ){
          timeout = gettimeofday() - timeout;
          for( intptr_t i = sp; i >= 0; i-- ){
            object = eReqs_[i];
            if( object->timeout_ == ~uint64_t(0) ) continue;
            object->timeout_ -= object->timeout_ < timeout ? object->timeout_ : timeout;
            if( object->timeout_ == 0 ){
              //closeAPI(object);
              if( object->descriptor_ != NULL ){
#ifndef NDEBUG
                BOOL cir =
#endif
                CancelIo(object->descriptor_->descriptor_);
#ifndef NDEBUG
                wm0 = GetLastError();
                assert( cir != NULL );
#endif
              }
              safeEvents_[ssp++] = events[i];
              events[i] = events[sp];
              events[sp] = NULL;
              //events[sp + 1] = NULL;
              eReqs_[i] = eReqs_[sp];
              eReqs_[sp] = NULL;
              sp--;
              requests_.remove(*object);
              assert( object->fiber_ != NULL );
              object->errno_ = WAIT_TIMEOUT;
              object->fiber_->thread()->postEvent(object);
            }
          }
          node = NULL;
          object = NULL;
        }
        else if( wm == WAIT_FAILED ){
          DWORD err = GetLastError();
          assert( 0 );
        }
        else {
          assert( 0 );
        }
      }
      if( node != NULL ){
        object->errno_ = GetLastError();
        //closeAPI(object);

        safeEvents_[ssp++] = events[wm];
        events[wm] = events[sp];
        events[sp] = NULL;
        //events[sp + 1] = NULL;
        eReqs_[wm] = eReqs_[sp];
        eReqs_[sp] = NULL;
        sp--;
        requests_.remove(*object);
        object->count_ = object->errno_ != ERROR_SUCCESS && object->errno_ != ERROR_HANDLE_EOF ? ~uint64_t(0) : nb;
        object->fiber_->thread()->postEvent(object);
      }
    }
  }
}
//------------------------------------------------------------------------------
#else
//------------------------------------------------------------------------------
void AsyncIoSlave::threadExecute()
{
//  priority(THREAD_PRIORITY_HIGHEST);
  intptr_t sp = -1;
#if SIZEOF_AIOCB
  struct aiocb * iocb;
#endif
  union {
    int nrd;
    int evCount;
  };
  int32_t error;
  uint64_t count;
  EventsNode * node;
  AsyncEvent * object = NULL;
  for(;;){
    AutoLock<InterlockedMutex> lock(*this);
    for( node = newRequests_.first(); node != NULL; node = newRequests_.first() ){
      object = &AsyncEvent::nodeObject(*node);
      //openAPI(object);
      errno = 0;
      switch( object->type_ ){
        case etDirectoryChangeNotification :
	  error = ENOSYS;
          break;
        case etLockFile :
	  error = ENOSYS;
          break;
        case etRead     :
	  if( object->length_ > 0 ){
            iocb = &object->iocb_;
            memset(iocb,0,sizeof(*iocb));
            iocb->aio_fildes = object->descriptor_->descriptor_;
            iocb->aio_nbytes = object->length_;
	    iocb->aio_buf = object->buffer_;
            iocb->aio_offset = object->position_;
#if HAVE_SIGVAL_SIVAL_PTR
	    iocb->aio_sigevent.sigev_value.sival_ptr = node; // udata
#elif HAVE_SIGVAL_SIGVAL_PTR
	    iocb->aio_sigevent.sigev_value.sigval_ptr = node; // udata
#endif
#ifdef SIGEV_KEVENT
            iocb->aio_sigevent.sigev_notify_kqueue = kqueue_;
            iocb->aio_sigevent.sigev_notify = SIGEV_KEVENT;
#endif
#if HAVE_AIO_READ
            if( aio_read(iocb) == 0 ) errno = EINPROGRESS;
#else
	    error = ENOSYS;
#endif
          }
	  else {
	    errno = EINVAL;
	  }
          break;
        case etWrite  :
	  if( object->length_ > 0 ){
            iocb = &object->iocb_;
      	    memset(iocb,0,sizeof(*iocb));
            iocb->aio_fildes = object->descriptor_->descriptor_;
            iocb->aio_nbytes = object->length_;
            iocb->aio_buf = object->buffer_;
            iocb->aio_offset = object->position_;
#if HAVE_SIGVAL_SIVAL_PTR
	    iocb->aio_sigevent.sigev_value.sival_ptr = node; // udata
#elif HAVE_SIGVAL_SIGVAL_PTR
	    iocb->aio_sigevent.sigev_value.sigval_ptr = node; // udata
#endif
#ifdef SIGEV_KEVENT
            iocb->aio_sigevent.sigev_notify_kqueue = kqueue_;
            iocb->aio_sigevent.sigev_notify = SIGEV_KEVENT;
#endif
#if HAVE_AIO_WRITE
            if( aio_write(iocb) == 0 ) errno = EINPROGRESS;
#else
	    error = ENOSYS;
#endif
	  }
	  else {
            errno = EINVAL;
	  }
          break;
        case etAccept :
#if HAVE_KQUEUE
          EV_SET(&kevents_[0],object->descriptor_->socket_,EVFILT_READ,EV_ADD | EV_ONESHOT,0,0,node);
          if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) != -1 )
            count = object->descriptor_->accept();
	  if( errno != EWOULDBLOCK ){
	    error = errno;
	    kevents_[0].flags = EV_DELETE;
            if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) == -1 && errno != ENOENT ){
              perror(NULL);
              assert( 0 );
              abort();
            }
            errno = error;
	  }
	  else {
	    errno = EINPROGRESS;
	  }
#else
          errno = ENOSYS;
#endif
          break;
        case etConnect :
	  if( object->descriptor_->socket_ < 0 ||
	      (uintptr_t) object->descriptor_->socket_ >= FD_SETSIZE ){
	    errno = EINVAL;
	  }
	  else {
	    FD_SET(object->descriptor_->socket_,rfds_);
	    FD_SET(object->descriptor_->socket_,wfds_);
            object->descriptor_->connect(object);
            if( errno != EINPROGRESS ){
    	      error = errno;
	      FD_CLR(object->descriptor_->socket_,rfds_);
	      FD_CLR(object->descriptor_->socket_,wfds_);
	      errno = error;
	    }
	  }
          break;
        default :
          assert( 0 );
      }
      if( errno == EINPROGRESS ){
        object->ioSlave_ = this;
        requests_.insToTail(newRequests_.remove(*object));
      }
      else if( errno != EINPROGRESS ){
        error = errno;
        //closeAPI(object);
        newRequests_.remove(*object);
        object->errno_ = error;
        object->count_ = ~(uint64_t) 0;
        object->fiber_->thread()->postEvent(object);
        sp--;
      }
    }
    if( requests_.count() == 0 ){
      if( terminated_ ) break;
      release();
      Semaphore::wait();
      acquire();
    }
    else {
      release();
      if( connect_ ){
	nrd = select(FD_SETSIZE,rfds_,wfds_,NULL,NULL);
      }
      else {
#if HAVE_KQUEUE
        evCount = kevent(kqueue_,NULL,0,&kevents_[0],1,NULL);
#else
        errno = ENOSYS;
#endif
      }
      error = errno;
      acquire();
      errno = error;
      if( evCount == -1 ){
        perror(NULL);
        assert( 0 );
        abort();
      }
      node = requests_.first();
      while( evCount > 0 ){
#if HAVE_KQUEUE
        struct kevent * kev = NULL;
#endif
        if( connect_ ){
	   if( node == NULL ) break;
	   object = &AsyncEvent::nodeObject(*node);
           assert( object->type_ == etConnect );
	   node = node->next();
	   if( !FD_ISSET(object->descriptor_->socket_,rfds_) &&
	       !FD_ISSET(object->descriptor_->socket_,wfds_) ){
	    FD_SET(object->descriptor_->socket_,rfds_);
	    FD_SET(object->descriptor_->socket_,wfds_);
            continue;
	  }
	}
#if HAVE_KQUEUE
	else {
          evCount--;
          kev = &kevents_[evCount];
          node = (EventsNode *) kev->udata;
  	  if( node == NULL ) continue;
	  object = &AsyncEvent::nodeObject(*node);
          if( kev->filter == EVFILT_TIMER ){
            assert( object->type_ == etAccept );
	    kev->filter = EVFILT_READ;
            kev->flags |= EV_ERROR;
            kev->data = EINTR;
          }
        }
#endif
        socklen_t errLen;
	error = 0;
	count = 0;
        switch( object->type_ ){
          case etRead    :
#if HAVE_KQUEUE
              assert( kev->filter == EVFILT_AIO );
              error = aio_error(&object->iocb_);
              assert( error != EINPROGRESS );
              count = aio_return(&object->iocb_);
#endif
	    break;
          case etWrite   :
#if HAVE_KQUEUE
	      assert( kev->filter == EVFILT_AIO );
              error = aio_error(&object->iocb_);
              assert( error != EINPROGRESS );
              count = aio_return(&object->iocb_);
#endif
	    break;
          case etAccept  :
#if HAVE_KQUEUE
            assert( kev->filter == EVFILT_READ );
            if( kev->flags & EV_ERROR ){
              error = kev->data;
              count = ~uint64_t(0);
            }
            else {
	      count = object->descriptor_->accept();
	      error = errno;
	    }
#endif
            break;
          case etConnect :
	    errLen = sizeof(error);
	    dynamic_cast<ksock::AsyncSocket *>(object->descriptor_)->getsockopt(SOL_SOCKET,SO_ERROR,&error,errLen);
            break;
          default        :
            assert( 0 );
        }
	      //closeAPI(object);
        requests_.remove(*object);
        object->ioSlave_ = NULL;
        object->errno_ = error;
        object->count_ = count;
        object->fiber_->thread()->postEvent(object);
      }
    }
  }
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
bool AsyncIoSlave::abortNotification(DirectoryChangeNotification * dcn)
{
  bool r = false;
#if defined(__WIN32__) || defined(__WIN64__)
  assert( !isWin9x() );
  AutoLock<InterlockedMutex> lock(*this);
  EventsNode * node;
  for( node = requests_.first(); node != NULL; node = node->next() ){
    AsyncEvent & object = AsyncEvent::nodeObject(*node);
    if( object.type_ == etDirectoryChangeNotification ){
      r = object.directoryChangeNotification_ == dcn;
      if( dcn == NULL || r ){
        object.abort_ = true;
        SetEvent(object.overlapped_.hEvent);
        post();
        if( r ) break;
      }
    }
  }
#endif
  return r;
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncMiscSlave::SocketInitializer::~SocketInitializer()
{
  ksock::api.close();
}
//------------------------------------------------------------------------------
AsyncMiscSlave::SocketInitializer::SocketInitializer()
{
  ksock::api.open();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncMiscSlave::~AsyncMiscSlave()
{
}
//------------------------------------------------------------------------------
AsyncMiscSlave::AsyncMiscSlave() : maxRequests_(64)
{
}
//------------------------------------------------------------------------------
void AsyncMiscSlave::threadBeforeWait()
{
  terminate();
  post();
}
//---------------------------------------------------------------------------
bool AsyncMiscSlave::transplant(AsyncEvent & request)
{
  bool r = false;
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
    if( requests_.count() < maxRequests_ ){
      requests_.insToTail(request);
      if( requests_.count() < 2 ) post();
      r = true;
    }
  }
  return r;
}
//------------------------------------------------------------------------------
void AsyncMiscSlave::threadExecute()
{
//  priority(THREAD_PRIORITY_HIGHEST);
//  priority(THREAD_PRIORITY_LOWEST);
  AsyncEvent * request;
  for(;;){    
    acquire();
    request = requests_.count() > 0 ? &requests_.remove(*requests_.first()) : NULL;
    release();
    if( request == NULL ){
      if( terminated_ ) break;
      Semaphore::wait();
    }
    else {
      if( request->type_ == etOpenFile ){
        int32_t err = 0;
        try {
          request->fileDescriptor_ = request->file_->openHelper(true);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etDirList ){
        int32_t err = 0;
        try {
          getDirList(
            *request->dirList_,
            request->string0_,
            request->string1_,
            request->recursive_,
            request->includeDirs_,
            request->exMaskAsList_
          );
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etCreateDir ){
        int32_t err = 0;
        try {
          request->rval_ = createDirectory(request->string0_);
          err = oserror();
          if( err != 0 ) err += errorOffset;
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etRemoveDir ){
        int32_t err = 0;
        try {
          request->rval_ = removeDirectory(request->string0_,request->recursive_);
          err = oserror();
          if( err != 0 ) err += errorOffset;
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etRemoveFile ){
        int32_t err = 0;
        try {
          request->rval_ = remove(request->string0_);
          err = oserror();
          if( err != 0 ) err += errorOffset;
        }
        catch( ExceptionSP & e ){
          err = e->code();
          request->rval_ = false;
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etResolveName ){
        int32_t err = 0;
        try {
          request->address_.resolveName(request->string0_,request->string1_,request->aiFlag_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etResolveAddress ){
        int32_t err = 0;
        try {
          request->string0_ = request->address_.resolveAddr(request->defPort_,request->aiFlag_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etStat ){
        int32_t err = 0;
        try {
          request->rval_ = stat(request->string0_,*request->stat_);
          err = oserror();
          if( err != 0 ) err += errorOffset;
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etRename ){
        int32_t err = 0;
        try {
          rename(request->string0_,request->string1_,request->createIfNotExist_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etCopy ){
        int32_t err = 0;
        try {
          copy(request->string0_,request->string1_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else {
        assert( 0 );
      }
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncProcessSlave::~AsyncProcessSlave()
{
}
//------------------------------------------------------------------------------
AsyncProcessSlave::AsyncProcessSlave() : maxRequests_(1)
{
}
//------------------------------------------------------------------------------
void AsyncProcessSlave::threadBeforeWait()
{
  terminate();
  post();
}
//---------------------------------------------------------------------------
bool AsyncProcessSlave::transplant(AsyncEvent & request)
{
  bool r = false;
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
    if( requests_.count() < maxRequests_ ){
      requests_.insToTail(request);
      post();
      r = true;
    }
  }
  return r;
}
//------------------------------------------------------------------------------
void AsyncProcessSlave::threadExecute()
{
//  priority(THREAD_PRIORITY_HIGHEST);
//  priority(THREAD_PRIORITY_LOWEST);
  AsyncEvent * request;
  for(;;){    
    acquire();
    request = requests_.count() > 0 ? &requests_.remove(*requests_.first()) : NULL;
    release();
    if( request == NULL ){
      if( terminated_ ) break;
      Semaphore::wait();
    }
    else {
      if( request->type_ == etExec ){
        int32_t err = 0;
        try {
          request->data_ = execute(*request->executeParameters_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etWaitForProcess ){
        int32_t err = 0;
        try {
          request->data_ = waitForProcess(request->pid_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else {
        assert( 0 );
      }
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncTimerSlave::~AsyncTimerSlave()
{
}
//------------------------------------------------------------------------------
AsyncTimerSlave::AsyncTimerSlave()
{
}
//------------------------------------------------------------------------------
void AsyncTimerSlave::transplant(AsyncEvent & request)
{
  AutoLock<InterlockedMutex> lock(*this);
  requests_.insToTail(request);
  post();
}
//------------------------------------------------------------------------------
void AsyncTimerSlave::threadBeforeWait()
{
  terminate();
  post();
}
//---------------------------------------------------------------------------
void AsyncTimerSlave::threadExecute()
{
//  priority(THREAD_PRIORITY_TIME_CRITICAL);
//  priority(THREAD_PRIORITY_LOWEST);
  uint64_t minTimeout, timerStartTime, elapsedTime, currentTime;
  EventsNode * requestNode;
  AsyncEvent * request, * minRequest;
  for(;;){
    minRequest = NULL;
    minTimeout = ~uint64_t(0);
    acquire();
    requestNode = requests_.first();
    while( requestNode != NULL ){
      request = &AsyncEvent::nodeObject(*requestNode);
      if( request->timeout_ < minTimeout ){
        minRequest = request;
        minTimeout = request->timeout_;
      }
      requestNode = requestNode->next();
    }
    release();
    if( minRequest == NULL ){
      if( terminated_ ) break;
      Semaphore::wait();
    }
    else {
      assert( minRequest->type_ == etTimer );
      timerStartTime = gettimeofday();
      timedWait(minRequest->timeout_);
      AutoLock<InterlockedMutex> lock(*this);
      requestNode = requests_.first();
      while( requestNode != NULL ){
        elapsedTime = (currentTime = gettimeofday()) - timerStartTime;
        request = &AsyncEvent::nodeObject(*requestNode);
        if( request->timerStartTime_ <= currentTime )
          request->timeout_ -= request->timeout_ >= elapsedTime ? elapsedTime : request->timeout_;
        if( request->timeout_ == 0 || request->abort_ ){
          assert( request->fiber_ != NULL );
          request->errno_ = request->abort_ ?
#if defined(__WIN32__) || defined(__WIN64__)
            ERROR_REQUEST_ABORTED
#else
            EINTR
#endif
            : 0;
          requestNode = requestNode->next();
          requests_.remove(*request);
          request->fiber_->thread()->postEvent(request);
        }
        else {
          requestNode = requestNode->next();
        }
      }
    }
  }
}
//------------------------------------------------------------------------------
void AsyncTimerSlave::abortTimer()
{
  AutoLock<InterlockedMutex> lock(*this);
  EventsNode * requestNode = requests_.first();
  while( requestNode != NULL ){
    AsyncEvent::nodeObject(*requestNode).abort_ = true;
    requestNode = requestNode->next();
  }
  post();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncAcquireSlave::~AsyncAcquireSlave()
{
#if defined(__WIN32__) || defined(__WIN64__)
  if( sems_[MAXIMUM_WAIT_OBJECTS - 1] != NULL )
    CloseHandle(sems_[MAXIMUM_WAIT_OBJECTS - 1]);
#endif
}
//------------------------------------------------------------------------------
AsyncAcquireSlave::AsyncAcquireSlave()
{
#if defined(__WIN32__) || defined(__WIN64__)
  intptr_t i;
  for( i = sizeof(eSems_) / sizeof(eSems_[0]) - 1; i >= 0; i-- ) eSems_[i] = NULL;
  for( i = sizeof(sems_) / sizeof(sems_[0]) - 1; i >= 0; i-- ) sems_[i] = NULL;
  if( (sems_[MAXIMUM_WAIT_OBJECTS - 1] = CreateEventA(NULL,TRUE,FALSE,NULL)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//------------------------------------------------------------------------------
void AsyncAcquireSlave::threadBeforeWait()
{
  terminate();
  post();
}
//---------------------------------------------------------------------------
bool AsyncAcquireSlave::transplant(AsyncEvent & request)
{
  bool r = false;
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
#if defined(__WIN32__) || defined(__WIN64__)
    if( requests_.count() + newRequests_.count() < MAXIMUM_WAIT_OBJECTS - 1 ){
      newRequests_.insToTail(request);
      BOOL es = SetEvent(sems_[MAXIMUM_WAIT_OBJECTS - 1]);
      DWORD err = GetLastError();
      assert( es != 0 );
      if( requests_.count() == 0 ) post();
      r = true;
    }
#else
    requests_.insToTail(request);
    post();
    r = true;
#endif
  }
  return r;
}
//------------------------------------------------------------------------------
void AsyncAcquireSlave::threadExecute()
{
//  priority(THREAD_PRIORITY_TIME_CRITICAL);
//  priority(THREAD_PRIORITY_LOWEST);
#if defined(__WIN32__) || defined(__WIN64__)
  AsyncEvent * object;
  EventsNode * node;
  intptr_t sp = -1;
  for(;;){
    AutoLock<InterlockedMutex> lock(*this);
    for(;;){
      node = newRequests_.first();
      if( node == NULL ) break;
      object = &AsyncEvent::nodeObject(*node);
      assert( sp < MAXIMUM_WAIT_OBJECTS - 1 );
      sp++;
      if( object->type_ == etAcquireMutex ) sems_[sp] = object->mutex_->sem_;
      else
      if( object->type_ == etAcquireSemaphore ) sems_[sp] = object->semaphore_->handle_;
      eSems_[sp] = object;
      requests_.insToTail(newRequests_.remove(*node));
    }
    if( requests_.count() == 0 ){
      if( terminated_ ) break;
      release();
      Semaphore::wait();
      acquire();
    }
    else {
      DWORD tma = INFINITE, wm0 = WAIT_OBJECT_0;
      uint64_t timeout = ~uint64_t(0);
      for( node = requests_.first(); node != NULL; node = node->next() ){
        object = &AsyncEvent::nodeObject(*node);
        if( object->timeout_ < timeout ){
          timeout = object->timeout_;
          tma = DWORD(timeout / 1000u);
        }
      }
      release();
      object = NULL;
      node = NULL;
      timeout = gettimeofday();
      sems_[sp + 1] = sems_[MAXIMUM_WAIT_OBJECTS - 1];
      DWORD wm = WaitForMultipleObjectsEx(DWORD(sp + 2),sems_,FALSE,tma,TRUE);
      acquire();
      if( wm >= wm0 && wm < WAIT_OBJECT_0 + sp + 1 ){
        wm -= WAIT_OBJECT_0;
        object = eSems_[wm];
        assert( object != NULL );
        node = &AsyncEvent::node(*object);
      }
      else if( wm >= WAIT_ABANDONED_0 && wm < WAIT_ABANDONED_0 + sp + 1 ){
        wm -= WAIT_ABANDONED_0;
        object = eSems_[wm];
        assert( object != NULL );
        node = &AsyncEvent::node(*object);
      }
      else if( wm == WAIT_OBJECT_0 + sp + 1 ){
        ResetEvent(sems_[wm]);
      }
      else if( wm == WAIT_ABANDONED_0 + sp + 1 ){
        ResetEvent(sems_[wm]);
      }
      else if( wm == WAIT_TIMEOUT ){
        timeout = gettimeofday() - timeout;
        for( intptr_t i = sp; i >= 0; i-- ){
          object = eSems_[i];
          if( object->timeout_ == ~uint64_t(0) ) continue;
          object->timeout_ -= object->timeout_ < timeout ? object->timeout_ : timeout;
          if( object->timeout_ == 0 ){
            sems_[i] = sems_[sp];
            sems_[sp] = NULL;
            //sems_[sp + 1] = NULL;
            eSems_[i] = eSems_[sp];
            eSems_[sp] = NULL;
            sp--;
            requests_.remove(*object);
            assert( object->fiber_ != NULL );
            object->errno_ = WAIT_TIMEOUT;
            object->fiber_->thread()->postEvent(object);
          }
        }
        node = NULL;
        object = NULL;
      }
      else if( wm == WAIT_FAILED ){
        DWORD err = GetLastError();
        assert( 0 );
      }
      else {
        assert( 0 );
      }
      if( node != NULL ){
        sems_[wm] = sems_[sp];
        sems_[sp] = NULL;
        //sems_[sp + 1] = NULL;
        eSems_[wm] = eSems_[sp];
        eSems_[sp] = NULL;
        sp--;
        requests_.remove(*object);
        assert( object->fiber_ != NULL );
        object->errno_ = 0;
        object->fiber_->thread()->postEvent(object);
      }
    }
  }
#else
  for(;;){
    AsyncEvent * request = NULL;
    acquire();
    if( requests_.count() > 0 ) request = &requests_.remove(*requests_.first());
    release();
    if( request == NULL ){
      if( terminated_ ) break;
      Semaphore::wait();
    }
    else {
      if( request->type_ == etAcquireMutex ){
        request->errno_ = 0;
        try {
          request->mutex_->acquire();
        }
        catch( ExceptionSP & e ){
          request->errno_ = e->code();
        }
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else if( request->type_ == etAcquireSemaphore ){
        request->errno_ = 0;
        try {
          request->semaphore_->timedWait(request->timeout_);
        }
        catch( ExceptionSP & e ){
          request->errno_ = e->code();
        }
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
      }
      else {
        assert( 0 );
      }
    }
  }
#endif
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//------------------------------------------------------------------------------
AsyncWin9xDirectoryChangeNotificationSlave::~AsyncWin9xDirectoryChangeNotificationSlave()
{
  if( sems_[MAXIMUM_WAIT_OBJECTS - 1] != NULL )
    CloseHandle(sems_[MAXIMUM_WAIT_OBJECTS - 1]);
}
//------------------------------------------------------------------------------
AsyncWin9xDirectoryChangeNotificationSlave::AsyncWin9xDirectoryChangeNotificationSlave() : sp_(-1)
{
  intptr_t i;
  for( i = sizeof(eSems_) / sizeof(eSems_[0]) - 1; i >= 0; i-- ) eSems_[i] = NULL;
  for( i = sizeof(sems_) / sizeof(sems_[0]) - 1; i >= 0; i-- ) sems_[i] = NULL;
  if( (sems_[MAXIMUM_WAIT_OBJECTS - 1] = CreateEventA(NULL,TRUE,FALSE,NULL)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
}
//------------------------------------------------------------------------------
void AsyncWin9xDirectoryChangeNotificationSlave::threadBeforeWait()
{
  terminate();
  post();
}
//---------------------------------------------------------------------------
bool AsyncWin9xDirectoryChangeNotificationSlave::transplant(AsyncEvent & request)
{
  intptr_t i;
  bool r = false;
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
    if( !terminated_ && requests_.count() + newRequests_.count() < MAXIMUM_WAIT_OBJECTS - 2 ){
      for( i = sp_; i >= 0; i-- ) if( sems_[i] == request.directoryChangeNotification_->hFFCNotification() ) break;
      if( i < 0 ){
        newRequests_.insToTail(request);
        if( sp_ >= 0 ) SetEvent(sems_[MAXIMUM_WAIT_OBJECTS - 1]);
        post();
        r = true;
      }
    }
  }
  return r;
}
//------------------------------------------------------------------------------
void AsyncWin9xDirectoryChangeNotificationSlave::threadExecute()
{
//  priority(THREAD_PRIORITY_TIME_CRITICAL);
  AsyncEvent * object;
  EventsNode * node;
  for(;;){
    AutoLock<InterlockedMutex> lock(*this);
    for(;;){
      node = newRequests_.first();
      if( node == NULL ) break;
      object = &AsyncEvent::nodeObject(*node);
      assert( object->type_ == etAcquireMutex );
      assert( sp_ < MAXIMUM_WAIT_OBJECTS - 1 );
      ++sp_;
      sems_[sp_] = object->directoryChangeNotification_->hFFCNotification();
      eSems_[sp_] = object;
      sems_[sp_ + 1] = sems_[MAXIMUM_WAIT_OBJECTS - 1];
      requests_.insToTail(newRequests_.remove(*node));
    }
    if( requests_.count() == 0 ){
      if( terminated_ ) break;
      release();
      Semaphore::wait();
      acquire();
    }
    else {
      release();
      object = NULL;
      node = NULL;
      DWORD wm0, wm = WaitForMultipleObjectsEx(DWORD(sp_ + 2),sems_,FALSE,INFINITE,FALSE);
      acquire();
      wm0 = WAIT_OBJECT_0;
      if( wm >= wm0 && wm < WAIT_OBJECT_0 + sp_ + 1 ){
        wm -= WAIT_OBJECT_0;
        assert( wm < MAXIMUM_WAIT_OBJECTS - 1 );
        object = eSems_[wm];
        assert( object != NULL );
        node = &AsyncEvent::node(*object);
      }
      else if( wm >= STATUS_ABANDONED_WAIT_0 && wm < STATUS_ABANDONED_WAIT_0 + sp_ + 1 ){
        wm -= STATUS_ABANDONED_WAIT_0;
        assert( wm < MAXIMUM_WAIT_OBJECTS - 1 );
        object = eSems_[wm];
        assert( object != NULL );
        node = &AsyncEvent::node(*object);
      }
      else if( wm == WAIT_OBJECT_0 + sp_ + 1 ){
        ResetEvent(sems_[WAIT_OBJECT_0 + sp_ + 1]);
      }
      else if( wm == STATUS_ABANDONED_WAIT_0 + sp_ + 1 ){
        ResetEvent(sems_[STATUS_ABANDONED_WAIT_0 + sp_ + 1]);
      }
      else {
        assert( 0 );
      }
      if( node != NULL ){
        xchg(sems_[wm],sems_[sp_]);
        sems_[sp_] = sems_[MAXIMUM_WAIT_OBJECTS - 1];
        eSems_[wm] = eSems_[sp_];
        eSems_[sp_] = NULL;
        sp_--;
        requests_.remove(*node);
        assert( object->fiber_ != NULL );
        object->errno_ = object->abort_ ? ERROR_REQUEST_ABORTED : 0;
        object->fiber_->thread()->postEvent(object);
      }
    }
  }
}
//------------------------------------------------------------------------------
bool AsyncWin9xDirectoryChangeNotificationSlave::abortNotification(DirectoryChangeNotification * dcn)
{
  bool r = false;
  assert( !isWin9x() );
  AutoLock<InterlockedMutex> lock(*this);
  EventsNode * node;
  for( node = requests_.first(); node != NULL; node = node->next() ){
    AsyncEvent & object = AsyncEvent::nodeObject(*node);
    if( object.type_ == etDirectoryChangeNotification ){
      r = object.directoryChangeNotification_ == dcn;
      if( dcn == NULL || r ){
        object.abort_ = true;
        object.directoryChangeNotification_->stop();
        post();
        if( r ) break;
      }
    }
  }
  return r;
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef NDEBUG
//------------------------------------------------------------------------------
AsyncStackBackTraceSlave::~AsyncStackBackTraceSlave()
{
}
//------------------------------------------------------------------------------
AsyncStackBackTraceSlave::AsyncStackBackTraceSlave()
{
}
//------------------------------------------------------------------------------
void AsyncStackBackTraceSlave::threadBeforeWait()
{
  terminate();
  post();
}
//---------------------------------------------------------------------------
void AsyncStackBackTraceSlave::transplant(AsyncEvent & request)
{
  AutoLock<InterlockedMutex> lock(*this);
  requests_.insToTail(request);
  post();
}
//------------------------------------------------------------------------------
void AsyncStackBackTraceSlave::threadExecute()
{
//  priority(THREAD_PRIORITY_TIME_CRITICAL);
  AsyncEvent * request;
  for(;;){
    acquire();
    request = NULL;
    if( requests_.count() > 0 ){
      request = &AsyncEvent::nodeObject(*requests_.first());
      requests_.remove(*request);
    }
    release();
    if( request == NULL ){
      if( terminated_ ) break;
      Semaphore::wait();
    }
    else {
      assert( request->type_ == etStackBackTrace || request->type_ == etStackBackTraceZero );
      request->errno_ = 0;
      HANDLE threadHandle;
      switch( request->type_ ){
        case etStackBackTrace :
          assert( request->thread_ != NULL );
          if( request->mutex0_ == NULL ) request->thread_->suspend();
          threadHandle = OpenThread(THREAD_ALL_ACCESS,FALSE,(DWORD) request->thread_->id());
          if( threadHandle != NULL ){
            request->string0_ =// DBGSTRING2CHARPTR(
              pdbutils::getBackTrace(
                pdbutils::DbgFrameGetAll,
                request->data1_,
                threadHandle
              );
            CloseHandle(threadHandle);
          }
          if( request->mutex0_ == NULL ) request->thread_->resume();
          if( request->mutex0_ == NULL ){
            request->fiber_->thread()->postEvent(request);
          }
          else {
            request->mutex0_->release();
          }
          break;
        case etStackBackTraceZero :
          while( !Thread::isSuspended(request->tid_) ) ksleep1();
          threadHandle = OpenThread(THREAD_ALL_ACCESS,FALSE,(DWORD) request->tid_);
          if( threadHandle != NULL ){
//          result = SuspendThread((HANDLE) request->threadHandle_);
//          if( result == (DWORD) -1 ) exit(GetLastError());
            request->string0_ =// DBGSTRING2CHARPTR(
              pdbutils::getBackTrace(
                pdbutils::DbgFrameGetAll,
                request->data1_,
                threadHandle
                );
//          result = ResumeThread((HANDLE) request->threadHandle_);
//          if( result == (DWORD) -1 ) exit(GetLastError());
            CloseHandle(threadHandle);
          }
          request->mutex0_->release();
          break;
        default:
          assert( 0 );
      }
    }
  }
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
#endif // if defined(__WIN32__) || defined(__WIN64__)
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Requester::~Requester()
{
//  intptr_t i;

  abort();
/*  for( i = ioSlaves_.count() - 1; i >= 0; i-- ){
    ioSlaves_[i].terminate();
    ioSlaves_[i].post();
    ioSlaves_[i].Thread::wait();
  }*/
#if !defined(__WIN32__) && !defined(__WIN64__)
/*  for( i = connectSlaves_.count() - 1; i >= 0; i-- ){
    connectSlaves_[i].terminate();
    connectSlaves_[i].post();
    connectSlaves_[i].Thread::wait();
  }*/
#endif
/*  for( i = ofSlaves_.count() - 1; i >= 0; i-- ){
    ofSlaves_[i].terminate();
    ofSlaves_[i].post();
    ofSlaves_[i].Thread::wait();
  }*/
/*  for( i = acquireSlaves_.count() - 1; i >= 0; i-- ){
    acquireSlaves_[i].terminate();
    acquireSlaves_[i].post();
    acquireSlaves_[i].Thread::wait();
  }*/
#if defined(__WIN32__) || defined(__WIN64__)
/*  for( i = wdcnSlaves_.count() - 1; i >= 0; i-- ){
    wdcnSlaves_[i].terminate();
    wdcnSlaves_[i].post();
    wdcnSlaves_[i].Thread::wait();
  }*/
#ifndef NDEBUG
/*  if( asyncStackBackTraceSlave_ != NULL ){
    asyncStackBackTraceSlave_->terminate();
    asyncStackBackTraceSlave_->post();
    asyncStackBackTraceSlave_->Thread::wait();
    asyncStackBackTraceSlave_ = NULL;
  }*/
#endif
#endif
}
//---------------------------------------------------------------------------
Requester::Requester() :
  ioSlavesSweepTime_(0),
#if !defined(__WIN32__) && !defined(__WIN64__)
  connectSlavesSweepTime_(0),
#endif
  ofSlavesSweepTime_(0),
  prSlavesSweepTime_(0),
  acquireSlavesSweepTime_(0)
#if defined(__WIN32__) || defined(__WIN64__)
  , wdcnSlavesSweepTime_(0)
#endif
{
}
//---------------------------------------------------------------------------
void Requester::abort()
{
  {
    AutoLock<InterlockedMutex> lock(timerRequestsMutex_);
    if( timerSlave_ != NULL ){
      timerSlave_->terminate();
      timerSlave_->abortTimer();
      timerSlave_->post();
      timerSlave_->Thread::wait();
      timerSlave_ = NULL;
    }
  }
  abortNotification();
}
//---------------------------------------------------------------------------
bool Requester::abortNotification(DirectoryChangeNotification * dcn)
{
  bool r = false;
#if defined(__WIN32__) || defined(__WIN64__)
  intptr_t i;
  if( isWin9x() ){
    AutoLock<InterlockedMutex> lock(wdcnRequestsMutex_);
    for( i = wdcnSlaves_.count() - 1; i >= 0; i-- ){
      wdcnSlaves_[i].terminate();
      r = wdcnSlaves_[i].abortNotification(dcn);
      wdcnSlaves_[i].post();
      wdcnSlaves_[i].Thread::wait();
      wdcnSlaves_.remove(i);
      if( r ) break;
    }
  }
  else {
    AutoLock<InterlockedMutex> lock(ioRequestsMutex_);
    for( i = ioSlaves_.count() - 1; i >= 0; i-- ){
      r = ioSlaves_[i].abortNotification(dcn);
      if( r ) break;
    }
  }
#endif
  return r;
}
//---------------------------------------------------------------------------
void Requester::postRequest(AsyncDescriptor * descriptor)
{
  intptr_t i;
  Fiber * fiber = currentFiber();
  assert( fiber != NULL );
  fiber->event_.descriptor_ = descriptor;
  switch( fiber->event_.type_ ){
    case etNone :
    case etError : 
      break;
    case etOpenFile :
    case etDirList :
    case etCreateDir :
    case etRemoveDir :
    case etRemoveFile :
    case etRename :
    case etCopy :
    case etResolveName :
    case etResolveAddress :
    case etStat :
      {
        AutoLock<InterlockedMutex> lock(ofRequestsMutex_);
        if( gettimeofday() - ofSlavesSweepTime_ >= 10000000 ){
          for( i = ofSlaves_.count() - 1; i >= 0; i-- )
            if( ofSlaves_[i].finished() ) ofSlaves_.remove(i);
          ofSlavesSweepTime_ = gettimeofday();
        }
        for( i = ofSlaves_.count() - 1; i >= 0; i-- )
          if( ofSlaves_[i].transplant(fiber->event_) ) break;
        if( i < 0 ){
          AsyncMiscSlave * p = newObject<AsyncMiscSlave>();
          AutoPtr<AsyncMiscSlave> slave(p);
          p->resume();
          ofSlaves_.add(slave.ptr());
          slave.ptr(NULL);
          p->transplant(fiber->event_);
          if( ofSlaves_.count() > numberOfProcessors() * 4 ) p->terminate();
        }
      }
      return;
    case etExec :
    case etWaitForProcess :
      {
        AutoLock<InterlockedMutex> lock(prRequestsMutex_);
        if( gettimeofday() - prSlavesSweepTime_ >= 10000000 ){
          for( i = prSlaves_.count() - 1; i >= 0; i-- )
            if( prSlaves_[i].finished() ) prSlaves_.remove(i);
          prSlavesSweepTime_ = gettimeofday();
        }
        for( i = prSlaves_.count() - 1; i >= 0; i-- )
          if( prSlaves_[i].transplant(fiber->event_) ) break;
        if( i < 0 ){
          AsyncProcessSlave * p = newObject<AsyncProcessSlave>();
          AutoPtr<AsyncProcessSlave> slave(p);
          p->resume();
          prSlaves_.add(slave.ptr());
          slave.ptr(NULL);
          p->transplant(fiber->event_);
          if( prSlaves_.count() > numberOfProcessors() * 4 ) p->terminate();
        }
      }
      return;
    case etDirectoryChangeNotification :
#if defined(__WIN32__) || defined(__WIN64__)
      if( isWin9x() ){
        AutoLock<InterlockedMutex> lock(wdcnRequestsMutex_);
        if( gettimeofday() - wdcnSlavesSweepTime_ >= 10000000 ){
          for( i = wdcnSlaves_.count() - 1; i >= 0; i-- )
            if( wdcnSlaves_[i].finished() ) wdcnSlaves_.remove(i);
          wdcnSlavesSweepTime_ = gettimeofday();
        }
        for( i = wdcnSlaves_.count() - 1; i >= 0; i-- )
          if( wdcnSlaves_[i].transplant(fiber->event_) ) break;
        if( i < 0 ){
          AsyncWin9xDirectoryChangeNotificationSlave * p = newObject<AsyncWin9xDirectoryChangeNotificationSlave>();
          AutoPtr<AsyncWin9xDirectoryChangeNotificationSlave> slave(p);
          p->resume();
          wdcnSlaves_.add(slave.ptr());
          slave.ptr(NULL);
          p->transplant(fiber->event_);
          if( wdcnSlaves_.count() > numberOfProcessors() * 4 ) p->terminate();
        }
        return;
      }
      // walk through
#endif
    case etWaitCommEvent :
    case etLockFile :
    case etRead :
    case etWrite :
    case etAccept :
#if defined(__WIN32__) || defined(__WIN64__)
    case etConnect :
#endif
      {
        AutoLock<InterlockedMutex> lock(ioRequestsMutex_);
        if( gettimeofday() - ioSlavesSweepTime_ >= 10000000 ){
          for( i = ioSlaves_.count() - 1; i >= 0; i-- )
            if( ioSlaves_[i].finished() ) ioSlaves_.remove(i);
          ioSlavesSweepTime_ = gettimeofday();
        }
        for( i = ioSlaves_.count() - 1; i >= 0; i-- )
          if( ioSlaves_[i].transplant(fiber->event_) ) break;
        if( i < 0 ){
          AsyncIoSlave * p = newObject<AsyncIoSlave>();
          AutoPtr<AsyncIoSlave> slave(p);
          p->resume();
          ioSlaves_.add(slave.ptr());
          slave.ptr(NULL);
          p->transplant(fiber->event_);
          if( ioSlaves_.count() > numberOfProcessors() * 4 ) p->terminate();
        }
      }
      return;
#if !defined(__WIN32__) && !defined(__WIN64__)
    case etConnect :
      {
        AutoLock<InterlockedMutex> lock(connectRequestsMutex_);
        if( gettimeofday() - connectSlavesSweepTime_ >= 10000000 ){
          for( i = connectSlaves_.count() - 1; i >= 0; i-- )
            if( connectSlaves_[i].finished() ) connectSlaves_.remove(i);
          connectSlavesSweepTime_ = gettimeofday();
        }
        for( i = connectSlaves_.count() - 1; i >= 0; i-- )
          if( connectSlaves_[i].transplant(fiber->event_) ) break;
        if( i < 0 ){
          AsyncIoSlave * p = newObjectV1<AsyncIoSlave>(true);
          AutoPtr<AsyncIoSlave> slave(p);
          p->resume();
          connectSlaves_.add(slave.ptr());
          slave.ptr(NULL);
          p->transplant(fiber->event_);
          if( connectSlaves_.count() > numberOfProcessors() * 4 ) p->terminate();
        }
      }
      return;
#endif
    case etClose :
    case etQuit :
    case etDispatch :
      break;
    case etTimer :
      {
        AutoLock<InterlockedMutex> lock(timerRequestsMutex_);
        if( timerSlave_ == NULL ){
          AutoPtr<AsyncTimerSlave> slave(newObject<AsyncTimerSlave>());
          slave->resume();
          timerSlave_ = slave.ptr(NULL);
        }
        timerSlave_->transplant(fiber->event_);
      }
      return;
    case etAcquireMutex :
    case etAcquireSemaphore :
      {
        AutoLock<InterlockedMutex> lock(acquireRequestsMutex_);
        if( gettimeofday() - acquireSlavesSweepTime_ >= 10000000 ){
          for( i = acquireSlaves_.count() - 1; i >= 0; i-- )
            if( acquireSlaves_[i].finished() ) acquireSlaves_.remove(i);
          acquireSlavesSweepTime_ = gettimeofday();
        }
        for( i = acquireSlaves_.count() - 1; i >= 0; i-- )
          if( acquireSlaves_[i].transplant(fiber->event_) ) break;
        if( i < 0 ){
          AsyncAcquireSlave * p = newObject<AsyncAcquireSlave>();
          AutoPtr<AsyncAcquireSlave> slave(p);
          p->resume();
          acquireSlaves_.add(slave.ptr());
          slave.ptr(NULL);
          p->transplant(fiber->event_);
          if( acquireSlaves_.count() > numberOfProcessors() * 4 ) p->terminate();
        }
      }
      return;
#if defined(__WIN32__) || defined(__WIN64__)
#ifndef NDEBUG
    case etStackBackTrace :
      {
        AutoLock<InterlockedMutex> lock(asyncStackBackTraceSlaveMutex_);
        if( asyncStackBackTraceSlave_ == NULL ){
          AutoPtr<AsyncStackBackTraceSlave> p(newObject<AsyncStackBackTraceSlave>());
          p->resume();
          asyncStackBackTraceSlave_.xchg(p);
        }
        asyncStackBackTraceSlave_->transplant(fiber->event_);
      }
      return;
#endif
#endif
    default :;
  }
  newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
void Requester::postRequest(AsyncEvent * event)
{
#if defined(__WIN32__) || defined(__WIN64__)
#ifndef NDEBUG
  switch( event->type_ ){
    case etStackBackTraceZero :
      {
        AutoLock<InterlockedMutex> lock(asyncStackBackTraceSlaveMutex_);
        if( asyncStackBackTraceSlave_ == NULL ){
          AutoPtr<AsyncStackBackTraceSlave> p(newObject<AsyncStackBackTraceSlave>());
          p->resume();
          asyncStackBackTraceSlave_.xchg(p);
        }
        asyncStackBackTraceSlave_->transplant(*event);
      }
      return;
    default :;
  }
#endif
#endif
  newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
uint8_t BaseThread::requester_[sizeof(Requester)];
//------------------------------------------------------------------------------
//VOID CALLBACK AsyncDescriptorsCluster::fileIOCompletionRoutine(
//  DWORD dwErrorCode,	// completion code
//  DWORD dwNumberOfBytesTransfered,	// number of bytes transferred
//  LPOVERLAPPED lpOverlapped) 	// pointer to structure with I/O information
//{
//  AsyncDescriptor * descriptor = static_cast<KOVERLAPPED *>(lpOverlapped)->file_;
//  ENTER_MUTEX_SECTION(mutex());
//  descriptor = descriptors().objectOfKey(Key<AsyncDescriptorKey>((HANDLE) lpOverlapped->hEvent));
//  LEAVE_MUTEX_SECTION;
//  assert( descriptor != NULL );
//  descriptor->cluster_->postEvent(
//    descriptor,
//    dwErrorCode,
//    dwErrorCode == ERROR_SUCCESS ? etReadWrite : etError,
//    dwErrorCode == ERROR_SUCCESS ? dwNumberOfBytesTransfered : -1
//  );
//}
//---------------------------------------------------------------------------
//VOID CALLBACK AsyncDescriptorsCluster::socketIOCompletionRoutine(
//  DWORD dwError,//  DWORD cbTransferred,//  LPWSAOVERLAPPED lpOverlapped,//  DWORD /*dwFlags*/)//{//  AsyncDescriptor * descriptor = static_cast<KWSAOVERLAPPED *>(lpOverlapped)->socket_;//  ENTER_MUTEX_SECTION(mutex());
//  descriptor = descriptors().objectOfKey(Key<AsyncDescriptorKey>((SOCKET) lpOverlapped->hEvent));
//  LEAVE_MUTEX_SECTION;
//  assert( descriptor != NULL );
//  descriptor->cluster_->postEvent(
//    descriptor,
//    dwError,
//    dwError == ERROR_SUCCESS ? etReadWrite : etError,
//    dwError == ERROR_SUCCESS ? cbTransferred : -1
//  );
//}//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
#if HAVE_SIGNAL_H
//typedef struct __siginfo {
//        int     si_signo;               /* signal number */
//        int     si_errno;               /* errno association */
        /*
         * Cause of signal, one of the SI_ macros or signal-specific
         * values, i.e. one of the FPE_... values for SIGFPE.  This
         * value is equivalent to the second argument to an old-style
         * FreeBSD signal handler.
         */
//        int     si_code;                /* signal code */
//        __pid_t si_pid;                 /* sending process */
//        __uid_t si_uid;                 /* sender's ruid */
//        int     si_status;              /* exit value */
//        void    *si_addr;               /* faulting instruction */
//        union sigval si_value;          /* signal value */
//        long    si_band;                /* band event for SIGPOLL */
//        int     __spare__[7];           /* gimme some slack */
//} siginfo_t;
//void AsyncDescriptorsCluster::sigAction(int sig,struct siginfo * si,void * context)
//{
#ifndef F_SETSIG
//  assert( sig == SIGIO );
#else
//  if( si->si_code == SI_ASYNCIO ){
//    AsyncSocket * socket;
//    AsyncDescriptorsCluster * cluster;
//    ENTER_MUTEX_SECTION(mutex());
//    cluster = clusters().objectOfKey(Key<sig_t>(sig));
//    LEAVE_MUTEX_SECTION;
//    socket = cluster->sockets_.objectOfKey(Key<sock_t>(si->si_fd));
//  }
#endif
//}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
