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
//---------------------------------------------------------------------------
#ifndef archiveH
#define archiveH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Archive : protected LZO1X, protected SHA256Filter {
  public:
    ~Archive();
    Archive();

    Archive & clear();
    Archive & pack(const Vector<utf8::String> & fileList);
    Archive & unpack(const utf8::String & path,Vector<utf8::String> * pList = NULL);
    const utf8::String & fileName() const;
    Archive & fileName(const utf8::String & name);

    static const uint8_t magic_[16];
  protected:
    utf8::String fileName_;
    utf8::String password_;

    Archive & readBuffer(void * buffer,uint64_t len,AsyncFile & archive);
    Archive & writeBuffer(const void * buffer,uint64_t len,AsyncFile & archive);
    Archive & flush(AsyncFile & archive);
    Archive & activateFeatures();
  private:
};
//---------------------------------------------------------------------------
inline const utf8::String & Archive::fileName() const
{
  return fileName_;
}
//---------------------------------------------------------------------------
inline Archive & Archive::fileName(const utf8::String & name)
{
  fileName_ = name;
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
