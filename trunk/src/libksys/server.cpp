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
#include <adicpp/ksys.h>
//------------------------------------------------------------------------------
namespace ksock {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
AcceptFiber::~AcceptFiber()
{
}
//---------------------------------------------------------------------------
AcceptFiber::AcceptFiber()
{
}
//---------------------------------------------------------------------------
void AcceptFiber::fiberExecute()
{
  Server * server = dynamic_cast<Server *>(ksys::Fiber::thread()->server());
  assert( server != NULL );
  try {
    for(;;){
      ServerFiber * serverFiber = dynamic_cast<ServerFiber *>(server->newFiber());
      assert( serverFiber != NULL );
      ksys::AutoPtr<Fiber> fiber(serverFiber);
      accept(*serverFiber);
      server->attachFiber(fiber);
    }
  }
  catch( ksys::ExceptionSP & e ){
#if defined(__WIN32__) || defined(__WIN64__)
    if( e->code() != ERROR_OPERATION_ABORTED + ksys::errorOffset &&
        e->code() != WSAENOTSOCK + ksys::errorOffset) throw;
#else
    if( !e->searchCode(ECANCELED) ) throw;
#endif
  }
}
//------------------------------------------------------------------------------
void AcceptFiber::fiberBreakExecution()
{
  event_.cancelAsyncEvent();
  shutdown();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void ServerFiber::fiberExecute()
{
  try {
    open();
    main();
    flush();
  }
  catch( ksys::ExceptionSP & ){
    close();
    throw;
  }
  close();
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Server::~Server()
{
}
//------------------------------------------------------------------------------
Server::Server() : acceptFiber_(NULL)
{
}
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
void Server::open()
{
  assert( acceptFiber_ == NULL );
  AcceptFiber * acceptFiber = newObject<AcceptFiber>();
  ksys::AutoPtr<ksys::Fiber> fiber(acceptFiber);
  acceptFiber->open();
  for( intptr_t i = bindAddrs_.count() - 1; i >= 0; i-- )
    acceptFiber->bind(bindAddrs_[i]);
  acceptFiber->listen();
  attachFiber(fiber);
  //acceptFiber->thread()->maxFibersPerThread(1);
  acceptFiber_ = acceptFiber;
}
//------------------------------------------------------------------------------
void Server::close()
{
  if( acceptFiber_ != NULL ){
    closeServer();
    acceptFiber_ = NULL;
  }
}
//------------------------------------------------------------------------------
} // namespace ksock
//------------------------------------------------------------------------------
