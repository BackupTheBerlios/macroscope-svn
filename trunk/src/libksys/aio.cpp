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
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
uintptr_t AsyncDescriptorsCluster::refCount_ = 0;
//------------------------------------------------------------------------------
void AsyncDescriptorsCluster::initialize()
{
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
void AsyncDescriptorsCluster::cleanup()
{
}
//------------------------------------------------------------------------------
AsyncDescriptorsCluster::~AsyncDescriptorsCluster()
{
  deallocateSig();
}
//------------------------------------------------------------------------------
AsyncDescriptorsCluster::AsyncDescriptorsCluster()
{
  allocateSig();
}
//------------------------------------------------------------------------------
void AsyncDescriptorsCluster::allocateSig()
{
  AutoLock<InterlockedMutex> lock(giant());
  if( refCount_ == 0 ){
    new (requester_) Requester;
    try {
      requester().resume();
    }
    catch( ... ){
      requester().~Requester();
      throw;
    }
  }
  refCount_++;
}
//------------------------------------------------------------------------------
void AsyncDescriptorsCluster::deallocateSig()
{
  AutoLock<InterlockedMutex> lock(giant());
  assert( refCount_ > 0 );
  if( refCount_ == 1 ){
    requester().terminate();
    while( !requester().finished() ){
      requester().post();
      sleep1();
    }
    requester().Thread::wait();
    requester().~Requester();
  }
  refCount_--;
}
//------------------------------------------------------------------------------
void AsyncDescriptorsCluster::attachDescriptor(AsyncDescriptor & descriptor,Fiber & toFiber)
{
  AutoLock<InterlockedMutex> lock(mutex_);
  if( descriptor.cluster_ == NULL ){
    descriptorsList_.insToTail(descriptor);
    descriptor.cluster_ = this;
    toFiber.Fiber::attachDescriptor(descriptor);
  }
  else if( descriptor.cluster_ != this ){
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
void AsyncDescriptorsCluster::detachDescriptor(AsyncDescriptor & descriptor)
{
  AutoLock<InterlockedMutex> lock(mutex_);
  if( descriptor.cluster_ != NULL ){
    if( descriptor.cluster_ == this ){
      descriptorsList_.remove(descriptor);
      descriptor.cluster_ = NULL;
      descriptor.fiber_->Fiber::detachDescriptor(descriptor);
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
void AsyncDescriptorsCluster::postEvent(AsyncEventType event,Fiber * fiber)
{
  AutoLock<InterlockedMutex> lock(mutex_);
  fiber->event_.type_ = event;
  fiber->event_.fiber_ = fiber;
  fiber->event_.descriptor_ = NULL;
  events_.insToTail(fiber->event_);
  if( events_.count() < 2 ) semaphore_.post();
}
//---------------------------------------------------------------------------
void AsyncDescriptorsCluster::postEvent(
  AsyncDescriptor * descriptor,
  int32_t errNo,
  AsyncEventType event,
  uint64_t count)
{
  AutoLock<InterlockedMutex> lock(mutex_);
  descriptor->fiber_->event_.fiber_ = descriptor->fiber_;
  descriptor->fiber_->event_.descriptor_ = descriptor;
  descriptor->fiber_->event_.errno_ = errNo;
  descriptor->fiber_->event_.type_ = event;
  descriptor->fiber_->event_.count_ = count;
  events_.insToTail(descriptor->fiber_->event_);
  if( events_.count() < 2 ) semaphore_.post();
}
//---------------------------------------------------------------------------
void AsyncDescriptorsCluster::postEvent(
  AsyncDescriptor * descriptor,
  int32_t errNo,
  AsyncEventType event,
  const AsyncDescriptorKey & file)
{
  assert( event == etOpenFile );
  AutoLock<InterlockedMutex> lock(mutex_);
  descriptor->fiber_->event_.fiber_ = descriptor->fiber_;
  descriptor->fiber_->event_.descriptor_ = descriptor;
  descriptor->fiber_->event_.errno_ = errNo;
  descriptor->fiber_->event_.type_ = event;
  descriptor->fiber_->event_.fileDescriptor_ = file.descriptor_;
  events_.insToTail(descriptor->fiber_->event_);
  if( events_.count() < 2 ) semaphore_.post();
}
//---------------------------------------------------------------------------
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
void AsyncIoSlave::execute()
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
    acquire();
    if( requests_.count() == 0 && newRequests_.count() == 0 ){
      release();
      Semaphore::wait();
      acquire();
      if( terminated_ ){
        release();
        break;
      }
    }
    for( node = newRequests_.first(); node != NULL; node = newRequests_.first() ){
      object = &AsyncEvent::nodeObject(*node);
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
            (SOCKET) object->buffer_,
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
        ResetEvent(events_[sp]);
        newRequests_.remove(*object);
        object->descriptor_->cluster()->postEvent(
	        object->descriptor_,
	        GetLastError(),
	        object->type_,
	        ~(uint64_t) 0
	      );
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
    node = NULL;
#if defined(__WIN32__) || defined(__WIN64__)
    if( requests_.count() > 0 ){
      DWORD wm;
      if( isw9x ){
        node = requests_.first();
		    object = &AsyncEvent::nodeObject(*node);
        wm = ~DWORD(0);
      }
      else {
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
          if( object->type_ != etConnect ){
            object->descriptor_->GetOverlappedResult(
              &object->overlapped_,
              &nb,
              TRUE
            );
          }
          else {
            nb = 0;
            object->descriptor_->WSAEnumNetworkEvents(events_[wm],FD_CONNECT_BIT);
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
        xchg(events_[wm],events_[sp]);
        eReqs_[wm] = eReqs_[sp];
        eReqs_[sp] = NULL;

        requests_.remove(*object);

        object->descriptor_->cluster()->postEvent(
	        object->descriptor_,
	        GetLastError(),
	        object->type_,
	        GetLastError() != ERROR_SUCCESS ? ~uint64_t(0) : nb
	      );
        sp--;
      }
    }
    release();
#elif HAVE_KQUEUE
    if( requests_.count() > 0 ){
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
        object->descriptor_->cluster()->postEvent(
  	      object->descriptor_,
	        error,
	        object->ioType_,
	        count
	      );
        acquire();
  	    object->ioThread_ = NULL;
        requests_.remove(*object);
        release();
      }
    }
#endif
  }
}
//------------------------------------------------------------------------------
void AsyncIoSlave::transplant(Events & requests)
{
  assert( !terminated_ );
#if defined(__WIN32__) || defined(__WIN64__)
#define MAX_REQS (MAXIMUM_WAIT_OBJECTS - 2)
#elif HAVE_KQUEUE
#define MAX_REQS 64
#endif
  bool r = false;
  AutoLock<InterlockedMutex> lock(*this);
  while( requests.count() > 0 && requests_.count() + newRequests_.count() < MAX_REQS ){
    newRequests_.insToTail(requests.remove(*requests.first()));
    r = true;
  }
  if( r ){
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
    post();
  }
#undef MAX_REQS
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
void AsyncOpenFileSlave::transplant(Events & requests)
{
  assert( !terminated_ );
  bool r = false;
  AutoLock<InterlockedMutex> lock(*this);
  while( requests.count() > 0 && requests_.count() < 16 ){
    requests_.insToTail(requests.remove(*requests.first()));
    r = true;
  }
  if( r ) post();
}
//------------------------------------------------------------------------------
void AsyncOpenFileSlave::execute()
{
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
          request->descriptor_->cluster()->postEvent(
            request->descriptor_,
            GetLastError(),
            etOpenFile,
            file
          );
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
          request->descriptor_->cluster()->postEvent(
            request->descriptor_,
            e->code(),
            etOpenFile,
            INVALID_HANDLE_VALUE
          );
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
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(request->type_,fiber);
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
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(request->type_,fiber);
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
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(request->type_,fiber);
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
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(request->type_,fiber);
      }
      else if( request->type_ == etResolveName ){
        int32_t err = 0;
        try {
          request->address_.resolve(request->string0_,request->defPort_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(request->type_,fiber);
      }
      else if( request->type_ == etResolveAddress ){
        int32_t err = 0;
        try {
          request->string0_ = request->address_.resolve();
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(request->type_,fiber);
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
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(request->type_,fiber);
      }
      else if( request->type_ == etRename ){
        int32_t err = 0;
        try {
          rename(request->string0_,request->string0_);
        }
        catch( ExceptionSP & e ){
          err = e->code();
        }
        request->errno_ = err;
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(request->type_,fiber);
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
void AsyncTimerSlave::transplant(Events & requests)
{
  assert( !terminated_ );
  AutoLock<InterlockedMutex> lock(*this);
  while( requests.count() > 0 ){
    requests_.insToTail(requests.remove(*requests.first()));
    post();
  }
}
//------------------------------------------------------------------------------
void AsyncTimerSlave::execute()
{
  priority(THREAD_PRIORITY_TIME_CRITICAL);
  uint64_t minTimeout, timerStartTime, elapsedTime, currentTime;
  EmbeddedListNode<AsyncEvent> * requestNode;
  AsyncEvent * request, * minRequest;
  for(;;){
    minRequest = NULL;
    minTimeout = ~uint64_t(0);
    {
      AutoLock<InterlockedMutex> lock(*this);
      requestNode = requests_.first();
      while( requestNode != NULL ){
        request = &AsyncEvent::nodeObject(*requestNode);
        if( request->timeout_ < minTimeout ){
          minRequest = request;
          minTimeout = request->timeout_;
        }
        requestNode = requestNode->next();
      }
    }
    if( minRequest == NULL ){
      if( terminated_ ) break;
      Semaphore::wait();
    }
    else {
      assert( minRequest->type_ == etTimer );
      timerStartTime = gettimeofday();
      Semaphore::timedWait(minRequest->timeout_);
      AutoLock<InterlockedMutex> lock(*this);
      requestNode = requests_.first();
      while( requestNode != NULL ){
        elapsedTime = (currentTime = gettimeofday()) - timerStartTime;
        request = &AsyncEvent::nodeObject(*requestNode);
        if( request->timerStartTime_ <= currentTime )
          request->timeout_ -= request->timeout_ >= elapsedTime ? elapsedTime : request->timeout_;
        if( request->timeout_ == 0 || request->abortTimer_ ){
          BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
          assert( fiber != NULL );
          request->errno_ = request->abortTimer_ ?
#if defined(__WIN32__) || defined(__WIN64__)
            ERROR_REQUEST_ABORTED + errorOffset
#else
            EINTR
#endif
            : 0;
          requestNode = requestNode->next();
          requests_.remove(*request);
          fiber->thread()->postEvent(request->type_,fiber);
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
  EmbeddedListNode<AsyncEvent> * requestNode = requests_.first();
  while( requestNode != NULL ){
    AsyncEvent::nodeObject(*requestNode).abortTimer_ = true;
    requestNode = requestNode->next();
  }
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
  if( (sems_[MAXIMUM_WAIT_OBJECTS - 1] = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,__PRETTY_FUNCTION__));
  }
#endif
}
//------------------------------------------------------------------------------
void AsyncAcquireSlave::transplant(Events & requests)
{
  assert( !terminated_ );
  AutoLock<InterlockedMutex> lock(*this);
#if defined(__WIN32__) || defined(__WIN64__)
  bool r = false;
  while( requests.count() > 0 && requests_.count() + newRequests_.count() < MAXIMUM_WAIT_OBJECTS - 2 ){
    newRequests_.insToTail(requests.remove(*requests.first()));
    r = true;
  }
  if( r ){
    SetEvent(sems_[MAXIMUM_WAIT_OBJECTS - 1]);
    post();
  }
#else
  while( requests.count() > 0 ){
    requests_.insToTail(requests.remove(*requests.first()));
    post();
  }
#endif
}
//------------------------------------------------------------------------------
void AsyncAcquireSlave::execute()
{
  priority(THREAD_PRIORITY_TIME_CRITICAL);
#if defined(__WIN32__) || defined(__WIN64__)
  intptr_t sp = -1;
  AsyncEvent * object;
  EmbeddedListNode<AsyncEvent> * node;
  for(;;){
    AutoLock<InterlockedMutex> lock(*this);
    for(;;){
      node = newRequests_.first();
      if( node == NULL ) break;
      object = &AsyncEvent::nodeObject(*node);
      assert( object->type_ == etAcquire );
      assert( sp < MAXIMUM_WAIT_OBJECTS - 1 );
      ++sp;
      sems_[sp] = object->mutex_->sem_;
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
      release();
      object = NULL;
      node = NULL;
      DWORD wm = WaitForMultipleObjectsEx(MAXIMUM_WAIT_OBJECTS,sems_,FALSE,INFINITE,TRUE);
      acquire();
      if( wm >= WAIT_OBJECT_0 && wm < WAIT_OBJECT_0 + sp + 1 ){
        wm -= WAIT_OBJECT_0;
        assert( wm < MAXIMUM_WAIT_OBJECTS - 1 );
        object = eSems_[wm];
        assert( object != NULL );
        node = &AsyncEvent::node(*object);
      }
      else if( wm >= STATUS_ABANDONED_WAIT_0 && wm < STATUS_ABANDONED_WAIT_0 + sp + 1 ){
        wm -= STATUS_ABANDONED_WAIT_0;
        assert( wm < MAXIMUM_WAIT_OBJECTS - 1 );
        object = eSems_[wm];
        assert( object != NULL );
        node = &AsyncEvent::node(*object);
      }
      else if( wm == WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS - 1	){
        ResetEvent(sems_[wm - WAIT_OBJECT_0]);
      }
      else if( wm == STATUS_ABANDONED_WAIT_0 + MAXIMUM_WAIT_OBJECTS - 1	){
        ResetEvent(sems_[wm - STATUS_ABANDONED_WAIT_0]);
      }
      else {
        assert( 0 );
      }
      if( node != NULL ){
        xchg(sems_[wm],sems_[sp]);
        eSems_[wm] = eSems_[sp];
        eSems_[sp] = NULL;
        sems_[sp] = NULL;
        sp--;
        requests_.remove(*node);
        BaseFiber * fiber = dynamic_cast<BaseFiber *>(object->fiber_);
        assert( fiber != NULL );
        fiber->thread()->postEvent(object->type_,fiber);
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
      BaseFiber * fiber = dynamic_cast<BaseFiber *>(request->fiber_);
      assert( fiber != NULL );
      fiber->thread()->postEvent(request->type_,fiber);
    }
  }
#endif
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Requester::~Requester()
{
}
//---------------------------------------------------------------------------
Requester::Requester()
{
}
//---------------------------------------------------------------------------
void Requester::execute()
{
  intptr_t i;
  priority(THREAD_PRIORITY_HIGHEST);
  for(;;){
    Semaphore::wait();
    if( terminated_ ) break;
    AutoLock<InterlockedMutex> lock(*this);
    try {
      for(;;){
        for( i = slaves_.count() - 1; i >= 0; i-- ){
          /*if( slaves_[i].finished() ){
            slaves_[i].Thread::wait();
            slaves_.remove(i);
            continue;
          }*/
          slaves_[i].transplant(ioRequests_);
          if( ioRequests_.count() == 0 ) break;
        }
        if( ioRequests_.count() == 0 ) break;
        AutoPtr<AsyncIoSlave> slave(new AsyncIoSlave);
        slave->resume();
  	    try {
	        slaves_.add(slave.ptr());
	      }
	      catch( ... ){
	        slave->terminate();
	        slave->post();
	        slave->Thread::wait();
	        throw;
	      }
	      slave->transplant(ioRequests_);
        //if( slaves_.count() > numberOfProcessors() ) slave->terminate();
        slave.ptr(NULL);
        if( ioRequests_.count() == 0 ) break;
      }
      for(;;){
        for( i = ofSlaves_.count() - 1; i >= 0; i-- ){
          ofSlaves_[i].transplant(ofRequests_);
          if( ofRequests_.count() == 0 ) break;
        }
        if( ofRequests_.count() == 0 ) break;
        AutoPtr<AsyncOpenFileSlave> slave(new AsyncOpenFileSlave);
        slave->resume();
  	    try {
	        ofSlaves_.add(slave.ptr());
	      }
	      catch( ... ){
	        slave->terminate();
	        slave->post();
	        slave->Thread::wait();
	        throw;
	      }
	      slave->transplant(ofRequests_);
        slave.ptr(NULL);
        if( ofRequests_.count() == 0 ) break;
      }
      if( timerRequests_.count() > 0 ){
        if( timerSlave_ == NULL ){
          AutoPtr<AsyncTimerSlave> slave(new AsyncTimerSlave);
          slave->resume();
          timerSlave_ = slave.ptr(NULL);
        }
        timerSlave_->transplant(timerRequests_);
      }
      for(;;){
        for( i = acquireSlaves_.count() - 1; i >= 0; i-- ){
          acquireSlaves_[i].transplant(acquireRequests_);
          if( acquireRequests_.count() == 0 ) break;
        }
        if( acquireRequests_.count() == 0 ) break;
        AutoPtr<AsyncAcquireSlave> slave(new AsyncAcquireSlave);
        slave->resume();
  	    try {
	        acquireSlaves_.add(slave.ptr());
	      }
	      catch( ... ){
	        slave->terminate();
	        slave->post();
	        slave->Thread::wait();
	        throw;
	      }
	      slave->transplant(acquireRequests_);
        slave.ptr(NULL);
        if( acquireRequests_.count() == 0 ) break;
      }
    }
    catch( ... ){
      post();
    }
  }
  abortTimer();
  for( i = slaves_.count() - 1; i >= 0; i-- ){
    slaves_[i].terminate();
    slaves_[i].post();
    slaves_[i].Thread::wait();
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
}
//------------------------------------------------------------------------------
void Requester::abortTimer()
{
  AutoLock<InterlockedMutex> lock(*this);
  if( timerSlave_ != NULL ){
    timerSlave_->terminate();
    timerSlave_->abortTimer();
    timerSlave_->post();
    timerSlave_->Thread::wait();
    timerSlave_ = NULL;
  }
}
//---------------------------------------------------------------------------
void Requester::postRequest(
  AsyncDescriptor * descriptor,
  uint64_t position,
  const void * buffer,
  uint64_t length,
  AsyncEventType ioType)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ioRequests_.count() == 0;
  descriptor->fiber_->event_.fiber_ = descriptor->fiber_;
  descriptor->fiber_->event_.descriptor_ = descriptor;
  descriptor->fiber_->event_.position_ = position;
  descriptor->fiber_->event_.cbuffer_ = buffer;
  descriptor->fiber_->event_.length_ = length;
  descriptor->fiber_->event_.type_ = ioType;
  ioRequests_.insToTail(descriptor->fiber_->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(AsyncDescriptor * descriptor,const ksock::SockAddr & addr)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ioRequests_.count() == 0;
  descriptor->fiber_->event_.fiber_ = descriptor->fiber_;
  descriptor->fiber_->event_.descriptor_ = descriptor;
  descriptor->fiber_->event_.type_ = etConnect;
  descriptor->fiber_->event_.address_ = addr;
  ioRequests_.insToTail(descriptor->fiber_->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(
  AsyncDescriptor * descriptor,
  const utf8::String & fileName,
  bool createIfNotExist,
  bool exclusive,
  bool readOnly)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ofRequests_.count() == 0;
  descriptor->fiber_->event_.fiber_ = descriptor->fiber_;
  descriptor->fiber_->event_.descriptor_ = descriptor;
  descriptor->fiber_->event_.type_ = etOpenFile;
  descriptor->fiber_->event_.createIfNotExist_ = createIfNotExist;
  descriptor->fiber_->event_.exclusive_ = exclusive;
  descriptor->fiber_->event_.readOnly_ = readOnly;
  descriptor->fiber_->event_.string0_ = fileName;
  ofRequests_.insToTail(descriptor->fiber_->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(AsyncDescriptor * descriptor,uint64_t position,uint64_t length,AsyncEvent::LockFileType lockType)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ioRequests_.count() == 0;
  descriptor->fiber_->event_.fiber_ = descriptor->fiber_;
  descriptor->fiber_->event_.descriptor_ = descriptor;
  descriptor->fiber_->event_.type_ = etLockFile;
  descriptor->fiber_->event_.lockType_ = lockType;
  descriptor->fiber_->event_.position_ = position;
  descriptor->fiber_->event_.length_ = length;
  ioRequests_.insToTail(descriptor->fiber_->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(Vector<utf8::String> * dirList,const utf8::String & dirAndMask,const utf8::String & exMask,bool recursive,bool includeDirs)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ofRequests_.count() == 0;
  BaseFiber * fiber = dynamic_cast<BaseFiber *>(currentFiber().ptr());
  assert( fiber != NULL );
  fiber->event_.descriptor_ = NULL;
  fiber->event_.dirList_ = dirList;
  fiber->event_.string0_ = dirAndMask;
  fiber->event_.string1_ = exMask;
  fiber->event_.recursive_ = recursive;
  fiber->event_.includeDirs_ = includeDirs;
  fiber->event_.type_ = etDirList;
  ofRequests_.insToTail(fiber->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(AsyncEventType event,const utf8::String & name,bool recursive)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ofRequests_.count() == 0;
  BaseFiber * fiber = dynamic_cast<BaseFiber *>(currentFiber().ptr());
  assert( fiber != NULL );
  fiber->event_.descriptor_ = NULL;
  fiber->event_.type_ = event;
  fiber->event_.string0_ = name;
  fiber->event_.recursive_ = recursive;
  ofRequests_.insToTail(fiber->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(const utf8::String & name,struct Stat & stat)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ofRequests_.count() == 0;
  BaseFiber * fiber = dynamic_cast<BaseFiber *>(currentFiber().ptr());
  assert( fiber != NULL );
  fiber->event_.descriptor_ = NULL;
  fiber->event_.string0_ = name;
  fiber->event_.stat_ = &stat;
  fiber->event_.type_ = etStat;
  ofRequests_.insToTail(fiber->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(const utf8::String & name,uintptr_t defPort)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ofRequests_.count() == 0;
  BaseFiber * fiber = dynamic_cast<BaseFiber *>(currentFiber().ptr());
  assert( fiber != NULL );
  fiber->event_.descriptor_ = NULL;
  fiber->event_.string0_ = name;
  fiber->event_.defPort_ = defPort;
  fiber->event_.type_ = etResolveName;
  ofRequests_.insToTail(fiber->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(const ksock::SockAddr & address)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ofRequests_.count() == 0;
  BaseFiber * fiber = dynamic_cast<BaseFiber *>(currentFiber().ptr());
  assert( fiber != NULL );
  fiber->event_.descriptor_ = NULL;
  fiber->event_.address_ = address;
  fiber->event_.type_ = etResolveAddress;
  ofRequests_.insToTail(fiber->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(const utf8::String & oldName,const utf8::String & newName)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = ofRequests_.count() == 0;
  BaseFiber * fiber = dynamic_cast<BaseFiber *>(currentFiber().ptr());
  assert( fiber != NULL );
  fiber->event_.descriptor_ = NULL;
  fiber->event_.string0_ = oldName;
  fiber->event_.string1_ = newName;
  fiber->event_.type_ = etRename;
  ofRequests_.insToTail(fiber->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(uint64_t timeout)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = timerRequests_.count() == 0;
  BaseFiber * fiber = dynamic_cast<BaseFiber *>(currentFiber().ptr());
  assert( fiber != NULL );
  fiber->event_.descriptor_ = NULL;
  fiber->event_.timerStartTime_ = gettimeofday();
  fiber->event_.timeout_ = timeout;
  fiber->event_.abortTimer_ = false;
  fiber->event_.type_ = etTimer;
  timerRequests_.insToTail(fiber->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
void Requester::postRequest(FiberInterlockedMutex * mutex)
{
  bool pf;
  AutoLock<InterlockedMutex> lock(*this);
  pf = acquireRequests_.count() == 0;
  BaseFiber * fiber = dynamic_cast<BaseFiber *>(currentFiber().ptr());
  assert( fiber != NULL );
  fiber->event_.descriptor_ = NULL;
  fiber->event_.mutex_ = mutex;
  fiber->event_.type_ = etAcquire;
  acquireRequests_.insToTail(fiber->event_);
  if( pf ) post();
}
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
uint8_t AsyncDescriptorsCluster::requester_[sizeof(Requester)];
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
