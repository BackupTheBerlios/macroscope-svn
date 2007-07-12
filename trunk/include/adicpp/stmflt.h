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
#define _NO_EXCEPTIONS 1
#include <adicpp/lzma/7zip/Compress/LZMA/LZMAEncoder.h>
#include <adicpp/lzma/7zip/Compress/LZMA/LZMADecoder.h>
#undef _NO_EXCEPTIONS
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class StreamCompressionFilter {
  public:
    virtual ~StreamCompressionFilter();
    StreamCompressionFilter();

    virtual StreamCompressionFilter & initializeCompression() = 0;
    virtual StreamCompressionFilter & compress() = 0;
    virtual StreamCompressionFilter & finishCompression() = 0;
    virtual StreamCompressionFilter & initializeDecompression() = 0;
    virtual StreamCompressionFilter & decompress() = 0;
    virtual StreamCompressionFilter & finishDecompression() = 0;
  protected:
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
class LZMAFilter : public StreamCompressionFilter, private BaseServer
{
  public:
    virtual ~LZMAFilter();
    LZMAFilter();

    StreamCompressionFilter & initializeCompression();
    StreamCompressionFilter & compress();
    StreamCompressionFilter & finishCompression();
    StreamCompressionFilter & initializeDecompression();
    StreamCompressionFilter & decompress();
    StreamCompressionFilter & finishDecompression();
  protected:
    virtual intptr_t encoderRead(void * buf,uintptr_t size) = 0;
    virtual intptr_t encoderWrite(const void * buf,uintptr_t size) = 0;
    class Encoder :
      public Fiber,
      public NCompress::NLZMA::CEncoder,
      public ISequentialInStream,
      public ISequentialOutStream
    {
      public:
        Encoder() { destroy_ = false; }

        STDMETHOD(QueryInterface)(REFIID,void **) { return E_FAIL; }
        ULONG STDMETHODCALLTYPE AddRef(void) { return 0; }
        ULONG STDMETHODCALLTYPE Release(void) { return 0; }

        STDMETHOD(Read)(void * data,UInt32 size,UInt32 * processedSize);
        STDMETHOD(Write)(const void *data,UInt32 size,UInt32 * processedSize);

        Semaphore sem_;
        LZMAFilter * filter_;
        Fiber * guest_;
        int32_t err_;
        bool flush_;

        void fiberExecute();
    };
    friend class Encoder;
    AutoPtr<Encoder> encoder_;

    virtual intptr_t decoderRead(void * buf,uintptr_t size) = 0;
    virtual intptr_t decoderWrite(const void * buf,uintptr_t size) = 0;
    class Decoder :
      public Fiber,
      public NCompress::NLZMA::CDecoder,
      public ISequentialInStream,
      public ISequentialOutStream
    {
      public:
        Decoder() { destroy_ = false; }

        STDMETHOD(QueryInterface)(REFIID,void **) { return E_FAIL; }
        ULONG STDMETHODCALLTYPE AddRef(void) { return 0; }
        ULONG STDMETHODCALLTYPE Release(void) { return 0; }

        STDMETHOD(Read)(void * data,UInt32 size,UInt32 * processedSize);
        STDMETHOD(Write)(const void *data,UInt32 size,UInt32 * processedSize);

        Semaphore sem_;
        LZMAFilter * filter_;
        Fiber * guest_;
        int32_t err_;
        bool flush_;

        void fiberExecute();
    };
    friend class Decoder;
    AutoPtr<Decoder> decoder_;

    LZMAFilter & dropEncoder();
    LZMAFilter & dropDecoder();
  private:
    LZMAFilter(const LZMAFilter &);
    void operator = (const LZMAFilter &);

    Fiber * newFiber() { return NULL; }
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LZMAFileFilter : public LZMAFilter {
  public:
    StreamCompressionFilter & compressFile(const utf8::String & srcFileName,const utf8::String & dstFileName);
    StreamCompressionFilter & decompressFile(const utf8::String & srcFileName,const utf8::String & dstFileName);
  protected:
    AsyncFile srcFile_;
    AsyncFile dstFile_;

    intptr_t encoderRead(void * buf,uintptr_t size);
    intptr_t encoderWrite(const void * buf,uintptr_t size);
    intptr_t decoderRead(void * buf,uintptr_t size);
    intptr_t decoderWrite(const void * buf,uintptr_t size);
  private:
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
