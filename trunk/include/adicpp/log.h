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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncFile;
class FiberInterlockedMutex;
//---------------------------------------------------------------------------
class LogFile : protected Thread {
  friend void initialize(int,char **);
  friend void cleanup();
  public:
    ~LogFile();
    LogFile();

    LogFile & open();
    LogFile & close();

    LogFile & fileName(const utf8::String & name);
    const utf8::String & fileName() const;

    LogFile & debug(uintptr_t level,const utf8::String::Stream & stream);

    bool debugLevel(uintptr_t level) const;
    LogFile & debugLevel(uintptr_t level,intptr_t value);
    LogFile & setDebugLevels(const utf8::String & levels);
    LogFile & setAllDebugLevels(intptr_t value);

    LogFile & rotationThreshold(uint64_t a);
    const uint64_t & rotationThreshold() const;
    LogFile & rotatedFileCount(uintptr_t a);
    const uintptr_t & rotatedFileCount() const;
    LogFile & codePage(uintptr_t a);
    const uintptr_t & codePage() const;
    LogFile & bufferDataTTA(uint64_t a);
    const uint64_t & bufferDataTTA() const;

    LogFile & flush(bool wait = false);
  protected:
    static const char * const priNicks_[];
    utf8::String file_;
    FiberInterlockedMutex mutex_;
    InterlockedMutex threadMutex_;
    AutoPtr<char,AutoPtrMemoryDestructor> buffer_;
    uintptr_t bufferPos_;
    uintptr_t bufferSize_;
    uint64_t bufferDataTTA_; // data time to accumulation
    uint64_t lastFlushTime_;
    Semaphore bufferSemaphore_;

    uint8_t enabledLevels_[32];
    uint64_t rotationThreshold_;
    uintptr_t rotatedFileCount_;
    uintptr_t codePage_;

    LogFile & internalLog(uintptr_t level,const utf8::String::Stream & stream);
    void rotate(AsyncFile & file);
    void threadExecute();
    void threadBeforeWait();

    AsyncFile threadFile_;
  private:
    static void initialize();
    static void cleanup();
};
//---------------------------------------------------------------------------
inline LogFile & LogFile::bufferDataTTA(uint64_t a)
{
  bufferDataTTA_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint64_t & LogFile::bufferDataTTA() const
{
  return bufferDataTTA_;
}
//---------------------------------------------------------------------------
inline LogFile & LogFile::debug(uintptr_t level,const utf8::String::Stream & stream)
{
  return internalLog(level,stream);
}
//---------------------------------------------------------------------------
inline const utf8::String & LogFile::fileName() const
{
  return file_;
}
//---------------------------------------------------------------------------
inline bool LogFile::debugLevel(uintptr_t level) const
{
  return bit(enabledLevels_,level & (sizeof(enabledLevels_) * 8u - 1)) != 0;
}
//---------------------------------------------------------------------------
inline LogFile & LogFile::debugLevel(uintptr_t level,intptr_t value)
{
  setBitValue(enabledLevels_,level & (sizeof(enabledLevels_) * 8u - 1),value > 0);
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
inline LogFile & LogFile::codePage(uintptr_t a)
{
  codePage_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & LogFile::codePage() const
{
  return codePage_;
}
//---------------------------------------------------------------------------
extern char stdErr_[sizeof(LogFile) / sizeof(char)];
extern LogFile & stdErr;
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _Log_H_ */
//---------------------------------------------------------------------------
