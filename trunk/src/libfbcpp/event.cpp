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
#include <adicpp/fbcpp.h>
//---------------------------------------------------------------------------
namespace fbcpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EPB::~EPB()
{
}
//---------------------------------------------------------------------------
EPB::EPB(EventHandler & eventHandler)
  : eventHandler_(&eventHandler),
    eventCount_(0),
    eventBuffer_(NULL),
    resultBuffer_(NULL),
    resultBufferLen_(0),
    eventId_(0)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EventHandler::EventThread::EventThread(EventHandler & eventHandler)
  : eventHandler_(&eventHandler)
{
}
//---------------------------------------------------------------------------
void EventHandler::EventThread::threadExecute()
{
  try{
    while( !terminated_ ){
      eventHandler_->semaphore_.wait();
      eventHandler_->execute();
    }
    exitCode_ = 0;
  }
  catch( ksys::ExceptionSP & e ){
    exitCode_ = e->codes()[0];
  }
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EventHandler::EventHandler()
  : database_(NULL),
    thread_(NULL)
{
}
//---------------------------------------------------------------------------
EventHandler::~EventHandler()
{
  detach();
}
//---------------------------------------------------------------------------
void EventHandler::execute()
{
  for( intptr_t i = epbs_.count() - 1; i >= 0; i-- ){
    EPB & epb = epbs_[i];
    if( ksys::interlockedIncrement(epb.eventFlag_, 0) == 0 ) continue;
    if( epb.firstEvent_ ){
      epb.firstEvent_ = false;
      epb.eventFlag_ = 0;
      api.isc_event_counts((ISC_LONG *) epb.vector_, (short) epb.resultBufferLen_, epb.eventBuffer_, epb.resultBuffer_);
    }
    else{
      epb.eventFlag_ = 0;
      api.isc_event_counts((ISC_LONG *) epb.vector_, (short) epb.resultBufferLen_, epb.eventBuffer_, epb.resultBuffer_);
      for( intptr_t j = epb.eventCount_ - 1; j >= 0; j-- ){
        if( epb.vector_[j] > 0 ){
          epb.eventHandler_->eventHandler(epb.eventNames_[j], epb.vector_[j]);
        }
      }
    }
    ISC_STATUS_ARRAY status;
    if( api.isc_que_events(status, &epb.eventHandler_->database_->handle_, &epb.eventId_, (short) epb.resultBufferLen_, epb.eventBuffer_, (isc_callback) epb.eventHandler_->eventFunction, &epb) != 0 )
      break;
  }
}
//---------------------------------------------------------------------------
EventHandler & EventHandler::attach(Database & database)
{
  if( !attached() ){
    database.eventHandlers_.add(this, utf8::ptr2Str(this));
    database_ = &database;
  }
  return *this;
}
//---------------------------------------------------------------------------
EventHandler & EventHandler::detach()
{
  if( attached() ){
    cancel();
    database_->eventHandlers_.removeByObject(this);
    database_ = NULL;
  }
  return *this;
}
//---------------------------------------------------------------------------
EventHandler & EventHandler::queue()
{
  cancel();
  if( attached() ){
    for( intptr_t i = epbs_.count() - 1; i >= 0; i-- ){
      EPB & epb = epbs_[i];
      epb.resultBufferLen_ = (short) api.isc_event_block(&epb.eventBuffer_, &epb.resultBuffer_, (short) epb.eventCount_, epb.eventNames_[0].c_str(), epb.eventNames_[1].c_str(), epb.eventNames_[2].c_str(), epb.eventNames_[3].c_str(), epb.eventNames_[4].c_str(), epb.eventNames_[5].c_str(), epb.eventNames_[6].c_str(), epb.eventNames_[7].c_str(), epb.eventNames_[8].c_str(), epb.eventNames_[9].c_str(), epb.eventNames_[10].c_str(), epb.eventNames_[11].c_str(), epb.eventNames_[12].c_str(), epb.eventNames_[13].c_str(), epb.eventNames_[14].c_str());
      epb.eventFlag_ = 0;
      epb.firstEvent_ = true;
      ISC_STATUS_ARRAY  status;
      if( api.isc_que_events(status, &database_->handle_, &epb.eventId_, (short) epb.resultBufferLen_, epb.eventBuffer_, (isc_callback) eventFunction, &epb) != 0 )
        database_->exceptionHandler(newObject<EEventQueue>(status, __PRETTY_FUNCTION__));
    }
    try{
      thread_ = newObject<EventThread>(*this);
      thread_->resume();
    }
    catch( ksys::ExceptionSP & ){
      cancel();
      throw;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
EventHandler & EventHandler::cancel()
{
  if( attached() ){
    if( thread_ != NULL ){
      thread_->terminate();
      semaphore_.post();
      thread_->wait();
      delete thread_;
      thread_ = NULL;
    }
    for( intptr_t i = epbs_.count() - 1; i >= 0; i-- ){
      EPB &             epb = epbs_[i];
      ISC_STATUS_ARRAY  status;
      if( api.isc_cancel_events(status, &database_->handle_, &epb.eventId_) != 0 )
        database_->exceptionHandler(newObject<EEventCancel>(status, __PRETTY_FUNCTION__));
      epb.eventId_ = 0;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
EventHandler & EventHandler::clear()
{
  cancel().epbs_.clear();
  return *this;
}
//---------------------------------------------------------------------------
EventHandler & EventHandler::add(const utf8::String & eventName)
{
  EPB * pEPB;
  if( epbs_.count() == 0 || epbs_[epbs_.count() - 1].eventCount_ == 15 ){
    epbs_.add(pEPB = newObject<EPB>(*this));
  }
  else{
    pEPB = &epbs_[epbs_.count() - 1];
  }
  pEPB->eventNames_[pEPB->eventCount_++] = eventName;
  return *this;
}
//---------------------------------------------------------------------------
EventHandler & EventHandler::eventHandler(const utf8::String & eventName, uintptr_t eventCount)
{
  ksys::stdErr.log(
    ksys::lmNOTIFY,
    utf8::String::Stream() <<
    "unhandled event: " << eventName << eventCount
  );
  return *this;
}
//---------------------------------------------------------------------------
void EventHandler::eventFunction(EPB * epb, short length, char * updated)
{
  char *  result  = epb->resultBuffer_;
  while( length-- )
    *result++ = *updated++;
  ksys::interlockedIncrement(epb->eventFlag_, 1);
  epb->eventHandler_->semaphore_.post();
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------

