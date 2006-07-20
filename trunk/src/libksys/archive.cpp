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
Archive & Archive::flush(AsyncFile & archive)
{
  uint8_t * p;
  int32_t ll;
  AutoPtr<uint8_t> cbuf;
  compress(cbuf,p,ll);
  if( SHA256Filter::active() ) encrypt(p,ll);
  archive.writeBuffer(p,ll);
  wBufPos(0);
  return *this;
}
//------------------------------------------------------------------------------
Archive & Archive::writeBuffer(const void * buffer,uint64_t len,AsyncFile & archive)
{
  while( len > 0 ){
    int64_t w = write(buffer,len);
    if( w == 0 ) flush(archive);
    buffer = (const uint8_t *) buffer + (uintptr_t) w;
    len -= (uintptr_t) w;
  }
  return *this;
}
//------------------------------------------------------------------------------
Archive & Archive::readBuffer(void * buffer,uint64_t len,AsyncFile & archive)
{
  while( len > 0 ){
    int64_t r = read(buffer,len);
    if( r == 0 ){
      int32_t cps; // compressed packet size
      archive.readBuffer(&cps,sizeof(cps));
      if( SHA256Filter::active() ) decrypt(&cps,sizeof(cps));
      if( cps < 0 ){
        rBufSize(-cps);
        archive.readBuffer(rBuf(),-cps);
        if( SHA256Filter::active() ) decrypt(rBuf(),-cps);
      }
      else if( cps >= 0 ){
        AutoPtr<uint8_t> buf;
        buf.alloc(cps);
        archive.readBuffer(buf.ptr() + sizeof(int32_t),cps);
        *(int32_t *) buf.ptr() = cps;
        if( SHA256Filter::active() ) decrypt(buf.ptr() + sizeof(cps),cps - sizeof(cps));
        decompress(buf);
      }
      rBufPos(0);
    }
    buffer = (uint8_t *) buffer + (uintptr_t) r;
    len -= (uintptr_t) r;
  }
  return *this;
}
//------------------------------------------------------------------------------
Archive & Archive::activateFeatures()
{
  LZO1X::active(false);
  SHA256Filter::active(false);
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
  LZO1X::active(true);
  return *this;
}
//------------------------------------------------------------------------------
Archive & Archive::pack(const Vector<utf8::String> & fileList)
{
  activateFeatures();
  intptr_t i;
  AutoPtr<uint8_t> cbuf;
  AsyncFile archive(fileName_);
  archive.createIfNotExist(true).open();
  AutoFileWRLock<AsyncFile> lock(archive);
  if( archive.size() == 0 ) archive.writeBuffer(magic_,sizeof(magic_));
  for( i = fileList.count() - 1; i >= 0; i-- ){
    AsyncFile file(fileList[i]);
    file.readOnly(true).open();
    uint64_t sz = file.size(), l, la;
    l = fileList[i].size();
    writeBuffer(&l,sizeof(l),archive);
    writeBuffer(fileList[i].c_str(),l,archive);
    writeBuffer(&sz,sizeof(sz),archive);
    for( l = 0; l < sz; l += la ){
      la = l > wBufSpace() ? wBufSpace() : l;
      file.readBuffer(wBuf() + wBufShift() + wBufPos(),la);
      wBufPos(wBufPos() + (uintptr_t) la);
      flush(archive);
    }
  }
/*  for( i = fileList.count() - 1; i >= -1; i-- ){
    if( i < 0 || write(fileList[i].c_str(),fileList[i].size() + 1) == 0 ){
      compress(cbuf,p,ll);
      if( SHA256Filter::active() ) encrypt(p,ll);
      archive.writeBuffer(p,ll);
    }
  }*/
  LZO1X::active(false);
  SHA256Filter::active(false);
  return *this;
}
//------------------------------------------------------------------------------
Archive & Archive::unpack(const utf8::String & path,Vector<utf8::String> * pList)
{
  Vector<utf8::String> list;
  if( pList == NULL ) pList = &list;
  AsyncFile archive(fileName_);
  archive.open();
  AutoFileWRLock<AsyncFile> lock(archive);
  uint8_t magic[sizeof(magic_)];
  archive.readBuffer(magic,sizeof(magic));
  if( memcmp(magic,magic_,sizeof(magic)) != 0 )
    Exception::throwSP(
#if defined(__WIN32__) || defined(__WIN64__)
      ERROR_INVALID_DATA,
#else
      EINVAL,
#endif
      __PRETTY_FUNCTION__
    );
  for(;;){
    uint64_t sz, l, la;
    try {
      readBuffer(&l,sizeof(l),archive);
    }
    catch( ExceptionSP & e ){
      if( dynamic_cast<EFileEOF *>(e.ptr()) != NULL ) break;
      throw;
    }
    utf8::String fileName;
    fileName.resize((uintptr_t) l);
    readBuffer(fileName.c_str(),l,archive);
    pList->add(fileName);
    readBuffer(&sz,sizeof(sz),archive);
    AutoPtr<uint8_t> buf;
    buf.alloc(wBufSize());
    AsyncFile file(includeTrailingPathDelimiter(path) + fileName);
    file.createIfNotExist(true);
    try {
      file.open();
    }
    catch( ... ){
      createDirectory(getPathFromPathName(includeTrailingPathDelimiter(path) + fileName));
    }
    file.open().resize(0);
    for( l = 0; l < sz; l += la ){
      la = l > wBufSize() ? wBufSize() : l;
      readBuffer(buf,la,archive);
      file.writeBuffer(buf,la);
    }
  }  
  return *this;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
