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

    LogFile & open();
    LogFile & close();

    LogFile & fileName(const utf8::String & name);
    const utf8::String & fileName() const;

    LogFile & log(LogMessagePriority pri,const utf8::String::Stream & stream);
    LogFile & debug(uintptr_t level,const utf8::String::Stream & stream);

    bool isDebugLevelEnabled(uintptr_t level) const;
    LogFile & enableDebugLevel(uintptr_t level);
    LogFile & disableDebugLevel(uintptr_t level);
    LogFile & setDebugLevels(const utf8::String & levels);

    LogFile & rotationThreshold(uint64_t a);
    const uint64_t & rotationThreshold() const;
    LogFile & rotatedFileCount(uintptr_t a);
    const uintptr_t & rotatedFileCount() const;

    const char * const & priNick(LogMessagePriority filter) const;
  protected:
    static const char * const priNicks_[];
    AsyncFile file_;
    AsyncFile lockFile_;
    FiberInterlockedMutex mutex_;
    uintptr_t enabledLevels_;
    uint64_t rotationThreshold_;
    uintptr_t rotatedFileCount_;

    LogFile & internalLog(LogMessagePriority pri,uintptr_t level,const utf8::String::Stream & stream);
    void rotate(uint64_t size);
  private:
    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline LogFile & LogFile::log(LogMessagePriority pri,const utf8::String::Stream & stream)
{
  return internalLog(pri,~uintptr_t(0),stream);
}
//---------------------------------------------------------------------------
inline LogFile & LogFile::debug(uintptr_t level,const utf8::String::Stream & stream)
{
  return internalLog(lmDEBUG,level,stream);
}
//---------------------------------------------------------------------------
inline const utf8::String & LogFile::fileName() const
{
  return file_.fileName();
}
//---------------------------------------------------------------------------
inline const char * const & LogFile::priNick(LogMessagePriority filter) const
{
  return priNicks_[filter];
}
//---------------------------------------------------------------------------
inline bool LogFile::isDebugLevelEnabled(uintptr_t level) const
{
  return (enabledLevels_ & (uintptr_t(1) << (level & (sizeof(uintptr_t) * CHAR_BIT - 1)))) != 0;
}
//---------------------------------------------------------------------------
inline LogFile & LogFile::enableDebugLevel(uintptr_t level)
{
  if( level <= 9 ) enabledLevels_ |= uintptr_t(1) << level;
  return *this;
}
//---------------------------------------------------------------------------
inline LogFile & LogFile::disableDebugLevel(uintptr_t level)
{
  if( level <= 9 ) enabledLevels_ &= ~(uintptr_t(1) << level);
  return *this;
}
//---------------------------------------------------------------------------
inline LogFile & LogFile::rotationThreshold(uint64_t a)
{
  rotationThreshold_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint64_t & LogFile::rotationThreshold() const
{
  return rotationThreshold_;
}
//---------------------------------------------------------------------------
inline LogFile & LogFile::rotatedFileCount(uintptr_t a)
{
  rotatedFileCount_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & LogFile::rotatedFileCount() const
{
  return rotatedFileCount_;
}
//---------------------------------------------------------------------------
extern char stdErr_[sizeof(LogFile) / sizeof(char)];
extern LogFile & stdErr;
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _Log_H_ */
//---------------------------------------------------------------------------
