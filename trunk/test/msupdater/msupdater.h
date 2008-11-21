/*-
 * Copyright 2006-2008 Guram Dukashvili
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
//------------------------------------------------------------------------------
#ifndef msupdaterH
#define msupdaterH
//------------------------------------------------------------------------------
using namespace ksys;
//------------------------------------------------------------------------------
class MSUpdaterService;
class MSUpdateFetcher;
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSUpdateSetuper : public Fiber {
  public:
    virtual ~MSUpdateSetuper();
    MSUpdateSetuper(MSUpdateFetcher * fetcher = NULL);
  protected:
    void fiberExecute();
  private:
    MSUpdateFetcher * fetcher_;
    void executeAction(const utf8::String & name,const ConfigSection & section);
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSUpdateFetcher : public Fiber {
  friend class MSUpdateSetuper;
  public:
    virtual ~MSUpdateFetcher();
    MSUpdateFetcher(MSUpdaterService * updater = NULL);
  protected:
    void fiberExecute();
  private:
    MSUpdaterService * updater_;
    Semaphore setupSem_;
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSUpdaterService : public Service, public BaseServer {
  friend class MSUpdateSetuper;
  friend class MSUpdateFetcher;
  public:
    virtual ~MSUpdaterService() {}
    MSUpdaterService();

    //void beforeDestruction() { BaseServer::beforeDestruction(); }

    static void genUpdatePackage(const utf8::String & setupConfigFile);
    void start();
    void stop();
    bool active();
  protected:
  private:
    ConfigSP config_;

    Fiber * newFiber(){ return NULL; }
};
//------------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
