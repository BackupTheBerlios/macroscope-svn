/*-
 * Copyright 2006-2007 Guram Dukashvili
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
#include <adicpp/adicpp.h>
#include "msmail.h"
//------------------------------------------------------------------------------
#define _VERSION_C_AS_HEADER_
#include "version.c"
#undef _VERSION_C_AS_HEADER_
//------------------------------------------------------------------------------
namespace msmail {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Service::Service() :
  msmailConfig_(newObject<InterlockedConfig<FiberInterlockedMutex> >()),
  msmail_(msmailConfig_)
{
#if defined(__WIN32__) || defined(__WIN64__)
  serviceType_ = SERVICE_WIN32_OWN_PROCESS;
  startType_ = SERVICE_AUTO_START;
  errorControl_ = SERVICE_ERROR_IGNORE;
  binaryPathName_ = getExecutableName();
  serviceStatus_.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  serviceStatus_.dwWaitHint = 600000; // give me 600 seconds for start or stop 
#endif
}
//------------------------------------------------------------------------------
void Service::install()
{
  msmailConfig()->parse().override();
  serviceName_ = msmailConfig_->text("service_name","msmail");
  displayName_ = msmailConfig_->text("service_display_name","Macroscope MAIL Service");
#if defined(__WIN32__) || defined(__WIN64__)
  utf8::String startType(msmailConfig_->text("service_start_type","auto"));
  if( startType.strcasecmp("auto") == 0 ){
    startType_ = SERVICE_AUTO_START;
  }
  else if( startType.strcasecmp("manual") == 0 ){
    startType_ = SERVICE_DEMAND_START;
  }
#endif
}
//------------------------------------------------------------------------------
void Service::uninstall()
{
  install();
}
//------------------------------------------------------------------------------
void Service::start()
{
  install();
  Array<ksock::SockAddr> addrs;
  ksock::SockAddr::resolveNameForBind(msmailConfig_->text("bind"),addrs,defaultPort);
  for( uintptr_t i = 0; i < addrs.count(); i++ ) msmail_.addBind(addrs[i]);
  //msmail_.fiberStackSize(1024 * 1024);
  msmail_.open();
  stdErr.debug(0,utf8::String::Stream() << msmail_version.gnu_ << " started (" << serviceName_ << ")\n");
}
//------------------------------------------------------------------------------
void Service::stop()
{
  msmail_.close();
  stdErr.debug(0,utf8::String::Stream() << msmail_version.gnu_ << " stopped (" << serviceName_ << ")\n");
}
//------------------------------------------------------------------------------
bool Service::active()
{
  return msmail_.active();
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
