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
#ifndef _shsem_H_
#define _shsem_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#if HAVE_SYS_IPC_H && HAVE_SYS_SEM_H
#ifndef SEM_R
#define SEM_R 0
#endif
#ifndef SEM_A
#define SEM_A 0
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class SVSharedSemaphore {
    // System V shared semaphore
  public:
    ~SVSharedSemaphore();
    SVSharedSemaphore(const utf8::String & name = utf8::String(), uintptr_t flags = SEM_R | SEM_A, uintptr_t count = 1);

    static key_t        getKey(const utf8::String & name);
    SVSharedSemaphore & post(uintptr_t sem = 0);
    SVSharedSemaphore & wait(uintptr_t sem = 0);
    bool                tryWait(uintptr_t sem = 0);

    const bool &        creator() const;
  protected:
    int   id_;
    int   count_;
    bool  creator_;
  private:
    SVSharedSemaphore(const SVSharedSemaphore &){}
    void operator =(const SVSharedSemaphore &){ }
};
//---------------------------------------------------------------------------
inline const bool & SVSharedSemaphore::creator() const
{
  return creator_;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__) &&\
    (\
      (!HAVE_SEMAPHORE_H && HAVE_SYS_IPC_H && HAVE_SYS_SEM_H) ||\
      (HAVE_SEMAPHORE_H && HAVE_SEM_OPEN && HAVE_SEM_OPEN_ENOSYS)\
    )
//#define USE_SV_SEMAPHORES 1
#error POSIX semaphores not implemented, see README for solution
#endif
//---------------------------------------------------------------------------
class Semaphore
#if USE_SV_SEMAPHORES
 : public SVSharedSemaphore
#endif
{
  public:
    ~Semaphore();
    Semaphore();

#if !USE_SV_SEMAPHORES
    Semaphore & post();
    Semaphore & wait();
    bool tryWait();
#endif
    bool timedWait(uint64_t timeout,bool noThrow = true);
  protected:
#if USE_SV_SEMAPHORES
#elif HAVE_SEMAPHORE_H
    sem_t   handle_;
    static uint8_t handleNull_[sizeof(sem_t)];
#elif defined(__WIN32__) || defined(__WIN64__)
    HANDLE  handle_;
#else
#error you system not have semaphores API
#endif
  private:
    Semaphore(const Semaphore &){}
    void operator =(const Semaphore &) {}
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class SharedSemaphore
#if USE_SV_SEMAPHORES
 : public SVSharedSemaphore
#else
// : public Semaphore
#endif
{
  public:
    ~SharedSemaphore();
    SharedSemaphore(const utf8::String & name = utf8::String(),
#if USE_SV_SEMAPHORES
      uintptr_t mode = SEM_R | SEM_A
#elif HAVE_SEMAPHORE_H
                        uintptr_t mode = S_IWUSR
#elif defined(__WIN32__) || defined(__WIN64__)
                        uintptr_t mode = SEMAPHORE_ALL_ACCESS
#endif
                        );
    static utf8::String genName(const utf8::String & name);
    static void         unlink(const utf8::String & name);
#if !USE_SV_SEMAPHORES
    const bool &        creator() const;
#endif
#if !USE_SV_SEMAPHORES
    SharedSemaphore & post();
    SharedSemaphore & wait();
    bool tryWait();
#endif
    bool timedWait(uint64_t timeout,bool noThrow = true);
  protected:
#if USE_SV_SEMAPHORES
#elif HAVE_SEMAPHORE_H
    sem_t   * pHandle_;
    utf8::AnsiString  name_;
    bool              creator_;
#elif defined(__WIN32__) || defined(__WIN64__)
    HANDLE            handle_;
    bool              creator_;
#endif
  private:
    SharedSemaphore(const SharedSemaphore &)
#if USE_SV_SEMAPHORES
#elif HAVE_SEMAPHORE_H

      : name_(utf8::String().getANSIString())
#endif

    {
    }
    void operator =(const SharedSemaphore &);
};
//---------------------------------------------------------------------------
#if !USE_SV_SEMAPHORES
//---------------------------------------------------------------------------
inline const bool & SharedSemaphore::creator() const
{
  return creator_;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _shsem_H_ */
//---------------------------------------------------------------------------
