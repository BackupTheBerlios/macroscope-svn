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
#ifndef adicppH
#define adicppH
//---------------------------------------------------------------------------
#include <adicpp/fbcpp.h>
#include <adicpp/mycpp.h>

#include <adicpp/adiexcpt.h>
#include <adicpp/adidb.h>
#include <adicpp/adist.h>
//---------------------------------------------------------------------------
namespace adicpp {
//---------------------------------------------------------------------------
class Initializer {
  friend class AutoInitializer;
  public:
    static void acquire();
    static void release();
  protected:
    static void initialize(int argc,char ** argv);
    static void cleanup();
  private:
    static int32_t mutex_;
    static int32_t initCount_;
};
//---------------------------------------------------------------------------
inline void Initializer::acquire()
{
  ksys::interlockedCompareExchangeAcquire(mutex_,-1,0);
}
//---------------------------------------------------------------------------
inline void Initializer::release()
{
  ksys::interlockedIncrement(mutex_,1);
}
//---------------------------------------------------------------------------
inline void Initializer::initialize(int argc,char ** argv)
{
  ksys::AutoLock<Initializer> lock(*(Initializer *) NULL);
  if( initCount_ == 0 ){
    ksys::initialize(argc,argv);
#if !DISABLE_FIREBIRD_INTERFACE
    fbcpp::initialize();
#endif
#if !DISABLE_MYSQL_INTERFACE
    mycpp::initialize();
#endif
  }
  initCount_++;
}
//---------------------------------------------------------------------------
inline void Initializer::cleanup()
{
  ksys::AutoLock<Initializer> lock(*(Initializer *) NULL);
  assert( initCount_ > 0 );
  if( initCount_ == 1 ){
#if !DISABLE_MYSQL_INTERFACE
    mycpp::cleanup();
#endif
#if !DISABLE_FIREBIRD_INTERFACE
    fbcpp::cleanup();
#endif
    ksys::cleanup();
  }
  initCount_--;
}
//---------------------------------------------------------------------------
class AutoInitializer {
  public:
    ~AutoInitializer(){ Initializer::cleanup(); }
    AutoInitializer(int argc = 0,char ** argv = NULL){ Initializer::initialize(argc,argv); }
  protected:
  private:
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif /* adicppH */
//---------------------------------------------------------------------------
