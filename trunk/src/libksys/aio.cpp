/*-
 * Copyright 2005 Guram Dukashvili
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
#if HAVE_KQUEUE
//------------------------------------------------------------------------------
int64_t AsyncDescriptor::read2(void *,uint64_t)
{
  throw ExceptionSP(new Exception(ENOSYS,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
int64_t AsyncDescriptor::write2(const void *,uint64_t)
{
  throw ExceptionSP(new Exception(ENOSYS,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
void AsyncDescriptor::shutdown2()
{
  throw ExceptionSP(new Exception(ENOSYS,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
void AsyncDescriptor::flush2()
{
  throw ExceptionSP(new Exception(ENOSYS,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
void AsyncDescriptor::close2()
{
  throw ExceptionSP(new Exception(ENOSYS,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
void AsyncDescriptor::openAPI()
{
  throw ExceptionSP(new Exception(ENOSYS,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
void AsyncDescriptor::closeAPI()
{
  throw ExceptionSP(new Exception(ENOSYS,__PRETTY_FUNCTION__));
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
void BaseThread::attachDescriptor(AsyncDescriptor & descriptor,Fiber & toFiber)
{
  AutoLock<InterlockedMutex> lock(mutex_);
  if( descriptor.fiber_ == NULL ){
    descriptorsList_.insToTail(descriptor);
    toFiber.descriptorsList_.insToTail(descriptor);
    descriptor.fiber_ = &toFiber;
  }
  else if( descriptor.fiber_->thread_ != this ){
    throw ExceptionSP(new Exception(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_INVALID_DATA + errorOffset
#else
      EINVAL
#endif
      ,__PRETTY_FUNCTION__)
    );
  }
}
//------------------------------------------------------------------------------
void BaseThread::detachDescriptor(AsyncDescriptor & descriptor)
{
  AutoLock<InterlockedMutex> lock(mutex_);
  if( descriptor.fiber_ != NULL ){
    if( descriptor.fiber_->thread_ == this ){
      descriptorsList_.remove(descriptor);
      descriptor.fiber_->descriptorsList_.remove(descriptor);
      descriptor.fiber_ = NULL;
    }
    else {
      throw ExceptionSP(new Exception(
#if defined(__WIN32__) || defined(__WIN64__)
        ERROR_INVALID_DATA + errorOffset
#else
        EINVAL
#endif
        ,__PRETTY_FUNCTION__)
      );
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncIoSlave::~AsyncIoSlave()
{
#if defined(__WIN32__) || defined(__WIN64__)
  for( intptr_t i = sizeof(events_) / sizeof(events_[0]) - 1; i >= 0; i-- )
    CloseHandle(events_[i]);
#elif HAVE_KQUEUE
  if( close(kqueue_) != 0 ){
    perror(NULL);
    assert( 0 );
    abort();
  }
#endif
}
//---------------------------------------------------------------------------
AsyncIoSlave::AsyncIoSlave()
{
#if defined(__WIN32__) || defined(__WIN64__)
  intptr_t i;
  int32_t err;
  for( i = sizeof(eReqs_) / sizeof(eReqs_[0]) - 1; i >= 0; i-- ) eReqs_[i] = NULL;
  for( i = sizeof(events_) / sizeof(events_[0]) - 1; i >= 0; i-- ) events_[i] = NULL;
  for( i = sizeof(events_) / sizeof(events_[0]) - 1; i >= 0; i-- ){
    if( events_[i] != NULL ) continue;
    if( (events_[i] = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL ){
      err = GetLastError() + errorOffset;
      throw ExceptionSP(
        new Exception(err,__PRETTY_FUNCTION__)
      );
    }
  }
#elif HAVE_KQUEUE
  kqueue_ = kqueue();
  if( kqueue < 0 ){
    int32_t err = errno;
    throw ExceptionSP(
      new Exception(err,__PRETTY_FUNCTION__)
    );
  }
  kevents_.resize(64);
#endif
}
//---------------------------------------------------------------------------
#if HAVE_KQUEUE
//---------------------------------------------------------------------------
void AsyncIoSlave::cancelEvent(const IoRequest & request)
{
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
  }
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
bool AsyncIoSlave::transplant(AsyncEvent & request)
{
#if defined(__WIN32__) || defined(__WIN64__)
#define MAX_REQS (MAXIMUM_WAIT_OBJECTS - 2)
#elif HAVE_KQUEUE
#define MAX_REQS 64
#endif
  bool r = false;
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
    if( requests_.count() + newRequests_.count() < MAX_REQS ){
      newRequests_.insToTail(request);
      if( newRequests_.count() < 2 ){
#if defined(__WIN32__) || defined(__WIN64__)
        SetEvent(events_[MAXIMUM_WAIT_OBJECTS - 1]);
#elif HAVE_KQUEUE
        struct kevent ev;
        EV_SET(&ev,1000,EVFILT_TIMER,EV_ADD | EV_ONESHOT,0,0,0);
        if( kevent(kqueue_,&ev,1,NULL,0,NULL) == -1 ){
          perror(NULL);
          abort();
        }
#endif
        if( requests_.count() == 0 ) post();
      }
      r = true;
    }
  }
#undef MAX_REQS
  return r;
}
//---------------------------------------------------------------------------
void AsyncIoSlave::openAPI(AsyncEvent * object)
{
  switch( object->type_ ){
    case etRead :
    case etWrite :
    case etAccept :
    case etConnect :
      object->descriptor_->openAPI();
  }
}
//---------------------------------------------------------------------------
void AsyncIoSlave::closeAPI(AsyncEvent * object)
{
  switch( object->type_ ){
    case etRead :
    case etWrite :
    case etAccept :
    case etConnect :
      object->descriptor_->closeAPI();
  }
}
//---------------------------------------------------------------------------
void AsyncIoSlave::threadExecute()
{
  priority(THREAD_PRIORITY_HIGHEST);
#if defined(__WIN32__) || defined(__WIN64__)
  BOOL rw = FALSE;
  DWORD nb = 0;
  intptr_t sp = -1;
  bool isw9x = isWin9x();
#elif HAVE_KQUEUE
  struct aiocb * iocb;
  int32_t error;
  uint64_t count;
#endif
  EventsNode * node;
  AsyncEvent * object = NULL;
  for(;;){
    AutoLock<InterlockedMutex> lock(*this);
    for( node = newRequests_.first(); node != NULL; node = newRequests_.first() ){
      object = &AsyncEvent::nodeObject(*node);
      openAPI(object);
#if defined(__WIN32__) || defined(__WIN64__)
      assert( sp < MAXIMUM_WAIT_OBJECTS - 1 );
      ++sp;
      eReqs_[sp] = object;
      memset(&object->overlapped_,0,sizeof(object->overlapped_));
      object->overlapped_.Offset = (DWORD) object->position_;
      object->overlapped_.OffsetHigh = (DWORD) (object->position_ >> 32);
      object->overlapped_.hEvent = events_[sp];
l1:   SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
      SetLastError(ERROR_SUCCESS);
#endif
      switch( object->type_ ){
        case etDirectoryChangeNotification :
#if defined(__WIN32__) || defined(__WIN64__)
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
#endif
          break;
#if defined(__WIN32__) || defined(__WIN64__)
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
#endif
        case etRead     :
		      if( object->length_ > 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
            if( object->length_ > 0x40000000 ) object->length_ = 0x40000000;
            rw = object->descriptor_->Read(
              object->buffer_,
              (DWORD) object->length_,
              &nb,
              isw9x ? NULL : &object->overlapped_
            );
#elif HAVE_KQUEUE
            if( object->descriptor_->isSocket() ){
              EV_SET(&kevents_[0],object->descriptor_->socket_,EVFILT_READ,EV_ADD | EV_ONESHOT,0,0,node);
              if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) == 0 ) errno = EINPROGRESS;
	          }
	          else {
              iocb = &object->iocb_;
              memset(iocb,0,sizeof(*iocb));
              iocb->aio_fildes = object->descriptor_->descriptor_;
              iocb->aio_nbytes = object->length_;
	            iocb->aio_buf = object->buffer_;
              iocb->aio_offset = object->position_;
	            iocb->aio_sigevent.sigev_value.sigval_ptr = node; // udata
              iocb->aio_sigevent.sigev_notify_kqueue = kqueue_;
              iocb->aio_sigevent.sigev_notify = SIGEV_KEVENT;
#if !HAVE_AIO_READ
#error async io not implemented because you system not have aio_read system call
#endif
              if( aio_read(iocb) == 0 ) errno = EINPROGRESS;
            }
#endif
	        }
	        else {
	          errno = EINVAL;
	        }
          break;
        case etWrite  :
	        if( object->length_ > 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
            if( object->length_ > 0x40000000 ) object->length_ = 0x40000000;
            rw = object->descriptor_->Write(
              object->buffer_,
              (DWORD) object->length_,
              &nb,
              isw9x ? NULL : &object->overlapped_
            );
#elif HAVE_KQUEUE
            if( object->descriptor_->isSocket() ){
              EV_SET(&kevents_[0],object->descriptor_->socket_,EVFILT_WRITE,EV_ADD | EV_ONESHOT,0,0,node);
              if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) == 0 ) errno = EINPROGRESS;
	          }
	          else {
              iocb = &object->iocb_;
      	      memset(iocb,0,sizeof(*iocb));
              iocb->aio_fildes = object->descriptor_->descriptor_;
              iocb->aio_nbytes = object->length_;
	            iocb->aio_buf = object->buffer_;
              iocb->aio_offset = object->position_;
	            iocb->aio_sigevent.sigev_value.sigval_ptr = node; // udata
              iocb->aio_sigevent.sigev_notify_kqueue = kqueue_;
              iocb->aio_sigevent.sigev_notify = SIGEV_KEVENT;
#if !HAVE_AIO_WRITE
#error async io not implemented because you system not have aio_write system call
#endif
              if( aio_write(iocb) == 0 ) errno = EINPROGRESS;
            }
#endif
	        }
	        else {
	          errno = EINVAL;
	        }
          break;
        case etAccept :
#if defined(__WIN32__) || defined(__WIN64__)
          rw = object->descriptor_->AcceptEx(
            object->socket_,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            isw9x ? NULL : &object->overlapped_
          );
#elif HAVE_KQUEUE
          EV_SET(&kevents_[0],object->descriptor_->socket_,EVFILT_READ,EV_ADD | EV_ONESHOT,0,0,node);
          if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) != -1 )
            count = object->descriptor_->accept();
	        if( errno != EWOULDBLOCK ){
	          int32_t err = errno;
	          kevents_[0].flags = EV_DELETE;
            if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) == -1 && errno != ENOENT ){
              perror(NULL);
              assert( 0 );
	            abort();
            }
	          errno = err;
	        }
	        else {
	          errno = EINPROGRESS;
	        }
#endif
          break;
        case etConnect :
#if defined(__WIN32__) || defined(__WIN64__)
          rw = object->descriptor_->Connect(events_[sp],object);
#elif HAVE_KQUEUE
          EV_SET(&kevents_[0],object->descriptor_->socket_,EVFILT_READ,EV_ADD | EV_ONESHOT,0,0,node);
          if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) != -1 )
            object->descriptor_->connect(&object);
  	      if( errno != EINPROGRESS ){
	          int32_t err = errno;
	          kevents_[0].flags = EV_DELETE;
            if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) == -1 && errno != ENOENT ){
              perror(NULL);
              assert( 0 );
	            abort();
            }
	          errno = err;
	        }
#endif
          break;
        default :
          assert( 0 );
      }
#if defined(__WIN32__) || defined(__WIN64__)
      if( rw == 0 && GetLastError() == ERROR_NO_SYSTEM_RESOURCES )
        if( (object->length_ >>= 1) > 0 ) goto l1;
      if( rw == 0 && GetLastError() != ERROR_IO_PENDING && GetLastError() != WSAEWOULDBLOCK ){
        closeAPI(object);
        ResetEvent(events_[sp]);
        newRequests_.remove(*object);
        object->errno_ = GetLastError();
        object->count_ = ~(uint64_t) 0;
        object->fiber_->thread()->postEvent(object);
        sp--;
      }
      else {
        requests_.insToTail(newRequests_.remove(*object));
        SetLastError(ERROR_SUCCESS);
      }
#elif HAVE_KQUEUE
      if( errno == EINPROGRESS ){
        requests_.insToTail(newRequests_.remove(*object));
      }
      else if( errno != EINPROGRESS ){
        object->descriptor_->cluster()->postEvent(
          object->descriptor_,
	        errno,
	        object->ioType_,
	        errno != 0 ? -1 : 0
        );
        object->ioThread_ = NULL;
        newRequests_.remove(object);
      }
#endif
    }
    if( requests_.count() == 0 ){
      if( terminated_ ) break;
      release();
      Semaphore::wait();
      acquire();
    }
    else {
      node = NULL;
#if defined(__WIN32__) || defined(__WIN64__)
      DWORD wm;
      if( isw9x ){
        node = requests_.first();
	      object = &AsyncEvent::nodeObject(*node);
        wm = ~DWORD(0);
      }
      else {
        /*intptr_t i, j = -1;
        uint64_t timeout = ~uint64_t(0);
        for( intptr_t i = sp; i >= 0; i-- ) if( eReqs_[i]->timeout_ < timeout ){
          timeout = eReqs_[i]->timeout_;
          j = i;
        }*/
        release();
        wm = WaitForMultipleObjectsEx(MAXIMUM_WAIT_OBJECTS,events_,FALSE,INFINITE,TRUE);
        DWORD err0 = GetLastError();
        acquire();
        SetLastError(err0);
