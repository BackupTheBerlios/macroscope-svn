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
#ifndef _thread_H_
#define _thread_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#ifndef PTHREAD_STACK_MIN
#if defined(__WIN32__) || defined(__WIN64__)
#define PTHREAD_STACK_MIN getpagesize()
#else
#define PTHREAD_STACK_MIN getpagesize()
#endif
#endif
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
#ifndef PTHREAD_DEFAULT_PRIORITY
#if __FreeBSD__
#define PTHREAD_DEFAULT_PRIORITY 15
#else
#define PTHREAD_DEFAULT_PRIORITY 0
#endif
#endif
#ifndef PTHREAD_MIN_PRIORITY
#if __FreeBSD__
#define PTHREAD_MIN_PRIORITY 0
#else
#define PTHREAD_MIN_PRIORITY 0
#endif
#endif
#ifndef PTHREAD_MAX_PRIORITY
#if __FreeBSD__
#define PTHREAD_MAX_PRIORITY 31
#else
#define PTHREAD_MAX_PRIORITY 0
#endif
#endif
#ifndef PTHREAD_RT_PRIORITY
#if __FreeBSD__
#define PTHREAD_RT_PRIORITY 64
#else
#define PTHREAD_RT_PRIORITY 0
#endif
#endif
#define THREAD_PRIORITY_ABOVE_NORMAL PTHREAD_DEFAULT_PRIORITY + 1
#define THREAD_PRIORITY_BELOW_NORMAL PTHREAD_DEFAULT_PRIORITY - 1
#define THREAD_PRIORITY_HIGHEST PTHREAD_MAX_PRIORITY
#define THREAD_PRIORITY_IDLE PTHREAD_MIN_PRIORITY
#define THREAD_PRIORITY_LOWEST PTHREAD_MIN_PRIORITY + 1
#define THREAD_PRIORITY_NORMAL PTHREAD_DEFAULT_PRIORITY
#define THREAD_PRIORITY_TIME_CRITICAL PTHREAD_RT_PRIORITY
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Thread {
  friend void initialize(int,char **);
  friend void cleanup();
  public:
    virtual ~Thread();
    Thread();

    Thread & resume();
    Thread & suspend();
    static bool isSuspended(uintptr_t tid);
    bool isSuspended() const;
    Thread & wait();

    uintptr_t id() const;
    const intptr_t & exitCode() const;
    bool active() const;

    const bool & started() const;
    Thread & terminate();
    const bool & terminated() const;
    const bool & finished() const;

    Thread & stackSize(uintptr_t newStackSize);
    const uintptr_t & stackSize() const;
    Thread & priority(uintptr_t pri);
    uintptr_t priority() const;

    class Action {
      public:
        ~Action();
        Action(void * handler = NULL, void * data = NULL);

        void * const &  handler() const;
        void * const &  data() const;

        bool operator == (const Action & action) const;
      protected:
      private:
        void *  handler_;
        void *  data_;
    };

    static Array<Action> & beforeExecuteActions();
    static Array<Action> & afterExecuteActions();
  protected:
    virtual void threadExecute() = 0;
    virtual void threadBeforeWait() {}
    uintptr_t       stackSize_;
#if defined(__WIN32__) || defined(__WIN64__)
    HANDLE          handle_;
    DWORD           id_;
#else
    pthread_t       handle_;
    //pthread_mutex_t mutex_;
#endif
    intptr_t        exitCode_;
    bool            started_;
    bool            terminated_;
    bool            finished_;
  private:
    static uint8_t  beforeExecuteActions_[];
    static uint8_t  afterExecuteActions_[];
#if defined(__WIN32__) || defined(__WIN64__)
    static DWORD            WINAPI threadFunc(LPVOID thread);
#else
    static void *           threadFunc(void * thread);
#endif
    static void             initialize();
    static void             cleanup();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline Thread::Action::~Action()
{
}
//---------------------------------------------------------------------------
inline Thread::Action::Action(void * handler, void * data) : handler_(handler), data_(data)
{
}
//---------------------------------------------------------------------------
inline void * const & Thread::Action::handler() const
{
  return handler_;
}
//---------------------------------------------------------------------------
inline void * const & Thread::Action::data() const
{
  return data_;
}
//---------------------------------------------------------------------------
inline bool Thread::Action::operator ==(const Action & action) const
{
  return handler_ == action.handler_ && data_ == action.data_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline Thread::~Thread()
{
//  assert(((started_ && finished_) || (!started_ && !finished_)) && handle_ == NULL);
  wait();
}
//---------------------------------------------------------------------------
inline bool Thread::isSuspended() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  return isSuspended(id_);
#else
  return false;
#endif
}
//---------------------------------------------------------------------------
inline uintptr_t Thread::id() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  return id_;
#else
  return (uintptr_t) handle_;
#endif
}
//---------------------------------------------------------------------------
inline const intptr_t & Thread::exitCode() const
{
  return exitCode_;
}
//---------------------------------------------------------------------------
inline bool Thread::active() const
{
#if defined(__WIN32__) || defined(__WIN64__)
  DWORD exitCode;
  return
    (GetExitCodeThread(handle_,&exitCode) != 0 && exitCode == STILL_ACTIVE) ||
    (started_ && !finished_)
  ;
#else
  return started_ && !finished_;
#endif
}
//---------------------------------------------------------------------------
inline const bool & Thread::started() const
{
  return started_;
}
//---------------------------------------------------------------------------
inline Thread & Thread::terminate()
{
  terminated_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & Thread::terminated() const
{
  return terminated_;
}
//---------------------------------------------------------------------------
inline const bool & Thread::finished() const
{
  return finished_;
}
//---------------------------------------------------------------------------
inline Thread & Thread::stackSize(uintptr_t newStackSize)
{
#if HAVE_PTHREAD_H
  if( newStackSize < PTHREAD_STACK_MIN ) newStackSize = PTHREAD_STACK_MIN;
#endif
  stackSize_ = newStackSize;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & Thread::stackSize() const
{
  return stackSize_;
}
//---------------------------------------------------------------------------
inline Array< Thread::Action> & Thread::beforeExecuteActions()
{
  return *reinterpret_cast< Array< Action> *>(beforeExecuteActions_);
}
//---------------------------------------------------------------------------
inline Array< Thread::Action> & Thread::afterExecuteActions()
{
  return *reinterpret_cast< Array< Action> *>(afterExecuteActions_);
}
//---------------------------------------------------------------------------
extern uint8_t currentThreadPlaceHolder[];
inline ThreadLocalVariable< Thread> & currentThread()
{
  return *reinterpret_cast< ThreadLocalVariable< Thread> *>(currentThreadPlaceHolder);
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#if !HAVE_GETTID
inline uintptr_t gettid()
{
#if defined(__WIN32__) || defined(__WIN64__)
  return (uintptr_t) GetCurrentThreadId();
#elif HAVE_PTHREAD_SELF
  return (uintptr_t) pthread_self();
#elif defined(__linux__) && defined(__i386__)
  pid_t ret;
  __asm__ __volatile__ ( "int $0x80" : "=a" (ret) : "0" (224) );
  if( ret < 0 ) ret = -1;
  return ret;
#else
#error gettid not implemented
#endif
}
#endif
//---------------------------------------------------------------------------
#endif /* _thread_H_ */
//---------------------------------------------------------------------------
