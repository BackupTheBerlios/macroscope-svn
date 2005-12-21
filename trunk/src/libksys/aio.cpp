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
AsyncDescriptor & AsyncDescriptor::attach(Fiber & fiber,AsyncDescriptorsCluster & cluster)
{
  cluster.attach(*this);
  fiber.attach(*this);
  return *this;
}
//------------------------------------------------------------------------------
AsyncDescriptor & AsyncDescriptor::detach()
{
  assert( (cluster_ == NULL && fiber_ == NULL) || (cluster_ != NULL && fiber_ != NULL) );
  if( cluster_ != NULL ){
    cluster_->detach(*this);
    fiber_->detach(*this);
  }
  return *this;
}
//------------------------------------------------------------------------------
void AsyncDescriptor::shutdown2()
{
  throw ExceptionSP(new Exception(ENOSYS,utf8::string(__PRETTY_FUNCTION__)));
}
//------------------------------------------------------------------------------
void AsyncDescriptor::flush2()
{
  throw ExceptionSP(new Exception(ENOSYS,utf8::string(__PRETTY_FUNCTION__)));
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
HWND AsyncDescriptorsCluster::hWnd_ = NULL;
#endif
uintptr_t AsyncDescriptorsCluster::hWndRefCount_ = 0;
uint8_t AsyncDescriptorsCluster::mutex_[sizeof(InterlockedMutex)];
//uint8_t AsyncDescriptorsCluster::descriptorsPH_[
//  sizeof(HashedObjectList<HashedObjectListKey<AsyncDescriptorKey>,AsyncDescriptor>)
//];
//------------------------------------------------------------------------------
void AsyncDescriptorsCluster::initialize()
{
  new (mutex_) InterlockedMutex;
//  new (descriptorsPH_) HashedObjectList<AsyncDescriptorKey,AsyncDescriptor>(true,false);
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
//  descriptors().~HashedObjectList<AsyncDescriptorKey,AsyncDescriptor>();
  mutex().~InterlockedMutex();
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
  AutoLock<InterlockedMutex> lock(mutex());
  if( hWndRefCount_ == 0 ){
    new (ioRequests_) EmbeddedList<IoRequest,IoRequest::node>;
    try {
      new (ioRequestsSemaphore_) Semaphore;
      try {
        new (ioRequestsMutex_) InterlockedMutex;
        try {
#if defined(__WIN32__) || defined(__WIN64__)
//          new (messagesController_) MessagesController;
#endif
          try {
#if defined(__WIN32__) || defined(__WIN64__)
//            messagesController().resume();
//            while( !messagesController().messageLoop_ && !messagesController().finished() ) Sleep(1);
//            if( messagesController().finished() ){
//              int32_t err = messagesController().exitCode();
//              throw ExceptionSP(
//                new Exception(err,utf8::string(__PRETTY_FUNCTION__))
//              );
//            }
#endif
            try {
              new (ioMaster_) AsyncIoMaster;
              try {
                ioMaster().resume();
              }
              catch( ... ){
                ioMaster().~AsyncIoMaster();
                throw;
              }
            }
            catch( ... ){
#if defined(__WIN32__) || defined(__WIN64__)
//              messagesController().terminate();
//              while( PostThreadMessage(messagesController().id_,WM_QUIT,0,0) == 0 ) Sleep(1);
//              messagesController().wait();
#endif
              throw;
            }
          }
          catch( ... ){
#if defined(__WIN32__) || defined(__WIN64__)
//            messagesController().~MessagesController();
#endif
            throw;
          }
        }
        catch( ... ){
          ioRequestsMutex().~InterlockedMutex();
          throw;
        }
      }
      catch( ... ){
        ioRequestsSemaphore().~Semaphore();
        throw;
      }
    }
    catch( ... ){
      ioRequests().~EmbeddedList<IoRequest,IoRequest::node>();
      throw;
    }
  }
  hWndRefCount_++;
}
//------------------------------------------------------------------------------
void AsyncDescriptorsCluster::deallocateSig()
{
  AutoLock<InterlockedMutex> lock(mutex());
  assert( hWndRefCount_ > 0 );
  if( hWndRefCount_ == 1 ){
    ioMaster().terminate();
    while( !ioMaster().finished() ){
      ioRequestsSemaphore().post();
      sleep1();
    }
    ioMaster().wait();
    ioMaster().~AsyncIoMaster();
#if defined(__WIN32__) || defined(__WIN64__)
//    messagesController().terminate();
//    while( PostThreadMessage(messagesController().id_,WM_QUIT,0,0) == 0 ) Sleep(1);
//    messagesController().wait();
//    messagesController().~MessagesController();
#endif
    ioRequestsMutex().~InterlockedMutex();
    ioRequestsSemaphore().~Semaphore();
    assert( ioRequests().count() == 0 );
    ioRequests().~EmbeddedList<IoRequest,IoRequest::node>();
  }
  hWndRefCount_--;
}
//------------------------------------------------------------------------------
AsyncDescriptorsCluster & AsyncDescriptorsCluster::attach(AsyncDescriptor & descriptor)
{
  if( descriptor.cluster_ == NULL ){
    AutoLock<InterlockedMutex> lock(mutex());
    descriptors_.add(&descriptor,descriptor);
    descriptor.cluster_ = this;
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncDescriptorsCluster & AsyncDescriptorsCluster::detach(AsyncDescriptor & descriptor)
{
  if( descriptor.cluster_ != NULL ){
    assert( descriptor.cluster_ == this );
    AutoLock<InterlockedMutex> lock(mutex());
    descriptors_.removeByObject(&descriptor);
    descriptor.cluster_ = NULL;
  }
  return *this;
}
//------------------------------------------------------------------------------
AsyncDescriptorsCluster & AsyncDescriptorsCluster::queue()
{
  semaphore_.wait();
  AsyncEvent * ev = NULL;
  eventMutex_.acquire();
  if( events_.count() > 0 )
    events_.remove(*(ev = events_.first()->object()));
  eventMutex_.release();
  if( ev != NULL ){
    Fiber * fiber;
    if( ev->event_ == etDispatch || ev->event_ == etQuit ){
      fiber = ev->fiber_;
    }
    else {
      fiber = ev->descriptor_->fiber_;
    }
    assert( !fiber->finished() );
    currentFiber()->switchFiber(fiber);
  }
  return *this;
}
//------------------------------------------------------------------------------
/*uint8_t AsyncDescriptorsCluster::messagesController_[sizeof(MessagesController)];
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncDescriptorsCluster::MessagesController::~MessagesController()
{
}
//---------------------------------------------------------------------------
AsyncDescriptorsCluster::MessagesController::MessagesController() : messageLoop_(false)
{
}
//---------------------------------------------------------------------------
void AsyncDescriptorsCluster::MessagesController::execute()
{
  AsyncDescriptorsCluster::hWnd_ = CreateWindowExA(
    0,"STATIC","",0,0,0,1,1,NULL,NULL,(HINSTANCE) GetCurrentProcess(),NULL);
  if( AsyncDescriptorsCluster::hWnd_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    throw ExceptionSP(new Exception(err,utf8::string(__PRETTY_FUNCTION__)));
  }
  union {
    LONG ptr;
    void * vptr;
  };
  vptr = AsyncDescriptorsCluster::wndProc;
  SetWindowLong(AsyncDescriptorsCluster::hWnd_,GWL_WNDPROC,ptr);
  SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);
  messageLoop_ = true;
  MSG msg;
  for(;;){
    if( PeekMessage(&msg,AsyncDescriptorsCluster::hWnd_,0,0,PM_REMOVE) != 0 ){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if( msg.message == WM_QUIT || terminated_ ) break;
    if( WaitMessage() == 0 ){
      int32_t err = GetLastError() + errorOffset;
      throw ExceptionSP(
        new Exception(err,utf8::string(__PRETTY_FUNCTION__))
      );
    }
  }
  DestroyWindow(AsyncDescriptorsCluster::hWnd_);
}*/
//---------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncIoSlave::~AsyncIoSlave()
{
#if defined(__WIN32__) || defined(__WIN64__)
  for( intptr_t i = MAXIMUM_WAIT_OBJECTS - 1; i >= 0; i-- ) CloseHandle(events_[i]);
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
  for( i = MAXIMUM_WAIT_OBJECTS - 1; i >= 0; i-- ) events_[i] = NULL;
  for( i = MAXIMUM_WAIT_OBJECTS - 1; i >= 0; i-- ){
    if( events_[i] != NULL ) continue;
    if( (events_[i] = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL ){
      err = GetLastError() + errorOffset;
      throw ExceptionSP(
        new Exception(err,utf8::string(__PRETTY_FUNCTION__))
      );
    }
  }
#elif HAVE_KQUEUE
  kqueue_ = kqueue();
  if( kqueue < 0 ){
    int32_t err = errno;
    throw ExceptionSP(
      new Exception(err,utf8::string(__PRETTY_FUNCTION__))
    );
  }
  kevents_.resize(64);
#endif
}
//---------------------------------------------------------------------------
AsyncIoSlave & AsyncIoSlave::
  transplant(EmbeddedList<IoRequest,IoRequest::node> & requests)
{
  bool r = false;
  acquire();
  while( requests.count() > 0 && requests_.count() + newRequests_.count() < 
#if defined(__WIN32__) || defined(__WIN64__)
    MAXIMUM_WAIT_OBJECTS - 2
#elif HAVE_KQUEUE
    64
#endif
  ){
    newRequests_.insToTail(requests.remove(*requests.first()->object()));
    newRequests_.last()->object()->ioThread_ = this;
    r = true;
  }
  release();
  if( r ){
#if defined(__WIN32__) || defined(__WIN64__)
    SetEvent(events_[MAXIMUM_WAIT_OBJECTS - 1]);
#elif HAVE_KQUEUE
    struct kevent ev;
    EV_SET(&ev,1000,EVFILT_TIMER,EV_ADD | EV_ONESHOT,0,0,0);
    if( kevent(kqueue_,&ev,1,NULL,0,NULL) == -1 ){
      int32_t err = errno;
      throw ExceptionSP(
        new Exception(err,utf8::string(__PRETTY_FUNCTION__))
      );
    }
#endif
    post();
  }
  return *this;
}
//---------------------------------------------------------------------------
#if HAVE_KQUEUE
AsyncIoSlave & AsyncIoSlave::cancelEvent(const IoRequest & request)
{
  struct kevent ke;
  EV_SET(&ke,request.descriptor_->socket_,EVFILT_READ | EVFILT_WRITE,EV_DELETE,0,0,0);
  if( kevent(kqueue_,&ke,1,NULL,0,NULL) == -1 ){
    perror(NULL);
    assert( 0 );
    abort();
  }
  EV_SET(&ke,1000,EVFILT_TIMER,EV_ADD | EV_ONESHOT,0,0,const_cast<IoRequest *>(&request));
  if( kevent(kqueue_,&ke,1,NULL,0,NULL) == -1 ){
    perror(NULL);
  }
  return *this;
}
#endif
//---------------------------------------------------------------------------
void AsyncIoSlave::execute()
{
  priority(THREAD_PRIORITY_HIGHEST);
#if defined(__WIN32__) || defined(__WIN64__)
  BOOL rw;
  DWORD nb;
  intptr_t sp = -1;
  bool isw9x = isWin9x();
#elif HAVE_KQUEUE
  struct aiocb * iocb;
  int32_t error;
  int64_t count;
#endif
  AsyncEvent * ev;
  EmbeddedListNode<IoRequest> * node;
  for(;;){
    acquire();
    if( requests_.count() == 0 ){
      release();
      Semaphore::wait();
      acquire();
      if( terminated_ ) break;
    }
    for( node = newRequests_.first(); node != NULL; node = newRequests_.first() ){
#if defined(__WIN32__) || defined(__WIN64__)
      node->object()->ehi_ = ++sp;
      eReqs_[node->object()->ehi_] = node;
      memset(&node->object()->overlapped_,0,sizeof(node->object()->overlapped_));
      node->object()->overlapped_.Offset = (DWORD) node->object()->position_;
      node->object()->overlapped_.OffsetHigh = (DWORD) (node->object()->position_ >> 32);
      node->object()->overlapped_.hEvent = events_[node->object()->ehi_];
      if( node->object()->length_ > 0x40000000 ) node->object()->length_ = 0x40000000;
l1:   SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
      SetLastError(ERROR_SUCCESS);
#endif
      switch( node->object()->ioType_ ){
        case etRead   :
#if defined(__WIN32__) || defined(__WIN64__)
          rw = node->object()->descriptor_->Read(
            node->object()->buffer_,
            (DWORD) node->object()->length_,
            &nb,
            isw9x ? NULL : &node->object()->overlapped_
          );
#elif HAVE_KQUEUE
          iocb = &node->object()->iocb_;
          iocb->aio_fildes = node->object()->descriptor_->descriptor_;
          iocb->aio_nbytes = node->object()->length_;
	  iocb->aio_buf = node->object()->buffer_;
          iocb->aio_offset = node->object()->position_;
	  iocb->aio_sigevent.sigev_value.sigval_ptr = node; // udata
          iocb->aio_sigevent.sigev_notify_kqueue = kqueue_;
          iocb->aio_sigevent.sigev_notify = SIGEV_KEVENT;
#if !HAVE_AIO_READ
#error async io not implemented because you system not have aio_read system call
#endif
          if( aio_read(iocb) == 0 ) errno = EINPROGRESS;
#endif
          break;
        case etWrite  :
#if defined(__WIN32__) || defined(__WIN64__)
          rw = node->object()->descriptor_->Write(
            node->object()->buffer_,
            (DWORD) node->object()->length_,
            &nb,
            isw9x ? NULL : &node->object()->overlapped_
          );
#elif HAVE_KQUEUE
          iocb = &node->object()->iocb_;
          iocb->aio_fildes = node->object()->descriptor_->descriptor_;
          iocb->aio_nbytes = node->object()->length_;
	  iocb->aio_buf = node->object()->buffer_;
          iocb->aio_offset = node->object()->position_;
	  iocb->aio_sigevent.sigev_value.sigval_ptr = node; // udata
          iocb->aio_sigevent.sigev_notify_kqueue = kqueue_;
          iocb->aio_sigevent.sigev_notify = SIGEV_KEVENT;
#if !HAVE_AIO_WRITE
#error async io not implemented because you system not have aio_write system call
#endif
          if( aio_write(iocb) == 0 ) errno = EINPROGRESS;
#endif
          break;
        case etAccept :
#if defined(__WIN32__) || defined(__WIN64__)
          rw = node->object()->descriptor_->AcceptEx(
            (SOCKET) node->object()->buffer_,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            isw9x ? NULL : &node->object()->overlapped_
          );
#elif HAVE_KQUEUE
          EV_SET(&kevents_[0],node->object()->descriptor_->socket_,EVFILT_READ,EV_ADD | EV_ONESHOT,0,0,node);
          if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) != -1 )
            count = node->object()->descriptor_->accept();
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
          rw = node->object()->descriptor_->Connect(
            events_[node->object()->ehi_],node->object()
          );
#elif HAVE_KQUEUE
          EV_SET(&kevents_[0],node->object()->descriptor_->socket_,EVFILT_READ,EV_ADD | EV_ONESHOT,0,0,node);
          if( kevent(kqueue_,&kevents_[0],1,NULL,0,NULL) != -1 )
            node->object()->descriptor_->connect(node->object());
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
        if( (node->object()->length_ >>= 1) > 0 ) goto l1;
      if( rw == 0 && GetLastError() != ERROR_IO_PENDING && GetLastError() != WSAEWOULDBLOCK ){
        ResetEvent(events_[node->object()->ehi_]);
        node->object()->descriptor_->cluster()->postEvent(
	  node->object()->descriptor_,
	  GetLastError(),
	  node->object()->ioType_,
	  -1
	);
	node->object()->ioThread_ = NULL;
        newRequests_.remove(*node->object());
        sp--;
      }
      else {
        requests_.insToTail(newRequests_.remove(*node->object()));
        SetLastError(ERROR_SUCCESS);
      }
#elif HAVE_KQUEUE
      count = 0;
      if( errno == 0 ){
        assert( ev->event_ == etAccept || ev->event_ == etConnect );
	goto l1;
      }
      if( errno == EINPROGRESS ){
        requests_.insToTail(newRequests_.remove(*node->object()));
      }
      else if( errno != EINPROGRESS ){
	count = -1;
l1:     node->object()->descriptor_->cluster()->postEvent(
          node->object()->descriptor_,
	  errno,
	  node->object()->ioType_,
	  count
        );
	node->object()->ioThread_ = NULL;
        newRequests_.remove(*node->object());
      }
#endif
    }
    release();
    node = NULL;
    if( requests_.count() > 0 ){
#if defined(__WIN32__) || defined(__WIN64__)
      if( isw9x ){
        node = requests_.first();
      }
      else {
        DWORD wm = WaitForMultipleObjectsEx(
          MAXIMUM_WAIT_OBJECTS,events_,FALSE,INFINITE,TRUE);
#if __INTEL_COMPILER
        if( wm < WAIT_OBJECT_0 + sp + 1 ){
#else
        if( wm >= WAIT_OBJECT_0 && wm < WAIT_OBJECT_0 + sp + 1 ){
#endif
          assert( wm < MAXIMUM_WAIT_OBJECTS - 1 );
          node = eReqs_[wm - WAIT_OBJECT_0];
          assert( node != NULL && wm == (DWORD) node->object_->ehi_ );
          ResetEvent(events_[wm]);
          SetLastError(ERROR_SUCCESS);
          if( node->object()->ioType_ != etConnect ){
            node->object()->descriptor_->GetOverlappedResult(
              &node->object()->overlapped_,
              &nb,
              TRUE
            );
          }
          else {
            nb = 0;
            node->object()->descriptor_->WSAEnumNetworkEvents(events_[node->object()->ehi_],FD_CONNECT_BIT);
          }
        }
        else if( wm == WAIT_OBJECT_0 + MAXIMUM_WAIT_OBJECTS - 1	){
          ResetEvent(events_[wm]);
        }
        else if( wm == WAIT_IO_COMPLETION ){
          assert( 0 );
        }
        else if( wm >= STATUS_ABANDONED_WAIT_0 && wm < STATUS_ABANDONED_WAIT_0 + sp + 1 ){
          assert( 0 );
        }
        else if( wm == WAIT_TIMEOUT ){
          assert( 0 );
        }
        else {
          assert( 0 );
        }
      }
      if( node != NULL ){
        xchg(events_[node->object()->ehi_],events_[sp]);
        eReqs_[node->object()->ehi_] = eReqs_[sp];
        eReqs_[sp]->object_->ehi_ = node->object()->ehi_;
        eReqs_[sp] = NULL;
        node->object()->descriptor_->cluster()->postEvent(
	  node->object()->descriptor_,
	  GetLastError(),
	  node->object()->ioType_,
	  GetLastError() != ERROR_SUCCESS ? -1 : nb
	);
	node->object()->ioThread_ = NULL;
        requests_.remove(*node->object());
        sp--;
      }
#elif HAVE_KQUEUE
      int count = kevent(kqueue_,NULL,0,&kevents_[0],kevents_.count(),NULL);
      if( count == -1 ){
        perror(NULL);
        assert( 0 );
        abort();
      }
      while( --count >= 0 ){
        struct kevent * kev = &kevents_[count];
        if( kev->filter == EVFILT_TIMER ){
          IoRequest * request = (IoRequest *) kev->udata;
          if( request == NULL || !request->node(*request).inserted() ) continue;
          node = &request->node(*request);
          assert( node->object()->ioType_ == etAccept );
	  kev->filter = EVFILT_READ;
          kev->flags |= EV_ERROR;
          kev->data = EINTR;
        }
        else {
          node = (EmbeddedListNode<IoRequest> *) kev->udata;
        }
        switch( node->object()->ioType_ ){
          case etRead    :
          case etWrite   :
	    error = aio_error(&node->object()->iocb_);
	    count = aio_return(&node->object()->iocb_);
            break;
          case etAccept  :
	    assert( kev->filter == EVFILT_READ );
            if( kev->flags & EV_ERROR ){
	      error = kev->data;
	      count = -1;
	    }
	    else {
	      count = node->object()->descriptor_->accept();
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
        node->object()->descriptor_->cluster()->postEvent(
  	  node->object()->descriptor_,
	  error,
	  node->object()->ioType_,
	  count
	);
	node->object()->ioThread_ = NULL;
        requests_.remove(*node->object());
      }
#endif
    }
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AsyncIoMaster::~AsyncIoMaster()
{
}
//---------------------------------------------------------------------------
AsyncIoMaster::AsyncIoMaster()
{
}
//---------------------------------------------------------------------------
void AsyncIoMaster::execute()
{
  intptr_t i;
  priority(THREAD_PRIORITY_HIGHEST);
  for(;;){
    AsyncDescriptorsCluster::ioRequestsSemaphore().wait();
    if( terminated_ ) break;
    try {
      AutoLock<InterlockedMutex> lock(AsyncDescriptorsCluster::ioRequestsMutex());
      for(;;){
        for( i = slaves_.count() - 1; i >= 0; i-- ){
          if( slaves_[i].finished() ){
            slaves_.remove(i);
            continue;
          }
          if( !slaves_[i].terminated() )
	    slaves_[i].transplant(AsyncDescriptorsCluster::ioRequests());
          if( AsyncDescriptorsCluster::ioRequests().count() == 0 ) break;
        }
        if( AsyncDescriptorsCluster::ioRequests().count() == 0 ) break;
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
	slave.ptr(NULL)->transplant(AsyncDescriptorsCluster::ioRequests());
        if( slaves_.count() > numberOfProcessors() ) slave->terminate();
      }
    }
    catch( ... ){
      AsyncDescriptorsCluster::ioRequestsSemaphore().post();
    }
  }
  for( i = slaves_.count() - 1; i >= 0; i-- ){
    slaves_[i].terminate();
    slaves_[i].post();
    slaves_[i].Thread::wait();
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
uint8_t AsyncDescriptorsCluster::ioMaster_[sizeof(AsyncIoMaster)];
uint8_t AsyncDescriptorsCluster::ioRequests_[sizeof(EmbeddedList<IoRequest,IoRequest::node>)];
uint8_t AsyncDescriptorsCluster::ioRequestsSemaphore_[sizeof(Semaphore)];
uint8_t AsyncDescriptorsCluster::ioRequestsMutex_[sizeof(InterlockedMutex)];
//------------------------------------------------------------------------------
/*LRESULT CALLBACK AsyncDescriptorsCluster::wndProc(
  HWND hWnd, // handle of window
  UINT uMsg, // message identifier
  WPARAM wParam, // first message parameter
  LPARAM lParam) // second message parameter
{
  AsyncEventType event;
  AsyncDescriptor * descriptor;
  if( uMsg == WM_ASYNCIO ){
    ENTER_MUTEX_SECTION(mutex());
    descriptor = descriptors().objectOfKey(AsyncDescriptorKey((SOCKET) wParam,0));
    LEAVE_MUTEX_SECTION;
    assert( descriptor != NULL );
    event = etError;
    if( WSAGETSELECTERROR(lParam) == 0 ){
      switch( WSAGETSELECTEVENT(lParam) ){
        case FD_READ    :
          event = etRead;
          break;
        case FD_WRITE   :
          event = etWrite;
          break;
        case FD_ACCEPT  :
          event = etAccept;
          break;
        case FD_CONNECT :
          event = etConnect;
          break;
        case FD_CLOSE   :
          event = etClose;
          break;
      }
    }
    descriptor->cluster_->postEvent(
      new AsyncEvent(descriptor,WSAGETSELECTERROR(lParam),event,0)
    );
    return 0;
  }
  return DefWindowProc(hWnd,uMsg,wParam,lParam);
}*/
//---------------------------------------------------------------------------
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