#if __INTEL_COMPILER
        if( wm < WAIT_OBJECT_0 + sp + 1 ){
#else
        if( wm >= WAIT_OBJECT_0 && wm < WAIT_OBJECT_0 + sp + 1 ){
#endif
          wm -= WAIT_OBJECT_0;
          assert( wm < MAXIMUM_WAIT_OBJECTS - 1 );
          object = eReqs_[wm];
          assert( object != NULL );
          node = &AsyncEvent::node(*object);
          ResetEvent(events_[wm]);
          SetLastError(ERROR_SUCCESS);
          if( object->type_ == etConnect ){
            nb = 0;
            object->descriptor_->WSAEnumNetworkEvents(events_[wm],FD_CONNECT_BIT);
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
        else if( wm == WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS - 1	){
          ResetEvent(events_[wm - WAIT_OBJECT_0]);
        }
        else if( wm == WAIT_IO_COMPLETION ||
                 (wm >= STATUS_ABANDONED_WAIT_0 && wm < STATUS_ABANDONED_WAIT_0 + sp + 1) ||
                 wm == WAIT_TIMEOUT ){
          assert( 0 );
        }
      }
      if( node != NULL ){
        object->errno_ = GetLastError();
        closeAPI(object);

        xchg(events_[wm],events_[sp]);
        eReqs_[wm] = eReqs_[sp];
        eReqs_[sp] = NULL;

        requests_.remove(*object);

        object->count_ = object->errno_ != ERROR_SUCCESS && object->errno_ != ERROR_HANDLE_EOF ? ~uint64_t(0) : nb;
        object->fiber_->thread()->postEvent(object);
        sp--;
      }
    }
#elif HAVE_KQUEUE
    else {
      int evCount = kevent(kqueue_,NULL,0,&kevents_[0],kevents_.count(),NULL);
      if( evCount == -1 ){
        perror(NULL);
        assert( 0 );
        abort();
      }
      while( --evCount >= 0 ){
        struct kevent * kev = &kevents_[evCount];
        node = (EmbeddedListNode<IoRequest> *) kev->udata;
	      if( node == NULL ) continue;
	      object = &IoRequest::nodeObject(*node);
        if( kev->filter == EVFILT_TIMER ){
          if( !requests_.nodeInserted(*node) ) continue;
          assert( object->ioType_ == etAccept );
	        kev->filter = EVFILT_READ;
          kev->flags |= EV_ERROR;
          kev->data = EINTR;
        }
        switch( object->ioType_ ){
          case etRead    :
            if( kev->filter == EVFILT_READ ){
	            count = -1;
              if( kev->flags & EV_ERROR ){
	              error = kev->data;
	              if( error == 0 ) error = ESHUTDOWN;
	            }
	            else if( kev->flags & EV_EOF ){
	              error = kev->fflags;
	              if( error == 0 ) error = ESHUTDOWN;
	            }
	            else {
  	            count = object->descriptor_->read2(
		              object->buffer_,
		              object->length_ > (uint64_t) kev->data ? kev->data : object->length_
		            );
		            error = errno;
	            }
	          }
	          else {
	            assert( kev->filter == EVFILT_AIO );
	            error = aio_error(&object->iocb_);
	            assert( error != EINPROGRESS );
	            count = aio_return(&object->iocb_);
	          }
	          break;
          case etWrite   :
            if( kev->filter == EVFILT_WRITE ){
	            count = -1;
              if( kev->flags & EV_ERROR ){
	              error = kev->data;
	              if( error == 0 ) error = ESHUTDOWN;
	            }
	            else if( kev->flags & EV_EOF ){
	              error = kev->fflags;
	              if( error == 0 ) error = ESHUTDOWN;
	            }
	            else {
  	            count = object->descriptor_->write2(
		              object->cbuffer_,
		              object->length_ > (uint64_t) kev->data ? kev->data : object->length_
		            );
		            error = errno;
	            }
	          }
	          else {
	            assert( kev->filter == EVFILT_AIO );
	            error = aio_error(&object->iocb_);
	            assert( error != EINPROGRESS );
	            count = aio_return(&object->iocb_);
	          }
	          break;
          case etAccept  :
	          assert( kev->filter == EVFILT_READ );
            if( kev->flags & EV_ERROR ){
	            error = kev->data;
	            count = -1;
	          }
	          else {
	            count = object->descriptor_->accept();
	            error = errno;
	          }
            break;
          case etConnect :
	          assert( kev->filter == EVFILT_READ );
            if( kev->flags & EV_ERROR ) error = kev->data;
            if( kev->flags & EV_EOF ) error = kev->fflags;
	          count = 0;
            break;
          default        :
                assert( 0 );
        }
        acquire();
        requests_.remove(*object);
        release();
        object->errno_ = error;
        object->count = count;
        object->fiber_->thread()->postEvent(object);
      }
    }
#endif
  }
}
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
AsyncOpenFileSlave::~AsyncOpenFileSlave()
{
}
//------------------------------------------------------------------------------
AsyncOpenFileSlave::AsyncOpenFileSlave()
{
}
//------------------------------------------------------------------------------
bool AsyncOpenFileSlave::transplant(AsyncEvent & request)
{
  bool r = false;
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
    if( requests_.count() < 64 ){
      requests_.insToTail(request);
      if( requests_.count() < 2 ) post();
      r = true;
    }
  }
  return r;
}
//------------------------------------------------------------------------------
void AsyncOpenFileSlave::threadExecute()
{
  priority(THREAD_PRIORITY_HIGHEST);
  AsyncEvent * request;
  for(;;){
    request = NULL;
    acquire();
    if( requests_.count() > 0 ) request = &requests_.remove(*requests_.first());
    release();
    if( request == NULL ){
      if( terminated_ ) break;
      Semaphore::wait();
    }
    else {
      if( request->type_ == etOpenFile ){
        try {
#if defined(__WIN32__) || defined(__WIN64__)
          HANDLE file = INVALID_HANDLE_VALUE;
          if( isWin9x() ){
            utf8::AnsiString ansiFileName(anyPathName2HostPathName(request->string0_).getANSIString());
            SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
            SetLastError(ERROR_SUCCESS);
            if( !request->readOnly_ )
              file = CreateFileA(
                ansiFileName,
                GENERIC_READ | GENERIC_WRITE,
                request->exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                request->createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
                NULL
              );
            if( file == INVALID_HANDLE_VALUE ){
              SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
              SetLastError(ERROR_SUCCESS);
              file = CreateFileA(
                ansiFileName,GENERIC_READ,
                request->exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                request->createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
                NULL
              );
            }
          }
          else {
            utf8::WideString unicodeFileName(anyPathName2HostPathName(request->string0_).getUNICODEString());
            SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
            SetLastError(ERROR_SUCCESS);
            if( !request->readOnly_ )
              file = CreateFileW(
                unicodeFileName,
                GENERIC_READ | GENERIC_WRITE,
                request->exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                request->createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
                NULL
              );
            if( file == INVALID_HANDLE_VALUE ){
              SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
              SetLastError(ERROR_SUCCESS);
              file = CreateFileW(
                unicodeFileName,
                GENERIC_READ,
                request->exclusive_ ? 0 : FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                request->createIfNotExist_ ? OPEN_ALWAYS : OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
                NULL
              );
            }
          }
          if( file != INVALID_HANDLE_VALUE && GetLastError() == ERROR_ALREADY_EXISTS ) SetLastError(0);
          request->errno_ = GetLastError();
          request->fileDescriptor_ = file;
          request->fiber_->thread()->postEvent(request);
#else
          int file;
          utf8::AnsiString ansiFileName(anyPathName2HostPathName(request->string()).getANSIString());
          mode_t um = umask(0);
          umask(um);
          errno = 0;
          file = ::open(
            ansiFileName,
            O_RDWR | (createIfNotExist ? O_CREAT : 0) | (exclusive_ ? O_EXLOCK : 0),
            um | S_IRUSR | S_IWUSR
          );
          if( file < 0 ){
            errno = 0;
            file = ::open(
              ansiFileName,
              O_RDONLY | (createIfNotExist ? O_CREAT : 0) | (exclusive_ ? O_EXLOCK : 0),
              um | S_IRUSR | S_IWUSR
            );
          }
          request->descriptor_->cluster()->postEvent(
            request->descriptor_,
            errno,
            etOpenFile,
            file
          );
#endif
        }
        catch( ExceptionSP & e ){
          request->errno_ = e->code();
          request->fileDescriptor_ = INVALID_HANDLE_VALUE;
          request->fiber_->thread()->postEvent(request);
        }
      }
      else if( request->type_ == etDirList ){
        int32_t err = 0;
        try {
          getDirList(*request->dirList_,request->string0_,request->string1_,request->recursive_,request->includeDirs_);
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
          remove(request->string0_);
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
      else if( request->type_ == etResolveName ){
        int32_t err = 0;
        try {
          request->address_.resolve(request->string0_,request->string1_);
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
          request->string0_ = request->address_.resolve(request->defPort_);
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
          rename(request->string0_,request->string1_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        assert( request->fiber_ != NULL );
        request->fiber_->thread()->postEvent(request);
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
void AsyncTimerSlave::threadExecute()
{
  priority(THREAD_PRIORITY_TIME_CRITICAL);
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
AsyncAcquireSlave::AsyncAcquireSlave() : sp_(-1)
{
#if defined(__WIN32__) || defined(__WIN64__)
  intptr_t i;
  for( i = sizeof(eSems_) / sizeof(eSems_[0]) - 1; i >= 0; i-- ) eSems_[i] = NULL;
  for( i = sizeof(sems_) / sizeof(sems_[0]) - 1; i >= 0; i-- ) sems_[i] = NULL;
  if( (sems_[MAXIMUM_WAIT_OBJECTS - 1] = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
  }
#endif
}
//------------------------------------------------------------------------------
bool AsyncAcquireSlave::transplant(AsyncEvent & request)
{
//  intptr_t i;
  bool r = false;
  if( !terminated_ ){
    AutoLock<InterlockedMutex> lock(*this);
#if defined(__WIN32__) || defined(__WIN64__)
    if( requests_.count() + newRequests_.count() < MAXIMUM_WAIT_OBJECTS - 2 ){
      //for( i = sp_; i >= 0; i-- ) if( sems_[i] == request.mutex_->sem_ ) break;
      //if( i < 0 ){
        newRequests_.insToTail(request);
        SetEvent(sems_[MAXIMUM_WAIT_OBJECTS - 1]);
        if( requests_.count() == 0 ) post();
        r = true;
      //}
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
  priority(THREAD_PRIORITY_TIME_CRITICAL);
#if defined(__WIN32__) || defined(__WIN64__)
  AsyncEvent * object;
  EventsNode * node;
  for(;;){
    AutoLock<InterlockedMutex> lock(*this);
    for(;;){
      node = newRequests_.first();
      if( node == NULL ) break;
      object = &AsyncEvent::nodeObject(*node);
      assert( object->type_ == etAcquire );
      assert( sp_ < MAXIMUM_WAIT_OBJECTS - 1 );
      ++sp_;
      sems_[sp_] = object->mutex_->sem_;
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
      assert( request->type_ == etAcquire );
      request->errno_ = 0;
      try {
        request->mutex_->acquire();
      }
      catch( ExceptionSP & e ){
        request->errno_ = e->code();
      }
      assert( request->fiber_ != NULL );
      request->fiber_->thread()->postEvent();
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
  if( (sems_[MAXIMUM_WAIT_OBJECTS - 1] = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
  }
}
//------------------------------------------------------------------------------
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
  priority(THREAD_PRIORITY_TIME_CRITICAL);
  AsyncEvent * object;
  EventsNode * node;
  for(;;){
    AutoLock<InterlockedMutex> lock(*this);
    for(;;){
      node = newRequests_.first();
      if( node == NULL ) break;
      object = &AsyncEvent::nodeObject(*node);
      assert( object->type_ == etAcquire );
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
#endif
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Requester::~Requester()
{
  intptr_t i;

  abort();
  for( i = ioSlaves_.count() - 1; i >= 0; i-- ){
    ioSlaves_[i].terminate();
    ioSlaves_[i].post();
    ioSlaves_[i].Thread::wait();
  }
  for( i = ofSlaves_.count() - 1; i >= 0; i-- ){
    ofSlaves_[i].terminate();
    ofSlaves_[i].post();
    ofSlaves_[i].Thread::wait();
  }
  for( i = acquireSlaves_.count() - 1; i >= 0; i-- ){
    acquireSlaves_[i].terminate();
    acquireSlaves_[i].post();
    acquireSlaves_[i].Thread::wait();
  }
  for( i = wdcnSlaves_.count() - 1; i >= 0; i-- ){
    wdcnSlaves_[i].terminate();
    wdcnSlaves_[i].post();
    wdcnSlaves_[i].Thread::wait();
  }
}
//---------------------------------------------------------------------------
Requester::Requester() :
  ioSlavesSweepTime_(0),
  ofSlavesSweepTime_(0),
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
  assert( currentFiber() != NULL );
  currentFiber()->event_.descriptor_ = descriptor;
  switch( currentFiber()->event_.type_ ){
    case etNone :
    case etError : 
      break;
    case etOpenFile :
    case etDirList :
    case etCreateDir :
    case etRemoveDir :
    case etRemoveFile :
    case etRename :
    case etResolveName :
    case etResolveAddress :
    case etStat :
      {
        AutoLock<InterlockedMutex> lock(ofRequestsMutex_);
        if( gettimeofday() - ofSlavesSweepTime_ >= 10000000 ){
          for( i = ofSlaves_.count() - 1; i >= 0; i-- )
            if( ofSlaves_[i].finished() ){
	            ofSlaves_[i].Thread::wait();
              ofSlaves_.remove(i);
            }
          ofSlavesSweepTime_ = gettimeofday();
        }
        for( i = ofSlaves_.count() - 1; i >= 0; i-- )
          if( ofSlaves_[i].transplant(currentFiber()->event_) ) break;
        if( i < 0 ){
          AsyncOpenFileSlave * p = new AsyncOpenFileSlave;
          AutoPtr<AsyncOpenFileSlave> slave(p);
          if( ofSlaves_.count() >= numberOfProcessors() ) p->terminate();
          p->resume();
          ofSlaves_.add(slave.ptr(NULL));
	        p->transplant(currentFiber()->event_);
        }
      }
      return;
    case etDirectoryChangeNotification :
#if defined(__WIN32__) || defined(__WIN64__)
      if( isWin9x() ){
        AutoLock<InterlockedMutex> lock(wdcnRequestsMutex_);
        if( gettimeofday() - wdcnSlavesSweepTime_ >= 10000000 ){
          for( i = wdcnSlaves_.count() - 1; i >= 0; i-- )
            if( wdcnSlaves_[i].finished() ){
	            wdcnSlaves_[i].Thread::wait();
              wdcnSlaves_.remove(i);
            }
          wdcnSlavesSweepTime_ = gettimeofday();
        }
        for( i = wdcnSlaves_.count() - 1; i >= 0; i-- )
          if( wdcnSlaves_[i].transplant(currentFiber()->event_) ) break;
        if( i < 0 ){
          AsyncWin9xDirectoryChangeNotificationSlave * p = new AsyncWin9xDirectoryChangeNotificationSlave;
          AutoPtr<AsyncWin9xDirectoryChangeNotificationSlave> slave(p);
          if( wdcnSlaves_.count() >= numberOfProcessors() ) p->terminate();
          p->resume();
          wdcnSlaves_.add(slave.ptr(NULL));
	        p->transplant(currentFiber()->event_);
        }
        return;
      }
      // walk through
#endif
    case etLockFile :
    case etRead :
    case etWrite :
    case etAccept :
    case etConnect :
      {
        AutoLock<InterlockedMutex> lock(ioRequestsMutex_);
        if( gettimeofday() - ioSlavesSweepTime_ >= 10000000 ){
          for( i = ioSlaves_.count() - 1; i >= 0; i-- )
            if( ioSlaves_[i].finished() ){
	            ioSlaves_[i].Thread::wait();
              ioSlaves_.remove(i);
            }
          ioSlavesSweepTime_ = gettimeofday();
        }
        for( i = ioSlaves_.count() - 1; i >= 0; i-- )
          if( ioSlaves_[i].transplant(currentFiber()->event_) ) break;
        if( i < 0 ){
          AsyncIoSlave * p = new AsyncIoSlave;
          AutoPtr<AsyncIoSlave> slave(p);
          if( ioSlaves_.count() >= numberOfProcessors() ) p->terminate();
          p->resume();
          ioSlaves_.add(slave.ptr(NULL));
	        p->transplant(currentFiber()->event_);
        }
      }
      return;
    case etClose :
    case etQuit :
    case etDispatch :
      break;
    case etTimer :
      {
        AutoLock<InterlockedMutex> lock(timerRequestsMutex_);
        if( timerSlave_ == NULL ){
          AutoPtr<AsyncTimerSlave> slave(new AsyncTimerSlave);
          slave->resume();
          timerSlave_ = slave.ptr(NULL);
        }
        timerSlave_->transplant(currentFiber()->event_);
      }
      return;
    case etAcquire :
      {
        AutoLock<InterlockedMutex> lock(acquireRequestsMutex_);
        if( gettimeofday() - acquireSlavesSweepTime_ >= 10000000 ){
          for( i = acquireSlaves_.count() - 1; i >= 0; i-- )
            if( acquireSlaves_[i].finished() ){
	            acquireSlaves_[i].Thread::wait();
              acquireSlaves_.remove(i);
            }
          acquireSlavesSweepTime_ = gettimeofday();
        }
        for( i = acquireSlaves_.count() - 1; i >= 0; i-- )
          if( acquireSlaves_[i].transplant(currentFiber()->event_) ) break;
        if( i < 0 ){
          AsyncAcquireSlave * p = new AsyncAcquireSlave;
          AutoPtr<AsyncAcquireSlave> slave(p);
          if( acquireSlaves_.count() >= numberOfProcessors() ) p->terminate();
          p->resume();
          acquireSlaves_.add(slave.ptr(NULL));
	        p->transplant(currentFiber()->event_);
        }
      }
      return;
  }
  throw ExceptionSP(new Exception(EINVAL,__PRETTY_FUNCTION__));
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
