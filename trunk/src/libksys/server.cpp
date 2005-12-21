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
namespace ksock {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void AcceptFiber::execute()
{
  assert( thread() != NULL );
  Server * server =
#ifndef NDEBUG
    dynamic_cast
#else
    static_cast
#endif
      <Server *>(thread()->server());
  assert( server != NULL );
  try {
    while( !terminated_ ){
      ksys::AutoPtr<ServerFiber> fiber(
#ifndef NDEBUG
        dynamic_cast
#else
        static_cast
#endif
        <ServerFiber *>(server->afNewFiber())
      );
      assert( fiber.ptr() != NULL );
      accept(*fiber.ptr());
      fiber->setlinger();
      server->attachFiber(*fiber.ptr());
      fiber.ptr(NULL);
      server->sweepFibers();
    }
  }
  catch( ksys::ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( e->codes()[0] != ERROR_OPERATION_ABORTED + ksys::errorOffset ) throw;
#else
    if( e->codes()[0] != EINTR ) throw;
#endif
  }
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void ServerFiber::execute()
{
  main();
  flush();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Server & Server::open()
{
  assert( acceptFiber_ == NULL );
  ksys::AutoPtr<AcceptFiber> fiber(new AcceptFiber);
  fiber->open().setlinger();
  for( intptr_t i = bindAddrs_.count() - 1; i >= 0; i-- )
    fiber->bind(bindAddrs_[i]);
  fiber->listen();
  attachFiber(*fiber.ptr());
  acceptFiber_ = fiber.ptr(NULL);
  return *this;
}
//------------------------------------------------------------------------------
Server & Server::close()
{
  if( acceptFiber_ != NULL ){
    acceptFiber_->shutdown();
    acceptFiber_->AsyncSocket::close();
    closeServer();
    acceptFiber_ = NULL;
  }
  return *this;
}
//------------------------------------------------------------------------------
void Server::attachFiber(ksys::BaseFiber & fiber)
{
  BaseServer::attachFiber(fiber);
  AsyncSocket * socket =
#ifndef NDEBUG
    dynamic_cast
#else
    static_cast
#endif
      <AsyncSocket *>(&fiber);
  assert( socket != NULL );
  socket->attach(fiber,*fiber.thread());
}
//------------------------------------------------------------------------------
} // namespace ksock
//------------------------------------------------------------------------------
