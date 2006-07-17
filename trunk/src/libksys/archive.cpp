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
#include <adicpp/ksys.h>
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
const uint8_t Archive::magic_[16] = {
  0x08, 0xC2, 0x10, 0x35, 0x24, 0x17, 0x48, 0x99,
  0x87, 0x0E, 0xD9, 0x2F, 0xC2, 0x23, 0xAE, 0x8B
};
//------------------------------------------------------------------------------
Archive::~Archive()
{
}
//------------------------------------------------------------------------------
Archive::Archive()
{
  wBufSize(getpagesize() * 64).method(LZO1X_999).crc(ADLER32).level(9).optimize(true);
}
//------------------------------------------------------------------------------
Archive & Archive::clear()
{
  return *this;
}
//------------------------------------------------------------------------------
Archive & Archive::pack(const Vector<utf8::String> & fileList)
{
  LZO1X::active(false);
  if( password_.strlen() > 0 ){
    if( password_.strncasecmp("sha256:",7) == 0 ){
      uint8_t sha256[32];
      base64Decode(
        utf8::String::Iterator(password_) + 7,
        sha256,
        sizeof(sha256)
      );
      init(sha256);
    }
    else {
      init(password_.c_str(),password_.size());
    }
    SHA256Filter::active(true);
  }
  for( intptr_t i = fileList.count() - 1; i >= 0; i-- ){
    AsyncFile file(fileList[i]);
    file.readOnly(true).open();
/*    int64_t w = write(buf,len);
    if( w == 0 ){
      AutoPtr<uint8_t> cbuf;
      uint8_t * p;
      int32_t ll;
      compress(cbuf,p,ll);
      if( SHA256Filter::active() ) encrypt(p,ll);
      file.writeBuffer(p,ll);
      wBufPos(0);
    }*/
  }
  LZO1X::active(false);
  return *this;
}
//------------------------------------------------------------------------------
Archive & Archive::unpack(const utf8::String & path)
{
  return *this;
}
//------------------------------------------------------------------------------
Archive & Archive::list(Vector<utf8::String> & list)
{
  return *this;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
