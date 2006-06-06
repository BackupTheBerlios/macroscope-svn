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
#ifndef _Log_H_
#define _Log_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
enum LogMessagePriority { 
  lmINFO,
  lmDEBUG,
  lmNOTIFY, 
  lmWARNING, 
  lmERROR, 
  lmFATAL, 
  lmPROFILER,
  lmDIRECT 
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncFile;
class FiberInterlockedMutex;
//---------------------------------------------------------------------------
class LogFile {
  friend void initialize();
  friend void cleanup();
  public:
    ~LogFile();
    LogFile();
    LogFile(const utf8::String & fileName);

    LogFile & open();
    LogFile & close();
    LogFile & fileName(const utf8::String & name);
    const utf8::String & fileName() const;

    /*LogFile &                   log(LogMessagePriority pri, const char * fmt, ...);
    LogFile &                   sysLog(LogMessagePriority pri, const char * fmt, ...);
    */
    LogFile & log(LogMessagePriority pri,const utf8::String::Stream & stream);

    const LogMessagePriority &  filter() const;
    LogFile & filter(LogMessagePriority filter);
    const char * const & priNick(LogMessagePriority filter) const;
  protected:
    static const char * const priNicks_[];
    FileHandleContainer file_;
    InterlockedMutex mutex_;
    AutoPtr<AsyncFile> afile_;
    AutoPtr<FiberInterlockedMutex> fmutex_;
    LogMessagePriority filter_;
  private:
    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline const utf8::String & LogFile::fileName() const
{
  return file_.fileName();
}
//---------------------------------------------------------------------------
inline const LogMessagePriority & LogFile::filter() const
{
  return filter_;
}
//---------------------------------------------------------------------------
inline LogFile & LogFile::filter(LogMessagePriority filter)
{
  filter_ = filter;
  return *this;
}
//---------------------------------------------------------------------------
inline const char * const & LogFile::priNick(LogMessagePriority filter) const
{
  return priNicks_[filter];
}
//---------------------------------------------------------------------------
extern char       stdErr_[sizeof(LogFile) / sizeof(char)];
extern LogFile &  stdErr;
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _Log_H_ */
