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
#ifndef _service_H_
#define _service_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
enum ServiceStatus {
  svcReady,
  svcStarting,
  svcStarted,
  svcSuspending,
  svcSuspended,
  svcResuming,
  svcResumed,
  svcStoping,
  svcStoped
};
//---------------------------------------------------------------------------
class Service {
  friend class Services;
  public:
    virtual ~Service();
    Service();

#if defined(__WIN32__) || defined(__WIN64__)
    Service & serviceMain(uintptr_t n,DWORD dwArgc,LPWSTR * lpszArgv);
    Service & handler(uintptr_t n,DWORD fdwControl);
#endif
  protected:
#if defined(__WIN32__) || defined(__WIN64__)
    Array<utf8::String> argv_;
#else
    utf8::String serviceName_;
    utf8::String displayName_;
#endif
    ServiceStatus status_;

    virtual void start();
    virtual void stop();
    virtual void suspend();
    virtual void resume();
    virtual utf8::String status();
    virtual bool active();
#if defined(__WIN32__) || defined(__WIN64__)
    Semaphore semaphore_;
    utf8::String serviceName_;
    utf8::WideString serviceNameW_;
    utf8::String displayName_;
    DWORD serviceType_;
    DWORD startType_;
    DWORD errorControl_;
    DWORD tagId_;
    utf8::String binaryPathName_;
    utf8::String loadOrderGroup_;
    utf8::String dependencies_;
    utf8::String serviceStartName_;
    utf8::String password_;

    SERVICE_STATUS_HANDLE statusHandle_;
    SERVICE_STATUS serviceStatus_;
  private:
    Service & install(SC_HANDLE hSCManager);
    Service & uninstall(SC_HANDLE hSCManager);
#endif
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Services
#if !defined(__WIN32__) && !defined(__WIN64__)
  : public Thread
#endif
{
  friend class Service;
  public:
    ~Services();
    Services(const utf8::String & name = utf8::String() /* not used under windows */);

    Services & clear();
    Services & add(Service * service);

    Services & install();
    Services & uninstall();
    Services & start(const utf8::String & serviceName);
    Services & stop(const utf8::String & serviceName);
    Services & suspend(const utf8::String & serviceName);
    Services & resume(const utf8::String & serviceName);
    Services & query(const utf8::String & serviceName);

    Services & stopServiceCtrlDispatcher();
    Services & startServiceCtrlDispatcher(bool daemonize = true,bool startServices = true);
#if defined(__WIN32__) || defined(__WIN64__)
    static Service * services_[];
#endif
  protected:
    Service & serviceByName(const utf8::String & serviceName);
  private:
#if defined(__WIN32__) || defined(__WIN64__)
    SC_HANDLE handle_;
    Array<SERVICE_TABLE_ENTRYW> serviceStartTable_;

    static VOID (WINAPI * const serviceMains_[])(DWORD,LPWSTR *);
    static VOID (WINAPI * const handlers_[])(DWORD);
#else
    Vector<Service> services_;
    utf8::String controlName_;
    AutoPtr<SharedMemoryQueue> control_;
    void execute();
    enum ServiceDispatcherCmd {
      svcStart, svcStop, svcSuspend, svcResume, svcQuery, svcStopDispatcher
    };
#endif
};
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
//---------------------------------------------------------------------------
template <uintptr_t N>
VOID WINAPI serviceMainTemplate(DWORD dwArgc,LPWSTR * lpszArgv)
{
  Services::services_[N]->serviceMain(N,dwArgc,lpszArgv);
}
//---------------------------------------------------------------------------
template <uintptr_t N>
VOID WINAPI handlerTemplate(DWORD fdwControl)
{
  Services::services_[N]->handler(N,fdwControl);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _service_H_ */
