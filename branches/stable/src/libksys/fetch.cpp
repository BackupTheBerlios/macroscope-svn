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
Fetcher::~Fetcher()
{
}
//---------------------------------------------------------------------------
Fetcher::Fetcher() :
  storeHostPath_(false),
  resume_(false),
  resumed_(false),
  fetched_(false),
  modified_(false)
{
}
//---------------------------------------------------------------------------
void Fetcher::parseUrl(
  const utf8::String & url,
  utf8::String & proto,
  utf8::String & user,
  utf8::String & password,
  utf8::String & host,
  utf8::String & path,
  utf8::String & name,
  utf8::String & port)
{
  uintptr_t c = 0;
  utf8::String::Iterator i(url.strstr("://"));
  if( i.eos() ){
    i = url;
  }
  else {
    proto = utf8::String(url,i).upper();
    i += 3;
  }
  utf8::String::Iterator portI(i), si(i);
  while( !i.eos() && (c = i.getChar()) != '@' && c != '/' ){
    if( c == ':' ) portI = i;
    i.next();
  }
  if( !i.eos() && c == '@' ){
    utf8::String::Iterator j(i);
    do {
      j.prev();
      if( j.getChar() == ':' ){
        password = utf8::String(j + 1,i);
        utf8::String::Iterator k(j);
        do {
          k.prev();
          if( k.bos() || (c = k.getChar() == '/') ){
            user = utf8::String(k + (c == '/'),j);
            break;
          }
        } while( !k.bos() );
        break;
      }
      else if( j.bos() || (c = j.getChar() == '/') ){
        user = utf8::String(j + (c == '/'),i);
        break;
      }
    } while( !j.bos() );
    portI = i;
  }
  else {
    i = si;
  }
  while( !portI.eos() && (c = portI.getChar()) != ':' && c != '/' ) portI.next();
  host = utf8::String(i + (i.getChar() == '@'),portI);
  if( portI.getChar() == ':' ){
    utf8::String::Iterator j(portI + 1);
    while( !j.eos() && j.getChar() != '/' ) j.next();
    port = utf8::String(portI + 1,j);
    i = j;
  }
  else {
    i = portI;
  }
  if( i.getChar() == '/' ){
    utf8::String::Iterator j(i + 1);
    j.last();
    while( j > portI && j.getChar() != '/' ) j.prev();
    path = utf8::String(i,j + (j.getChar() == '/'));
    i = j;
  }
  name = utf8::String(i + 1);
}
//------------------------------------------------------------------------------
utf8::String Fetcher::asctime2HttpTime(const char * tm)
{
  return 
    utf8::String(tm,3) + ", " + 
    utf8::String(tm + 8,2) + " " + 
    utf8::String(tm + 4,3) + " " +
    utf8::String(tm + 20,4) + " " +
    utf8::String(tm + 11,8) + " GMT"
  ;
}
//------------------------------------------------------------------------------
time_t Fetcher::httpTime2asctime(const char * ht)
{
//Wed, 28 Jun 2006 12:01:13 GMT
  struct tm tma;
  memset(&tma,0,sizeof(tma));
  tma.tm_isdst = -1;
  tma.tm_mon = (int) str2Month(ht + 8);
  sscanf(ht + 5,"%u",(unsigned int *) &tma.tm_mday);
  sscanf(ht + 12,"%u",(unsigned int *) &tma.tm_year);
  sscanf(ht + 17,"%u:%u:%u",
    (unsigned int *) &tma.tm_hour,
    (unsigned int *) &tma.tm_min,
    (unsigned int *) &tma.tm_sec
  );
  tma.tm_year -= 1900;
  time_t t = timegm(&tma);
//  const char * tmp = asctime(gmtime(&t));
  return t;
}
//------------------------------------------------------------------------------
void Fetcher::setmtime(const utf8::String & fileName,uint64_t atime,uint64_t mtime)
{
  Stat st;
  if( stat(fileName,st) ) utime(fileName,atime,mtime + getgmtoffset() / 1000000u);
}
//------------------------------------------------------------------------------
Fetcher & Fetcher::fetch(const utf8::String & localName)
{
  utf8::String
    proto, user, proxyUser, password, proxyPassword, host, proxy,
    path, name, port, proxyPort
  ;
  parseUrl(proxy_,proto,proxyUser,proxyPassword,proxy,path,name,proxyPort);
  parseUrl(url_,proto,user,password,host,path,name,port);
  if( port.isNull() ){
    port = "80";
    if( proto.compare("FTP") == 0 ) port = "21";
  }
  ksock::SockAddr addr;
  if( !proxy.isNull() ){
    addr.resolveName(proxy,proxyPort);
  }
  else {
    addr.resolveName(host,port);
  }
  Stat st;
  AsyncFile file(
    (!localPath_.isNull() ? includeTrailingPathDelimiter(localPath_) : utf8::String()) +
    (storeHostPath_ ? path.ncompare("/",1) == 0 ? utf8::String(utf8::String::Iterator(path) + 1) : path : utf8::String()) +
    (!localName.isNull() ? localName : name)
  );
  localPathName_ = file.fileName();
  ksock::AsyncSocket socket;
  MemoryStream response;
  unsigned int code = 0;
  uint64_t resume = 0, cs = 0;
  time_t lmtime = 0, rmtime = 0;
  do {
    response.clear();
    socket.close();
    utf8::String::Stream request;
    if( !proxy.isNull() ){
      if( proxyPort.isNull() ) proxyPort = "3128";
      request << "GET " << url_ << " HTTP/1.0\r\n";
    }
    else {
      request << "GET " << path << name << " HTTP/1.0\r\n" <<
        "Host: " << host << ":" << port << "\r\n";
    }
    if( !proxyUser.isNull() || !proxyPassword.isNull() ){
      utf8::String auth64(proxyUser + ":" + proxyPassword);
      utf8::String base64;
      base64.resize(rfcBase64Encode((const uint8_t *) auth64.c_str(),auth64.size(),NULL,0));
      rfcBase64Encode(
        (const uint8_t *) auth64.c_str(),
        auth64.size(),
        (uint8_t *) base64.c_str(),
        base64.size()
      );
      request << "Proxy-authorization: Basic " << base64 << "\r\n"
        "Proxy-Connection: Keep-Alive\r\n"
      ;
    }
    if( !user.isNull() || !password.isNull() ){
      utf8::String auth64(user + ":" + password);
      utf8::String base64;
      base64.resize(rfcBase64Encode((const uint8_t *) auth64.c_str(),auth64.size(),NULL,0));
      rfcBase64Encode(
        (const uint8_t *) auth64.c_str(),
        auth64.size(),
        (uint8_t *) base64.c_str(),
        base64.size()
      );
      request << "Authorization: Basic " << base64 << "\r\n";
    }
    if( code == 416 && cs > (uint64_t) st.st_size ) remove(file.fileName());
    if( stat(file.fileName(),st) ){
      if( code != 304 ){
        lmtime = st.st_mtime - getgmtoffset() / 1000000u;
        const char * tm = asctime(gmtime(&lmtime));
        request << "If-Modified-Since: " << asctime2HttpTime(tm) << "\r\n";
      }
      if( resume_ ){
        if( code == 304 ){
          request << "Range: bytes=" << utf8::int2Str(resume = st.st_size) << "-\r\n";
        }
      }
    }
    request <<
      "Content-Encoding:\r\n"
      "Accept-Encoding:\r\n"
      "Accept-Charset: utf-8\r\n"
      "Accept: */*\r\n"
      "Last-Modified:\r\n"
//      "Content-Length:\r\n"
      "Connection: close\r\n"
      "\r\n"
    ;
    socket.connect(addr);
    socket.write(request.plane(),request.count());
    const char * crlf = NULL;
    for(;;){
      response.resize(response.pos() + getpagesize() / 8);
      int64_t r = 0;
      try {
        r = socket.sysRecv(
          (uint8_t *) response.raw() + response.pos(),
          response.count() - response.pos()
        );
      }
      catch( ExceptionSP & e ){
        if( e->code() != 0 + errorOffset ) throw;
      }
      if( r == 0 ) break;
      response.seek(response.pos() + (uintptr_t) r);
      if( response.pos() >= 5 && strncmp((const char *) response.raw(),"HTTP/",5) == 0 ){
        const char * httpCode = (const char *) response.raw() + 5;
        while( httpCode - (const char *) response.raw() < (intptr_t) response.pos() ){
          if( isspace(*httpCode) && httpCode + 1 - (const char *) response.raw() < (intptr_t) response.pos() && isdigit(httpCode[1]) ){
            httpCode++;
            const char * crlf2 = utf8::strnstr((const char *) response.raw(),"\r\n",response.pos());
            crlf = utf8::strnstr((const char *) response.raw(),"\r\n\r\n",response.pos());
            if( crlf2 != NULL && crlf != NULL ){
              sscanf(httpCode,"%u",&code);
              const char * contentLength = utf8::strnstr((const char *) response.raw(),"Content-Length: ",response.pos());
              if( contentLength != NULL ){
                const char * crlf4 = utf8::strnstr(contentLength,"\r\n",response.pos() - (contentLength - (const char *) response.raw()));
                if( crlf4 != NULL ){
                  sscanf(contentLength + 16,"%"PRIu64,&cs);
                  //cs -= crlf - (const char *) response.raw() + 4;
                }
              }
              const char * lastModified = utf8::strnstr((const char *) response.raw(),"Last-Modified: ",response.pos());
              if( lastModified != NULL ){
                const char * crlf3 = utf8::strnstr(lastModified,"\r\n",response.pos() - (lastModified - (const char *) response.raw()));
                if( crlf3 != NULL ) rmtime = httpTime2asctime(lastModified + 15);
              }
              if( code == 416 ){
                const char * contentRange = utf8::strnstr((const char *) response.raw(),"Content-Range: ",response.pos());
                if( contentRange != NULL ){
                  const char * crlf5 = utf8::strnstr(contentRange,"\r\n",response.pos() - (contentRange - (const char *) response.raw()));
                  if( crlf5 != NULL ){
                    const char * delim = utf8::strnstr(contentRange + 14,"/",crlf5 - contentRange - 14);
                    sscanf(delim != NULL ? delim + 1 : contentRange + 14,"%"PRIu64,&cs);
                  }
                }
              }
              if( code == 304 || code == 416 || code == 206 ) break;
              if( code != 200 )
                newObjectV1C2<Exception>(
#if defined(__WIN32__) || defined(__WIN64__)
                  ERROR_INVALID_DATA,
#else
                  EINVAL,
#endif
                  utf8::String(httpCode,crlf - httpCode)
                )->throwSP();
              response.resize(response.pos());
              response.seek(crlf - (const char *) response.raw() + 4);
            }
            break;
          }
          httpCode++;
        }
      }
      if( crlf != NULL ) break;
    }
    if( crlf == NULL )
      newObjectV1C2<Exception>(
#if defined(__WIN32__) || defined(__WIN64__)
        ERROR_INVALID_DATA,
#else
        EINVAL,
#endif
        utf8::String((const char *) response.raw(),response.pos())
      )->throwSP();
  } while( code == 304 || (code == 416 && cs > (uint64_t) st.st_size) );
  file.createIfNotExist(true).open();
  try {
    if( resume_ ){
      file.seek(resume);
    }
    else {
      file.resize(0);
    }
    file.writeBuffer((const uint8_t *) response.raw() + response.pos(),response.count() - response.pos());
    for(;;){
      int64_t r = 0;
      if( response.count() < (uintptr_t) getpagesize() ) response.resize(getpagesize());
      response.seek(0);
      while( response.pos() < response.count() ){
        r = 0;
        try {
          r = socket.sysRecv(
            (uint8_t *) response.raw() + response.pos(),
            response.count() - response.pos()
          );
        }
        catch( ExceptionSP & e ){
          if( e->code() != 0 + errorOffset ) throw;
        }
        if( r == 0 ) break;
        response.seek(response.pos() + (uintptr_t) r);
      }
      file.writeBuffer(response.raw(),response.pos());
      if( r == 0 && file.size() >= cs ) break;
    }
  }
  catch( ... ){
    file.close();
    setmtime(file.fileName(),st.st_atime,rmtime);
    throw;
  }
  file.close();
  setmtime(file.fileName(),st.st_atime,rmtime);
  resumed_ = resume_ && resume > 0 && file.size() < cs;
  fetched_ = !resume_ || (resume_ && resume == 0);
  modified_ = lmtime != rmtime;
  return *this;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
