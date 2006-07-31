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
#ifndef _shmem_H_
#define _shmem_H_
//---------------------------------------------------------------------------
namespace ksys {

//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class SharedMemory {
  public:
                    ~SharedMemory();
                    SharedMemory(const utf8::String & name = utf8::String(), uintptr_t len = 0, const void * address = NULL,
#if HAVE_SYS_MMAN_H
      uintptr_t mode = S_IRWXU
#elif defined(__WIN32__) || defined(__WIN64__)
                    uintptr_t mode = 0   // not used only for compatability
#endif
                    );
    static void     unlink(const utf8::String & name);
    void *&         memory();
    void * const &  memory() const;
    const bool &    creator() const;
  protected:
    union {
        void *      memory_;
        char *      c_;
        int8_t *    i8_;
        uint8_t *   u8_;
        int16_t *   i16_;
        uint16_t *  u16_;
        int32_t *   i32_;
        uint32_t *  u32_;
        int64_t *   i64_;
        uint64_t *  u64_;
        intptr_t *  ip_;
        uintptr_t * up_;
    };
    uintptr_t         length_;
#if HAVE_SYS_MMAN_H
    int               file_;
    utf8::AnsiString  name_;
#elif defined(__WIN32__) || defined(__WIN64__)
    HANDLE            file_;
    HANDLE            map_;
#endif
    SharedSemaphore   semaphore_;
  private:
    SharedMemory(const SharedMemory &)
    #if HAVE_SYS_MMAN_H

      : name_(utf8::String().getANSIString())
#endif

