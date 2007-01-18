//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#if HAVE_SYS_IPC_H && HAVE_SYS_SEM_H
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
SVSharedSemaphore::~SVSharedSemaphore()
{
#ifndef NDEBUG
  int r;
#endif
  if( id_ >= 0 && creator() ){
#ifndef NDEBUG
    r =
#endif
    semctl(id_, 0, IPC_RMID, 0);
#ifndef NDEBUG
    assert(r == 0 || errno == EINVAL);
#endif
  }
}
//---------------------------------------------------------------------------
SVSharedSemaphore::SVSharedSemaphore(const utf8::String & name, uintptr_t flags, uintptr_t count)
  : id_(-1),
    count_(count),
    creator_(false)
{
  id_ = semget(name.strlen() > 0 ? getKey(name) : IPC_PRIVATE, (int) count, (int) (flags | IPC_CREAT | IPC_EXCL));
  if( id_ == -1 ){
    if( errno == EEXIST )
      id_ = semget(name.strlen() > 0 ? getKey(name) : IPC_PRIVATE, (int) count, (int) (flags & ~(IPC_CREAT | IPC_EXCL)));
    if( id_ == -1 ){
      int32_t err = errno;
      newObject<Exception>(err, name + ", key=" + utf8::int2Str((uintmax_t) getKey(name)) + ", " + __PRETTY_FUNCTION__)->throwSP();
    }
  }
  else{
    creator_ = true;
    // this is need for emulation behavior of POSIX semaphores
    // because by default System V semaphores initialized in zero
    for( intptr_t i = count - 1; i >= 0; i-- )
      post(i);
  }
}
//---------------------------------------------------------------------------
key_t SVSharedSemaphore::getKey(const utf8::String & name)
{
  uintptr_t h = name.hash(true);
  key_t k = hashT< key_t>(&h, sizeof(h));
  if( k == IPC_PRIVATE && name.strlen() > 0 )
    newObject<Exception>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  return k;
}
//---------------------------------------------------------------------------
SVSharedSemaphore & SVSharedSemaphore::post(uintptr_t sem)
{
  assert(sem < (uintptr_t) count_);
  struct sembuf op;
  op.sem_num = sem;
  op.sem_op = 1;
  op.sem_flg = SEM_UNDO;
  if( semop(id_, &op, 1) != 0 ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
SVSharedSemaphore & SVSharedSemaphore::wait(uintptr_t sem)
{
  assert(sem < (uintptr_t) count_);
  struct sembuf op;
  op.sem_num = sem;
  op.sem_op = -1;
  op.sem_flg = 0/*SEM_UNDO*/;
  if( semop(id_, &op, 1) != 0 ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
bool SVSharedSemaphore::tryWait(uintptr_t sem)
{
  assert(sem < (uintptr_t) count_);
  struct sembuf op;
  op.sem_num = sem;
  op.sem_op = -1;
  op.sem_flg = /*SEM_UNDO | */IPC_NOWAIT;
  errno = 0;
  if( semop(id_, &op, 1) != 0 && errno != EAGAIN ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return errno == 0;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Semaphore::~Semaphore()
{
#if USE_SV_SEMAPHORES
#elif HAVE_SEMAPHORE_H
  if( handle_ != NULL ){
#ifndef NDEBUG
    int r =
#endif
    sem_destroy(&handle_);
#ifndef NDEBUG
    assert(r == 0);
#endif
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  if( handle_ != NULL ){
    CloseHandle(handle_);
    handle_ = NULL;
  }
#endif
}
//---------------------------------------------------------------------------
Semaphore::Semaphore()
{
#if USE_SV_SEMAPHORES
#elif HAVE_SEMAPHORE_H
  handle_ = NULL;
  if( sem_init(&handle_, 0, 0) != 0 ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  handle_ = CreateSemaphoreA(NULL,0,~(ULONG) 0 >> 1,NULL);
  if( handle_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//---------------------------------------------------------------------------
#if !USE_SV_SEMAPHORES
//---------------------------------------------------------------------------
Semaphore & Semaphore::post()
{
#if HAVE_SEMAPHORE_H
  if( sem_post(&handle_) != 0 ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  if( ReleaseSemaphore(handle_,1,NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
Semaphore & Semaphore::wait()
{
#if HAVE_SEMAPHORE_H
  if( sem_wait(&handle_) != 0 ){
    int32_t err = errno;
    newObject<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  DWORD r = WaitForSingleObject(handle_,INFINITE);
  if( r != WAIT_OBJECT_0 && r != WAIT_ABANDONED ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
bool Semaphore::timedWait(uint64_t timeout)
{
#if HAVE_SEMAPHORE_H
#if HAVE_SEM_TIMEDWAIT
  struct timespec t;
  t.tv_sec = timeout / 1000000u;
  t.tv_nsec = long(timeout % 1000000u);
  int r = sem_timedwait(&handle_,&t);
  if( r != 0 && errno != ETIMEDOUT ){
    r = errno;
    newObject<Exception>(r,__PRETTY_FUNCTION__)->throwSP();
  }
  return r == 0;
#else
  bool r;
  for(;;){
    r = tryWait();
    if( r ) break;
    if( timeout == 0 ) break;
    timeout--;
    ksleep(1);
  }
//  sem_timedwait();
//  newObject<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
  return r;
#endif
#elif defined(__WIN32__) || defined(__WIN64__)
  uint64_t t = timeout / 1000u + (timeout > 0 && timeout < 1000u);
  DWORD r = WaitForSingleObject(handle_,t > ~DWORD(0) - 1 ? ~DWORD(0) - 1 : DWORD(t));
  if( r == WAIT_FAILED ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return r == WAIT_OBJECT_0 || r == WAIT_ABANDONED;
#endif
}
//---------------------------------------------------------------------------
bool Semaphore::tryWait()
{
#if HAVE_SEMAPHORE_H
  errno = 0;
  int r = sem_trywait(&handle_);
  if( r != 0 && errno != EAGAIN ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return r == 0;
#elif defined(__WIN32__) || defined(__WIN64__)
  DWORD r = WaitForSingleObject(handle_,0);
  if( r == WAIT_FAILED ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return r == WAIT_OBJECT_0 || r == WAIT_ABANDONED;
#endif
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
SharedSemaphore::~SharedSemaphore()
{
#if USE_SV_SEMAPHORES
#elif HAVE_SEMAPHORE_H
#ifndef NDEBUG
  int r;
#endif
  if( handle_ != SEM_FAILED ){
#ifndef NDEBUG
    r = 
#endif
    sem_close(handle_);
#ifndef NDEBUG
    assert(r == 0 || errno == EINVAL);
#endif
    if( creator() ){
#ifndef NDEBUG
      r =
#endif
      sem_unlink(name_);
#ifndef NDEBUG
      assert(r == 0);
#endif
    }
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  if( handle_ != NULL )
    CloseHandle(handle_);
#endif
}
//---------------------------------------------------------------------------
#if USE_SV_SEMAPHORES
SharedSemaphore::SharedSemaphore(const utf8::String & name, uintptr_t mode)
:
  SVSharedSemaphore(name, mode)
#elif HAVE_SEMAPHORE_H
SharedSemaphore::SharedSemaphore(const utf8::String & name, uintptr_t mode)
  : handle_(SEM_FAILED),
    name_(genName(name).getANSIString()),
    creator_(false)
#elif defined(__WIN32__) || defined(__WIN64__)
SharedSemaphore::SharedSemaphore(const utf8::String & name, uintptr_t mode)
  : handle_(NULL),
    creator_(false)
#endif
{
#if USE_SV_SEMAPHORES
#elif HAVE_SEMAPHORE_H
#ifndef NDEBUG
  fprintf(stderr,"%s %s\n",(const char *) name_,__PRETTY_FUNCTION__);
#endif
  handle_ = sem_open(name_, O_EXCL | O_CREAT, (mode_t) mode, 0);
  if( handle_ == SEM_FAILED ){
    if( errno == EEXIST )
      handle_ = sem_open(name_, 0, (mode_t) mode, 0);
  }
  else {
    creator_ = true;
  }
  if( handle_ == SEM_FAILED ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  if( isWin9x() )
    handle_ = CreateSemaphoreA(NULL, 0, ~(ULONG) 0 >> 1, name.getANSIString());
  else
    handle_ = CreateSemaphoreW(NULL, 0, ~(ULONG) 0 >> 1, name.getUNICODEString());
  creator_ = handle_ != NULL;
  if( handle_ == NULL && GetLastError() == ERROR_ALREADY_EXISTS ){
    if( isWin9x() )
      handle_ = OpenSemaphoreA(DWORD(mode), FALSE, name.getANSIString());
    else
      handle_ = OpenSemaphoreW(DWORD(mode), FALSE, name.getUNICODEString());
  }
  if( handle_ == NULL ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#endif
}
//---------------------------------------------------------------------------
utf8::String SharedSemaphore::genName(const utf8::String & name)
{
  uint64_t h = name.hash_ll(true);
  return pathDelimiterStr + base32Encode(&h,sizeof(h));
}
//---------------------------------------------------------------------------
void SharedSemaphore::unlink(const utf8::String & name)
{
#if USE_SV_SEMAPHORES
  int id  = semget(getKey(name), 0, SEM_A | SEM_R);
#ifndef NDEBUG
  int r;
#endif
  if( id >= 0 ){
#ifndef NDEBUG
    r =
#endif
    semctl(id, 0, IPC_RMID, 0);
#ifndef NDEBUG
    assert(r == 0);
#endif
  }
#elif HAVE_SEMAPHORE_H
  //  fprintf(stderr,"%s %s\n",(const char *) genName(name).getANSIString(),__PRETTY_FUNCTION__);
  sem_t * handle  = sem_open(genName(name).getANSIString(), 0, S_IWUSR, 0);
  if( handle != SEM_FAILED ){
#ifndef NDEBUG
    int r;
#endif
#ifndef NDEBUG
    r = 
#endif
    sem_close(handle);
#ifndef NDEBUG
    assert(r == 0);
#endif
#ifndef NDEBUG
    r =
#endif
    sem_unlink(genName(name).getANSIString());
#ifndef NDEBUG
    assert(r == 0);
#endif
  }
#else
  utf8::String s(name);
  s = name;
#endif
}
//---------------------------------------------------------------------------
#if !USE_SV_SEMAPHORES
//---------------------------------------------------------------------------
SharedSemaphore & SharedSemaphore::post()
{
#if HAVE_SEMAPHORE_H
  if( sem_post(handle_) != 0 ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  if( ReleaseSemaphore(handle_, 1, NULL) == 0 ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
SharedSemaphore & SharedSemaphore::wait()
{
#if HAVE_SEMAPHORE_H
  if( sem_wait(handle_) != 0 ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  DWORD r = WaitForSingleObject(handle_, INFINITE);
  if( r == WAIT_FAILED || r != WAIT_OBJECT_0 ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
#endif
  return *this;
}
//---------------------------------------------------------------------------
bool SharedSemaphore::tryWait()
{
#if HAVE_SEMAPHORE_H
  int r = sem_trywait(handle_);
  if( r != 0 || errno != EAGAIN ){
    int32_t err = errno;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return r == 0;
#elif defined(__WIN32__) || defined(__WIN64__)
  DWORD r = WaitForSingleObject(handle_, 0);
  if( r == WAIT_FAILED || r == WAIT_TIMEOUT ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return r == WAIT_OBJECT_0;
#endif
}
//---------------------------------------------------------------------------
bool SharedSemaphore::timedWait(uint64_t timeout)
{
#if HAVE_SEMAPHORE_H
  newObject<Exception>(ENOSYS,__PRETTY_FUNCTION__)->throwSP();
  return false;
#elif defined(__WIN32__) || defined(__WIN64__)
  uint64_t t = timeout / 1000u + (timeout > 0 && timeout < 1000u);
  DWORD r = WaitForSingleObject(handle_,t > ~DWORD(0) - 1 ? ~DWORD(0) - 1 : DWORD(t));
  if( r == WAIT_FAILED ){
    int32_t err = GetLastError() + errorOffset;
    newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
  }
  return r == WAIT_OBJECT_0 || r == WAIT_ABANDONED;
#endif
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
SharedMemory::~SharedMemory()
{
#if HAVE_SYS_MMAN_H
#ifndef NDEBUG
  int r;
#endif
  if( memory_ != NULL ){
#ifndef NDEBUG
    r =
#endif
    munmap(memory_, length_);
#ifndef NDEBUG
    assert(r == 0);
#endif
  }
  if( file_ >= 0 && creator() ){
#ifndef NDEBUG
    r =
#endif
    shm_unlink(name_);
#ifndef NDEBUG
    assert(r == 0);
#endif
  }
#elif defined(__WIN32__) || defined(__WIN64__)
  if( memory_ != NULL )
    UnmapViewOfFile(memory_);
  if( map_ != NULL )
    CloseHandle(map_);
  if( file_ != INVALID_HANDLE_VALUE )
    CloseHandle(file_);
#endif
}
//---------------------------------------------------------------------------
#if HAVE_SYS_MMAN_H
SharedMemory::SharedMemory(const utf8::String & name, uintptr_t length, const void * memory, uintptr_t mode)
  : memory_(NULL),
    length_(0),
    file_(-1),
    name_(name.strstr(utf8::String(pathDelimiterStr)).position() == 0 ? name.getANSIString() : (getTempPath() + name).getANSIString()),
#elif defined(__WIN32__) || defined(__WIN64__)
SharedMemory::SharedMemory(const utf8::String & name, uintptr_t length, const void * memory, uintptr_t mode)
  : memory_(NULL),
    length_(0),
    file_(INVALID_HANDLE_VALUE),
    map_(NULL),
#endif
    semaphore_(name)
{
  int32_t err;
  if( length == 0 ) length = getpagesize();
#if HAVE_SYS_MMAN_H
  int mmap_flags  = MAP_NOSYNC | MAP_NOCORE | MAP_SHARED;
  if( memory != NULL ) mmap_flags |= MAP_FIXED;
  if( name.strlen() > 0 ){
    file_ = shm_open(name_, O_RDWR | O_CREAT | O_EXCL | O_TRUNC, (mode_t) mode);
    if( file_ < 0 && errno == EEXIST ){
      //      if( creator() && shm_unlink(name_) != 0 && errno != ENOENT ) goto l1;
      file_ = shm_open(name_, O_RDWR, (mode_t) mode);
      if( file_ < 0 ) goto l1;
    }
  }
  else {
    mmap_flags |= MAP_ANON;
  }
  if( creator() ){
    if( ftruncate(file_, length) != 0 ) goto l1;
  }
  else {
    semaphore_.wait();
  }
  semaphore_.post();
  memory_ = mmap((void *) memory, length, PROT_READ | PROT_WRITE, mmap_flags, file_, 0);
  if( memory_ == MAP_FAILED )
    goto l1;
#elif defined(__WIN32__) || defined(__WIN64__)
  if( name.strlen() > 0 ){
    if( isWin9x() ){
      file_ = CreateFileA(name.strstr(pathDelimiterStr).position() == 0 ? name.getANSIString() : (getTempPath() + name).getANSIString(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
    }
    else
      file_ = CreateFileW(name.strstr(pathDelimiterStr).position() == 0 ? name.getUNICODEString() : (getTempPath() + name).getUNICODEString(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if( file_ == INVALID_HANDLE_VALUE ) goto l1;
  }
  if( isWin9x() )
    map_ = CreateFileMappingA(file_, NULL, DWORD(mode), DWORD((uint64_t) length_ >> 32), DWORD(length_), name.getANSIString());
  else
    map_ = CreateFileMappingW(file_, NULL, DWORD(mode), DWORD((uint64_t) length_ >> 32), DWORD(length_), name.getUNICODEString());
  if( map_ == NULL && GetLastError() == ERROR_ALREADY_EXISTS ){
    if( isWin9x() )
      map_ = OpenFileMappingA(PAGE_WRITECOPY, FALSE, name.getANSIString());
    else
      map_ = OpenFileMappingW(PAGE_WRITECOPY, FALSE, name.getUNICODEString());
  }
  if( map_ == NULL ) goto l1;
  //  if( mode == PAGE_READONLY ) mode = FILE_MAP_READ;
  //  else
  //  if( mode == PAGE_READWRITE ) mode = FILE_MAP_WRITE;
  //  else
  //  if( mode == PAGE_WRITECOPY ) mode = FILE_MAP_COPY;
  memory_ = MapViewOfFileEx(map_, FILE_MAP_COPY, 0, 0, DWORD(length), (void *) memory);
  if( memory_ == NULL ) goto l1;
#else
#error shared memory not implemented
#endif
  goto l2;
l1:
#if defined(__WIN32__) || defined(__WIN64__)
  err = GetLastError() + errorOffset;
#else
  err = errno;
#endif
  newObject<Exception>(err, __PRETTY_FUNCTION__)->throwSP();
l2:
  length_ = length;
}
//---------------------------------------------------------------------------
void SharedMemory::unlink(const utf8::String & name)
{
  SharedSemaphore::unlink(name);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
ClassSMQRDLCK SMQ_RDL;
ClassSMQRDULK SMQ_RDU;
ClassSMQWRLCK SMQ_WRL;
ClassSMQWRULK SMQ_WRU;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
SharedMemoryQueue::~SharedMemoryQueue()
{
}
//---------------------------------------------------------------------------
SharedMemoryQueue::SharedMemoryQueue(const utf8::String & name, uintptr_t length, const void * memory, uintptr_t mode)
  : SharedMemory(name, length, memory, mode),
    rdQueueRDLockObject_(name + ".rdrdl"),
    rdQueueWRLockObject_(name + ".rdwrl"),
    wrQueueRDLockObject_(name + ".wrrdl"),
    wrQueueWRLockObject_(name + ".wrwrl"),
    rdQueueMutexObject_(name + ".rdm"),
    wrQueueMutexObject_(name + ".wrm"),
    rdQueueRDIndicatorObject_(name + ".rdrdi"),
    rdQueueWRIndicatorObject_(name + ".rdwri"),
    wrQueueRDIndicatorObject_(name + ".wrrdi"),
    wrQueueWRIndicatorObject_(name + ".wrwri"),
    rdQueueRDLockedV_(false),
    rdQueueWRLockedV_(false),
    wrQueueRDLockedV_(false),
    wrQueueWRLockedV_(false)
{
  assert(name.strlen() > 0);
  assert(length_ >= sizeof(Queue) * 2 + 2 && (length_ & 1) == 0);
  rdQueue_ = (Queue *) memory_;
  wrQueue_ = (Queue *) (u8_ + length_ / 2);
  rdQueueRDLock_ = &rdQueueRDLockObject_;
  rdQueueWRLock_ = &rdQueueWRLockObject_;
  wrQueueRDLock_ = &wrQueueRDLockObject_;
  wrQueueWRLock_ = &wrQueueWRLockObject_;
  rdQueueMutex_ = &rdQueueMutexObject_;
  wrQueueMutex_ = &wrQueueMutexObject_;
  rdQueueRDIndicator_ = &rdQueueRDIndicatorObject_;
  rdQueueWRIndicator_ = &rdQueueWRIndicatorObject_;
  wrQueueRDIndicator_ = &wrQueueRDIndicatorObject_;
  wrQueueWRIndicator_ = &wrQueueWRIndicatorObject_;
  rdQueueRDLocked_ = &rdQueueRDLockedV_;
  rdQueueWRLocked_ = &rdQueueWRLockedV_;
  wrQueueRDLocked_ = &wrQueueRDLockedV_;
  wrQueueWRLocked_ = &wrQueueWRLockedV_;
  if( creator() ){
    // initializer must by only one
    rdQueue_->head_ = 0;
    rdQueue_->tail_ = 0;
    rdQueue_->bound_ = length_ / 2 - sizeof(Queue);
    wrQueue_->head_ = 0;
    wrQueue_->tail_ = 0;
    wrQueue_->bound_ = length_ / 2 - sizeof(Queue);
    rdQueueMutexObject_.post();
    wrQueueMutexObject_.post();
    rdQueueRDLockObject_.post();
    rdQueueWRLockObject_.post();
    wrQueueRDLockObject_.post();
    wrQueueWRLockObject_.post();
  }
  else{
    semaphore_.wait();
  }
  semaphore_.post();
}
//---------------------------------------------------------------------------
void SharedMemoryQueue::unlink(const utf8::String & name)
{
  SharedMemory::unlink(name);
  SharedSemaphore::unlink(name + ".rdrdl");
  SharedSemaphore::unlink(name + ".rdwrl");
  SharedSemaphore::unlink(name + ".wrrdl");
  SharedSemaphore::unlink(name + ".wrwrl");
  SharedSemaphore::unlink(name + ".rdm");
  SharedSemaphore::unlink(name + ".wrm");
  SharedSemaphore::unlink(name + ".rdrdi");
  SharedSemaphore::unlink(name + ".rdwri");
  SharedSemaphore::unlink(name + ".wrrdi");
  SharedSemaphore::unlink(name + ".wrwri");
}
//---------------------------------------------------------------------------
SharedMemoryQueue & SharedMemoryQueue::swap()
{
  xchg(rdQueue_, wrQueue_);
  xchg(rdQueueRDIndicator_, wrQueueRDIndicator_);
  xchg(rdQueueWRIndicator_, wrQueueWRIndicator_);
  xchg(rdQueueMutex_, wrQueueMutex_);
  xchg(rdQueueRDLock_, wrQueueRDLock_);
  xchg(rdQueueWRLock_, wrQueueWRLock_);
  xchg(rdQueueRDLocked_, wrQueueRDLocked_);
  xchg(rdQueueWRLocked_, wrQueueWRLocked_);
  return *this;
}
//---------------------------------------------------------------------------
SharedMemoryQueue & SharedMemoryQueue::Queue::read(SharedMemoryQueue * queue, void * buf, uintptr_t len)
{
  if( !*queue->rdQueueRDLocked_ )
    queue->rdQueueRDLock_->wait();
  try{
    while( len > 0 ){
      queue->rdQueueMutex_->wait();
      uintptr_t l = head_ < tail_ ? bound_ - tail_ : head_ - tail_;
      if( l == 0 ){
        queue->rdQueueMutex_->post();
        queue->rdQueueWRIndicator_->post(); // enable writes
        queue->rdQueueRDIndicator_->wait(); // wait for data
        continue;
      }
      if( l > len )
        l = len;
      if( buf != NULL )
        memcpy(buf, data_ + tail_, l);
      else
        memset(buf, 0, l);
      tail_ += l;
      assert(tail_ <= bound_);
      if( tail_ == bound_ )
        tail_ = 0;
      buf = (uint8_t *) buf + l;
      len -= l;
      queue->rdQueueMutex_->post();
    }
  }
  catch( ... ){
    if( !*queue->rdQueueRDLocked_ )
      queue->rdQueueRDLock_->post();
    throw;
  }
  if( !*queue->rdQueueRDLocked_ )
    queue->rdQueueRDLock_->post();
  queue->rdQueueWRIndicator_->post(); // enable writes
  return *queue;
}
//---------------------------------------------------------------------------
SharedMemoryQueue & SharedMemoryQueue::Queue::write(SharedMemoryQueue * queue, const void * buf, uintptr_t len)
{
  if( !*queue->wrQueueWRLocked_ )
    queue->wrQueueWRLock_->wait();
  try{
    while( len > 0 ){
      queue->wrQueueMutex_->wait();
      uintptr_t l = head_ >= tail_ ? bound_ - head_ + tail_ : tail_ - head_;
      if( l < 2 ){
        queue->wrQueueMutex_->post();
        queue->wrQueueRDIndicator_->post(); // enable reads
        queue->wrQueueWRIndicator_->wait(); // wait for write permission
        continue;
      }
      if( l > len )
        l = len;
      if( buf != NULL )
        memcpy(data_ + head_, buf, l);
      else
        memset(data_ + head_, 0, l);
      head_ += l;
      assert(head_ <= bound_);
      if( head_ == bound_ )
        head_ = 0;
      buf = (uint8_t *) buf + l;
      len -= l;
      queue->wrQueueMutex_->post();
    }
  }
  catch( ... ){
    if( !*queue->wrQueueWRLocked_ )
      queue->wrQueueWRLock_->post();
    throw;
  }
  if( !*queue->wrQueueWRLocked_ )
    queue->wrQueueWRLock_->post();
  queue->wrQueueRDIndicator_->post(); // enable reads
  return *queue;
}
//---------------------------------------------------------------------------
SharedMemoryQueue & SharedMemoryQueue::operator >>(utf8::String & a)
{
  uintptr_t l;
  read(&l, sizeof(l));
  try{
    a.resize(l);
  }
  catch( ... ){
    return read(NULL, l);
  }
  return read(a.c_str(), l);
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
