/*-
 * Copyright 2006 Guram Dukashvili
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
namespace msmail {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Server::~Server()
{
}
//------------------------------------------------------------------------------
Server::Server(const ConfigSP config) : config_(config)
{
}
//------------------------------------------------------------------------------
void Server::open()
{
//  maxThreads(1);
  ksock::Server::open();
  for( uintptr_t i = config_->value("spool_fibers",20); i > 0; i-- )
    attachFiber(new SpoolWalker(*this));
}
//------------------------------------------------------------------------------
void Server::close()
{
  ksock::Server::close();
}
//------------------------------------------------------------------------------
Fiber * Server::newFiber()
{
  return new ServerFiber(*this);
}
//------------------------------------------------------------------------------
Server::Data & Server::data(ServerType type)
{
  switch( type ){
    case stNode       : return nodeData_;
    case stStandalone : return standaloneData_;
  }
  throw ExceptionSP(new Exception(EINVAL,__PRETTY_FUNCTION__));
}
//------------------------------------------------------------------------------
utf8::String Server::spoolDir() const
{
  utf8::String spool(excludeTrailingPathDelimiter(
    config_->value("spool",getExecutablePath() + "spool")
  ));
  createDirectoryAsync(spool);
  return includeTrailingPathDelimiter(spool);
}
//------------------------------------------------------------------------------
utf8::String Server::mailDir() const
{
  utf8::String mail(excludeTrailingPathDelimiter(
    config_->value("spool",getExecutablePath() + "spool")) + "mail");
  createDirectoryAsync(mail);
  return includeTrailingPathDelimiter(mail);
}
//------------------------------------------------------------------------------
utf8::String Server::mqueueDir() const
{
  utf8::String mqueue(excludeTrailingPathDelimiter(
    config_->value("spool",getExecutablePath() + "spool")) + "mqueue");
  createDirectoryAsync(mqueue);
  return includeTrailingPathDelimiter(mqueue);
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Server::Data::~Data()
{
}
//------------------------------------------------------------------------------
Server::Data::Data()
{
}
//------------------------------------------------------------------------------
} // namespace msmail
//------------------------------------------------------------------------------