    {
    }
    SharedMemory & operator =(const SharedMemory &)
    {
      return *this;
    }
};
//---------------------------------------------------------------------------
inline void *& SharedMemory::memory()
{
  return memory_;
}
//---------------------------------------------------------------------------
inline void * const & SharedMemory::memory() const
{
  return memory_;
}
//---------------------------------------------------------------------------
inline const bool & SharedMemory::creator() const
{
  return semaphore_.creator();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ClassSMQRDLCK {
};
class ClassSMQRDULK {
};
class ClassSMQWRLCK {
};
class ClassSMQWRULK {
};

extern ClassSMQRDLCK  SMQ_RDL;
extern ClassSMQRDULK  SMQ_RDU;
extern ClassSMQWRLCK  SMQ_WRL;
extern ClassSMQWRULK  SMQ_WRU;
//---------------------------------------------------------------------------
class SharedMemoryQueue : public SharedMemory {
  public:
                        ~SharedMemoryQueue();
                        SharedMemoryQueue(const utf8::String & name = utf8::String(), uintptr_t len = 0, const void * address = NULL,
#if HAVE_SYS_MMAN_H
      uintptr_t mode = S_IRWXU
#elif defined(__WIN32__) || defined(__WIN64__)
                        uintptr_t mode = 0   // not used only for compatability
#endif
                        );

    static void         unlink(const utf8::String & name);
    SharedMemoryQueue & ref();

    SharedMemoryQueue & rdQueueRDLock();
    SharedMemoryQueue & rdQueueRDUnlock();
    SharedMemoryQueue & rdQueueWRLock();
    SharedMemoryQueue & rdQueueWRUnlock();
    SharedMemoryQueue & wrQueueRDLock();
    SharedMemoryQueue & wrQueueRDUnlock();
    SharedMemoryQueue & wrQueueWRLock();
    SharedMemoryQueue & wrQueueWRUnlock();
    SharedMemoryQueue & swap();

    SharedMemoryQueue & read(void * buf, uintptr_t len);
    SharedMemoryQueue & write(const void * buf, uintptr_t len);

    template< typename T> SharedMemoryQueue & operator <<(const T & a)
    {
      return write(&a, sizeof(a));
    }
    template< typename T> SharedMemoryQueue & operator >>(T & a)
    {
      return read(&a, sizeof(a));
    }

    SharedMemoryQueue & operator <<(const ClassSMQRDLCK &);
    SharedMemoryQueue & operator <<(const ClassSMQRDULK &);
    SharedMemoryQueue & operator <<(const ClassSMQWRLCK &);
    SharedMemoryQueue & operator <<(const ClassSMQWRULK &);

    SharedMemoryQueue & operator <<(int8_t a);
    SharedMemoryQueue & operator <<(uint8_t a);
    SharedMemoryQueue & operator <<(int16_t a);
    SharedMemoryQueue & operator <<(uint16_t a);
    SharedMemoryQueue & operator <<(int32_t a);
    SharedMemoryQueue & operator <<(uint32_t a);
    SharedMemoryQueue & operator <<(int64_t a);
    SharedMemoryQueue & operator <<(uint64_t a);
#if !HAVE_INT32_T_AS_INT
    SharedMemoryQueue & operator <<(int a);
    SharedMemoryQueue & operator <<(unsigned int a);
#endif
#if !HAVE_INTPTR_T_AS_INT && !HAVE_INTPTR_T_AS_INT64_T
    SharedMemoryQueue & operator <<(intptr_t a);
    SharedMemoryQueue & operator <<(uintptr_t a);
#endif
#if !HAVE_INTPTR_T_AS_INTMAX_T && !HAVE_INT64_T_AS_INTMAX_T
    SharedMemoryQueue & operator <<(intmax_t a);
    SharedMemoryQueue & operator <<(uintmax_t a);
#endif
    SharedMemoryQueue & operator <<(float a);
    SharedMemoryQueue & operator <<(double a);
#if HAVE_LONG_DOUBLE
    SharedMemoryQueue & operator <<(long double a);
#endif
    SharedMemoryQueue & operator <<(const utf8::String & a);

    SharedMemoryQueue & operator >>(int8_t & a);
    SharedMemoryQueue & operator >>(uint8_t & a);
    SharedMemoryQueue & operator >>(int16_t & a);
    SharedMemoryQueue & operator >>(uint16_t & a);
    SharedMemoryQueue & operator >>(int32_t & a);
    SharedMemoryQueue & operator >>(uint32_t & a);
    SharedMemoryQueue & operator >>(int64_t & a);
    SharedMemoryQueue & operator >>(uint64_t & a);
#if !HAVE_INT32_T_AS_INT
    SharedMemoryQueue & operator >>(int & a);
    SharedMemoryQueue & operator >>(unsigned int & a);
#endif
#if !HAVE_INTPTR_T_AS_INT && !HAVE_INTPTR_T_AS_INT32_T && !HAVE_INTPTR_T_AS_INT64_T
    SharedMemoryQueue & operator >>(intptr_t & a);
    SharedMemoryQueue & operator >>(uintptr_t & a);
#endif
#if !HAVE_INTPTR_T_AS_INTMAX_T && !HAVE_INT64_T_AS_INTMAX_T
    SharedMemoryQueue & operator >>(intmax_t & a);
    SharedMemoryQueue & operator >>(uintmax_t & a);
#endif
    SharedMemoryQueue & operator >>(float & a);
    SharedMemoryQueue & operator >>(double & a);
#if HAVE_LONG_DOUBLE
    SharedMemoryQueue & operator >>(long double & a);
#endif
    SharedMemoryQueue & operator >>(utf8::String & a);
  protected:
  private:
    SharedMemoryQueue(const SharedMemory &)
    {
    }
    SharedMemoryQueue(const SharedMemoryQueue &)
    {
    }
    SharedMemoryQueue & operator =(const SharedMemory &)
    {
      return *this;
    }
    SharedMemoryQueue & operator =(const SharedMemoryQueue &)
    {
      return *this;
    }

    SharedSemaphore rdQueueRDLockObject_;
    SharedSemaphore rdQueueWRLockObject_;
    SharedSemaphore wrQueueRDLockObject_;
    SharedSemaphore wrQueueWRLockObject_;
    SharedSemaphore rdQueueMutexObject_;
    SharedSemaphore wrQueueMutexObject_;
    SharedSemaphore rdQueueRDIndicatorObject_;
    SharedSemaphore rdQueueWRIndicatorObject_;
    SharedSemaphore wrQueueRDIndicatorObject_;
    SharedSemaphore wrQueueWRIndicatorObject_;
    bool            rdQueueRDLockedV_;
    bool            rdQueueWRLockedV_;
    bool            wrQueueRDLockedV_;
    bool            wrQueueWRLockedV_;
    struct Queue {
      intptr_t  pid_;
      uintptr_t head_;
      uintptr_t tail_;
      uintptr_t bound_;
#if _MSC_VER
#pragma warning(disable:4200)
#endif
      uint8_t   data_[EMPTY_ARRAY_SIZE];
#if _MSC_VER
#pragma warning(default:4200)
#endif

      SharedMemoryQueue & read(SharedMemoryQueue * queue, void * buf, uintptr_t len);
      SharedMemoryQueue & write(SharedMemoryQueue * queue, const void * buf, uintptr_t len);
    };
    friend struct Queue;
    SharedSemaphore * rdQueueRDLock_;
    SharedSemaphore * rdQueueWRLock_;
    SharedSemaphore * wrQueueRDLock_;
    SharedSemaphore * wrQueueWRLock_;
    SharedSemaphore * wrQueueLock_;
    SharedSemaphore * rdQueueMutex_;
    SharedSemaphore * wrQueueMutex_;
    SharedSemaphore * rdQueueRDIndicator_;
    SharedSemaphore * rdQueueWRIndicator_;
    SharedSemaphore * wrQueueRDIndicator_;
    SharedSemaphore * wrQueueWRIndicator_;
    Queue *           rdQueue_;
    Queue *           wrQueue_;
    bool *            rdQueueRDLocked_;
    bool *            rdQueueWRLocked_;
    bool *            wrQueueRDLocked_;
    bool *            wrQueueWRLocked_;
};
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::ref()
{
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::rdQueueRDLock()
{
  rdQueueRDLock_->wait();
  *rdQueueRDLocked_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::rdQueueRDUnlock()
{
  rdQueueRDLock_->post();
  *rdQueueRDLocked_ = false;
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::rdQueueWRLock()
{
  rdQueueWRLock_->wait();
  *rdQueueWRLocked_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::rdQueueWRUnlock()
{
  rdQueueWRLock_->post();
  *rdQueueWRLocked_ = false;
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::wrQueueRDLock()
{
  wrQueueRDLock_->wait();
  *wrQueueRDLocked_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::wrQueueRDUnlock()
{
  wrQueueRDLock_->post();
  *wrQueueRDLocked_ = false;
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::wrQueueWRLock()
{
  wrQueueWRLock_->wait();
  *wrQueueWRLocked_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::wrQueueWRUnlock()
{
  wrQueueWRLock_->post();
  *wrQueueWRLocked_ = false;
  return *this;
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::read(void * buf, uintptr_t len)
{
  return rdQueue_->read(this, buf, len);
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::write(const void * buf, uintptr_t len)
{
  return wrQueue_->write(this, buf, len);
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(const ClassSMQRDLCK &)
{
  return rdQueueRDLock();
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(const ClassSMQRDULK &)
{
  return rdQueueRDUnlock();
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(const ClassSMQWRLCK &)
{
  return wrQueueWRLock();
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(const ClassSMQWRULK &)
{
  return wrQueueWRUnlock();
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(int8_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(uint8_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(int16_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(uint16_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(int32_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(uint32_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(int64_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(uint64_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#if !HAVE_INT32_T_AS_INT
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(int a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(unsigned int a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT && !HAVE_INTPTR_T_AS_INT32_T && !HAVE_INTPTR_T_AS_INT64_T
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(intptr_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(uintptr_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T && !HAVE_INT64_T_AS_INTMAX_T
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(intmax_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(uintmax_t a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(float a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(double a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(long double a)
{
  return write(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator <<(const utf8::String & a)
{
  uintptr_t l = a.size();
  return write(&l, sizeof(l)).write(a.c_str(), l);
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(int8_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(uint8_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(int16_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(uint16_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(int32_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(uint32_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(int64_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(uint64_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#if !HAVE_INT32_T_AS_INT
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(int & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(unsigned int & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT && !HAVE_INTPTR_T_AS_INT32_T && !HAVE_INTPTR_T_AS_INT64_T
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(intptr_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(uintptr_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T && !HAVE_INT64_T_AS_INTMAX_T
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(intmax_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(uintmax_t & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(float & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(double & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
//---------------------------------------------------------------------------
inline SharedMemoryQueue & SharedMemoryQueue::operator >>(long double & a)
{
  return read(&a, sizeof(a));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _shmem_H_ */
//---------------------------------------------------------------------------
