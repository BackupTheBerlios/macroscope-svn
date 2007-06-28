/*-
 * Copyright 2007 Guram Dukashvili
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
#ifndef stmfltH
#define stmfltH
//---------------------------------------------------------------------------
#include <adicpp/lzma/7zip/Compress/LZMA/LZMAEncoder.h>
#include <adicpp/lzma/7zip/Compress/LZMA/LZMADecoder.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class StreamCompressionFilter {
  public:
    virtual ~StreamCompressionFilter();
    StreamCompressionFilter();

    virtual StreamCompressionFilter & compress(const void * buf,uintptr_t count) = 0;
    virtual StreamCompressionFilter & decompress(void * buf,uintptr_t count) = 0;
    virtual StreamCompressionFilter & flush() = 0;
  protected:
    virtual StreamCompressionFilter & readBuffer(void * buf,uintptr_t count) = 0;
    virtual StreamCompressionFilter & writeBuffer(const void * buf,uintptr_t count) = 0;
    virtual StreamCompressionFilter & writeCompressedBuffer(const void * buf,uintptr_t count) = 0;
  private:
    StreamCompressionFilter(const StreamCompressionFilter &);
    void operator = (const StreamCompressionFilter &);
};
//---------------------------------------------------------------------------
inline StreamCompressionFilter::~StreamCompressionFilter()
{
}
//---------------------------------------------------------------------------
inline StreamCompressionFilter::StreamCompressionFilter()
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LZMAFilter :
  public StreamCompressionFilter,
  private BaseServer,
  private Fiber,
  private NCompress::NLZMA::CEncoder,
  private NCompress::NLZMA::CDecoder,
  private ISequentialInStream,
  private ISequentialOutStream
{
  public:
    virtual ~LZMAFilter();
    LZMAFilter();

    StreamCompressionFilter & compress(const void * buf,uintptr_t count);
    StreamCompressionFilter & decompress(void * buf,uintptr_t count);
    StreamCompressionFilter & flush();
  protected:
    StreamCompressionFilter & readBuffer(void * buf,uintptr_t count);
    StreamCompressionFilter & writeBuffer(const void * buf,uintptr_t count);
    StreamCompressionFilter & writeCompressedBuffer(const void * buf,uintptr_t count);
  private:
    LZMAFilter(const LZMAFilter &);
    void operator = (const LZMAFilter &);

    STDMETHOD(QueryInterface)(REFIID,void **) { return E_FAIL; }
    ULONG STDMETHODCALLTYPE AddRef(void) { return 0; }
    ULONG STDMETHODCALLTYPE Release(void) { return 0; }

    Fiber * guest_;
    const void * rData_;
    uintptr_t rCount_;
    void * wData_;
    uintptr_t wCount_;
    bool flush_;

    Semaphore threadSem_;

    Fiber * newFiber() { return NULL; }
    void fiberExecute();

    STDMETHOD(Read)(void * data,UInt32 size,UInt32 * processedSize);
    STDMETHOD(Write)(const void *data,UInt32 size,UInt32 * processedSize);
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class StreamCryptor {
  public:
    virtual ~StreamCryptor();
    StreamCryptor();
  protected:
    virtual int read() = 0;
    virtual int write() = 0;
    virtual int flush() = 0;
  private:
    StreamCryptor(const StreamCryptor &){}
    void operator = (const StreamCryptor &){}
};
//---------------------------------------------------------------------------
inline StreamCryptor::~StreamCryptor()
{
}
//---------------------------------------------------------------------------
inline StreamCryptor::StreamCryptor()
{
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif /* stmfltH */
//---------------------------------------------------------------------------
