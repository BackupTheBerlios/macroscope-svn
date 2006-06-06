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
#ifndef _fbevent_H_
#define _fbevent_H_
//---------------------------------------------------------------------------
namespace fbcpp {

//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EPB {
    friend class EventHandler;
  public:
    ~EPB();
    EPB();
    EPB(EventHandler & eventHandler);
  protected:
  private:
    EventHandler *    eventHandler_;
    utf8::String      eventNames_[15];
    uintptr_t         eventCount_;
    char *            eventBuffer_;
    char *            resultBuffer_;
    uintptr_t         resultBufferLen_;
    ISC_LONG          eventId_;
    ISC_STATUS_ARRAY  vector_;
    int32_t           eventFlag_;
    bool              firstEvent_;
};
//---------------------------------------------------------------------------
inline EPB::EPB()
  : eventHandler_(NULL)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EventHandler {
  private:
    class EventThread : public ksys::Thread {
      private:
        EventHandler * eventHandler_;
        EventThread(const EventThread &){}
        void operator = (const EventThread &){}
      protected:
        void threadExecute();
      public:
        EventThread(EventHandler & eventHandler);
    };
    friend class EventThread;
  public:
    virtual ~EventHandler();
    EventHandler();

    EventHandler &          attach(Database & database);
    EventHandler &          detach();

    EventHandler &          queue();
    EventHandler &          cancel();

    EventHandler &          checkEvent();

    EventHandler &          clear();
    EventHandler &          add(const utf8::String & eventName);
    // properties
    bool                    attached();
  protected:
    static void eventFunction(EPB * epb, short length, char * updated);
    Database * database_;
    EventThread * thread_;
    ksys::Vector<EPB>  epbs_;
    ksys::Semaphore semaphore_;

    void execute();
    virtual EventHandler & eventHandler(const utf8::String & eventName, uintptr_t eventCount);
  private:
};
//---------------------------------------------------------------------------
inline bool EventHandler::attached()
{
  return database_ != NULL;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
#endif /* _fbdb_H_ */
