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
#ifndef fetchH
#define fetchH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Fetcher {
  public:
    ~Fetcher();
    Fetcher();

    const utf8::String & url() const;
    Fetcher & url(const utf8::String & a);
    const utf8::String & proxy() const;
    Fetcher & proxy(const utf8::String & a);
    const utf8::String & localPath() const;
    Fetcher & localPath(const utf8::String & a);
    const bool & storeHostPath() const;
    Fetcher & storeHostPath(bool a);
    const bool & resume() const;
    Fetcher & resume(bool a);

    Fetcher & fetch(const utf8::String & localName = utf8::String());
    static utf8::String asctime2HttpTime(const char * tm);
    static time_t httpTime2asctime(const char * ht);
  protected:
    utf8::String url_;
    utf8::String proxy_;
    utf8::String localPath_;
    bool storeHostPath_;
    bool resume_;

    void parseUrl(
      const utf8::String & url,
      utf8::String & proto,
      utf8::String & user,
      utf8::String & password,
      utf8::String & host,
      utf8::String & path,
      utf8::String & name,
      utf8::String & port
    );
    static void setmtime(const utf8::String & fileName,uint64_t atime,uint64_t mtime);
  private:
};
//---------------------------------------------------------------------------
inline const utf8::String & Fetcher::url() const
{
  return url_;
}
//---------------------------------------------------------------------------
inline Fetcher & Fetcher::url(const utf8::String & a)
{
  url_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const utf8::String & Fetcher::proxy() const
{
  return proxy_;
}
//---------------------------------------------------------------------------
inline Fetcher & Fetcher::proxy(const utf8::String & a)
{
  proxy_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const utf8::String & Fetcher::localPath() const
{
  return localPath_;
}
//---------------------------------------------------------------------------
inline Fetcher & Fetcher::localPath(const utf8::String & a)
{
  localPath_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & Fetcher::storeHostPath() const
{
  return storeHostPath_;
}
//---------------------------------------------------------------------------
inline Fetcher & Fetcher::storeHostPath(bool a)
{
  storeHostPath_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & Fetcher::resume() const
{
  return resume_;
}
//---------------------------------------------------------------------------
inline Fetcher & Fetcher::resume(bool a)
{
  resume_ = a;
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
