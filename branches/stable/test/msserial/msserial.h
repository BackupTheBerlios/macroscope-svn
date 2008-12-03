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
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class SerialPortFiber;
class MSSerialService;
//------------------------------------------------------------------------------
class SerialPortControl {
  friend class SerialPortFiber;
  friend class MSSerialService;
  public:
    virtual ~SerialPortControl();
    SerialPortControl();
  protected:
  private:
    SerialPortControl(const SerialPortControl &);
    void operator = (const SerialPortControl &);

    Semaphore semaphore_;
    AsyncFile device_;
    SerialPortFiber * control_;
    SerialPortFiber * reader_;
    SerialPortFiber * writer_;

    void open(const utf8::String & device);
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class SerialPortFiber : public ksock::ServerFiber {
  friend class MSSerialService;
  public:
    virtual ~SerialPortFiber();
    SerialPortFiber(MSSerialService * service = NULL,SerialPortControl * control = NULL);
  protected:
  private:
    SerialPortFiber(const SerialPortFiber &);
    void operator = (const SerialPortFiber &);

    MSSerialService * service_;
    SerialPortControl * control_;

    void removeControl();
    void main();
};
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
class MSSerialService : public Service, public BaseServer {
  friend class SerialPortFiber;
  public:
    virtual ~MSSerialService() {}
    MSSerialService();

    void start();
    void stop();
    bool active();
  protected:
  private:
    ConfigSP config_;
    FiberWriteLock serialPortsReadWriteLock_;
    Vector<SerialPortControl> serialPorts_;

    Fiber * newFiber();
};
//------------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
