/*-
 * Copyright 2005-2008 Guram Dukashvili
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
#ifndef _conf_H_
#define _conf_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
// conf file format
//
// sectionName1 "sectionParam1,sectionParam2" {
//   key1 = value1;
//   key2 = value1, value2, value3;
//   key3 = "value1", "value2", value3;
//   "subSectionName1" subSectionParam1, sectionParam2 {
//   }
//   key4 = "value1";
// }
//
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ConfigSection {
  friend class Config;
  public:
    virtual ~ConfigSection();
    ConfigSection(const utf8::String & name = utf8::String());

    const utf8::String & name() const;

    uintptr_t sectionCount() const;
    uintptr_t valueCount() const;

    bool isSection(const utf8::String & section, const HashedObjectListItem< utf8::String,ConfigSection> ** pItem = NULL) const;
    bool isValue(const utf8::String & key, const HashedObjectListItem< utf8::String,Mutant> ** pItem = NULL) const;
    const ConfigSection & section(const utf8::String & section) const;
    const ConfigSection & section(uintptr_t i) const;
    Mutant value(const utf8::String & key = utf8::String(), const Mutant & defValue = Mutant()) const;
    Mutant value(uintptr_t i,utf8::String * pKey = NULL) const;
    utf8::String text(const utf8::String & key = utf8::String(), const utf8::String & defText = utf8::String()) const;
    utf8::String text(uintptr_t i,utf8::String * pKey = NULL) const;

    bool isSectionByPath(const utf8::String & path) const;
    bool isValueByPath(const utf8::String & path) const;
    ConfigSection & sectionByPath(const utf8::String & path) const;
    utf8::String textByPath(const utf8::String & path,const utf8::String & defText = utf8::String()) const;
    Mutant valueByPath(const utf8::String & path,const Mutant & defValue = Mutant()) const;

    ConfigSection & saveSection(uintptr_t codePage,AsyncFile & file,bool recursive,uintptr_t level = 0);

    ConfigSection & setValue(const utf8::String & key,const Mutant & value) const;
    ConfigSection & setValue(uintptr_t i,const Mutant & value) const;

    ConfigSection & clear();
    ConfigSection & addSection(const ConfigSection & section);
  protected:
    Mutant & valueRefByPath(const utf8::String & path) const;
    Mutant & valueRef(const utf8::String & key) const;
  private:
    ConfigSection(const ConfigSection &);
    void operator = (const ConfigSection &);

    utf8::String name_;
    mutable HashedObjectList<utf8::String,ConfigSection> subSections_;
    mutable HashedObjectList<utf8::String,Mutant> values_;
    mutable Mutant nullValueRef_;
};
//---------------------------------------------------------------------------
inline ConfigSection::~ConfigSection()
{
}
//---------------------------------------------------------------------------
inline ConfigSection::ConfigSection(const utf8::String & name) : name_(name)
{
  subSections_.caseSensitive(false);
  values_.caseSensitive(false);
}
//---------------------------------------------------------------------------
inline const utf8::String & ConfigSection::name() const
{
  return name_;
}
//---------------------------------------------------------------------------
inline ConfigSection & ConfigSection::clear()
{
  subSections_.clear();
  values_.clear();
  return *this;
}
//---------------------------------------------------------------------------
inline uintptr_t ConfigSection::sectionCount() const
{
  return subSections_.count();
}
//---------------------------------------------------------------------------
inline uintptr_t ConfigSection::valueCount() const
{
  return values_.count();
}
//---------------------------------------------------------------------------
inline bool ConfigSection::isSection(const utf8::String & section, const HashedObjectListItem< utf8::String,ConfigSection> ** pItem) const
{
  HashedObjectListItem<utf8::String,ConfigSection> * item;
  item = subSections_.itemOfKey(section);
  if( pItem != NULL ) *pItem = item;
  return item != NULL;
}
//---------------------------------------------------------------------------
inline bool ConfigSection::isValue(const utf8::String & key, const HashedObjectListItem< utf8::String,Mutant> ** pItem) const
{
  HashedObjectListItem< utf8::String,Mutant> *  item  = values_.itemOfKey(key);
  if( pItem != NULL ) *pItem = item;
  return item != NULL;
}
//---------------------------------------------------------------------------
inline const ConfigSection & ConfigSection::section(const utf8::String & section) const
{
  HashedObjectListItem<utf8::String,ConfigSection> * item;
  item = subSections_.itemOfKey(section);
  if( item == NULL ){
    utf8::String s(section);
    subSections_.add(newObjectC1<ConfigSection>(s),section,&item);
  }
  return *item->object();
}
//---------------------------------------------------------------------------
inline const ConfigSection & ConfigSection::section(uintptr_t i) const
{
  HashedObjectListItem< utf8::String,ConfigSection> * item;
  item = subSections_.itemOfIndex(i);
  assert(item != NULL);
  return *item->object();
}
//---------------------------------------------------------------------------
inline Mutant & ConfigSection::valueRef(const utf8::String & key) const
{
  HashedObjectListItem< utf8::String,Mutant> *  item;
  if( (item = values_.itemOfKey(key)) == NULL ) return nullValueRef_;
  return *item->object();
}
//---------------------------------------------------------------------------
inline utf8::String ConfigSection::textByPath(const utf8::String & path,const utf8::String & defText) const
{
  return valueByPath(path,defText);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AsyncFile;
//---------------------------------------------------------------------------
class Config : public ConfigSection {
  friend void initialize(int,char **);
  friend void cleanup();
  friend class EConfig;
  friend class Section;
  public:
    ~Config();
    Config();

    Config &                    parse();
    Config &                    override(const Array< utf8::String> & oargv = Array< utf8::String>());

    const utf8::String &        fileName() const;
    Config &                    fileName(const utf8::String & name);

    const uintptr_t &           maxTryOpenCount() const;
    Config &                    maxTryOpenCount(uintptr_t n);
    const int64_t &             minTimeBetweenTryOpen() const;
    Config &                    minTimeBetweenTryOpen(int64_t min);
    const int64_t &             maxTimeBetweenTryOpen() const;
    Config &                    maxTimeBetweenTryOpen(int64_t max);

    static const utf8::String & defaultFileName();
    static utf8::String         defaultFileName(const utf8::String & name);
    const time_t & mtime() const;
    const uintptr_t & codePage() const;
    Config & codePage(uintptr_t a);
    const bool & silent() const;
    Config & silent(bool a);
    Config & save(AsyncFile * file = NULL);
  protected:
    Config & parseSectionHeader(ConfigSection & root);
    Config & parseSectionBody(ConfigSection & root);
  private:
    static uint8_t defaultFileName_[];
    AsyncFile file_;
    uintptr_t codePage_;
    AutoPtr<AsyncFile::LineGetBuffer> buffer_;

    time_t mtime_;
    Array<utf8::String> security_;
    uintptr_t maxTryOpenCount_;
    int64_t minTimeBetweenTryOpen_;
    int64_t maxTimeBetweenTryOpen_;

    enum TokenType { 
      ttUnknown, 
      ttUnexpected, 
      ttNumeric, 
      ttString, 
      ttQuotedString, 
      ttLeftBrace, 
      ttRightBrace, 
      ttEqual, 
      ttSemicolon, 
      ttColon, 
      ttEof
    };
    TokenType               lastTokenType_;

    utf8::String            ahead_;
    utf8::String::Iterator  aheadi_;

    uintptr_t line_;
    bool silent_;

    utf8::String  getToken(TokenType & tt, bool throwUnexpectedEof = false);
    Config &      putToken(const utf8::String & s, TokenType tt);

    static void   initialize();
    static void   cleanup();
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline const utf8::String & Config::fileName() const
{
  return file_.fileName();
}
//---------------------------------------------------------------------------
inline Config & Config::fileName(const utf8::String & name)
{
  file_.fileName(name).exclusive(true);
  mtime_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline const utf8::String & Config::defaultFileName()
{
  return *reinterpret_cast< const utf8::String *>(defaultFileName_);
}
//---------------------------------------------------------------------------
inline const uintptr_t & Config::maxTryOpenCount() const
{
  return maxTryOpenCount_;
}
//---------------------------------------------------------------------------
inline Config & Config::maxTryOpenCount(uintptr_t n)
{
  maxTryOpenCount_ = n < 1 ? 1 : n;
  return *this;
}
//---------------------------------------------------------------------------
inline const int64_t & Config::minTimeBetweenTryOpen() const
{
  return minTimeBetweenTryOpen_;
}
//---------------------------------------------------------------------------
inline const int64_t & Config::maxTimeBetweenTryOpen() const
{
  return maxTimeBetweenTryOpen_;
}
//---------------------------------------------------------------------------
inline const time_t & Config::mtime() const
{
  return mtime_;
}
//---------------------------------------------------------------------------
inline const uintptr_t & Config::codePage() const
{
  return codePage_;
}
//---------------------------------------------------------------------------
inline Config & Config::codePage(uintptr_t a)
{
  codePage_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & Config::silent() const
{
  return silent_;
}
//---------------------------------------------------------------------------
inline Config & Config::silent(bool a)
{
  silent_ = a;
  return *this;
}
//---------------------------------------------------------------------------
inline Config & Config::save(AsyncFile * file)
{
  saveSection(codePage_,file == NULL ? file_ : *file,true);
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <typename T>
class InterlockedConfig : public Config, public T {
  friend void initialize();
  friend void cleanup();
  friend class EConfig;
  friend class Section;
  public:
    ~InterlockedConfig();
    InterlockedConfig();

    void addRef();
    void remRef();
  protected:
  private:
    volatile int32_t refCount_;
    InterlockedConfig(const InterlockedConfig<T> &){}
    void operator = (const InterlockedConfig<T> &){}
};
//---------------------------------------------------------------------------
template <typename T> inline
InterlockedConfig<T>::~InterlockedConfig()
{
}
//---------------------------------------------------------------------------
template <typename T> inline
InterlockedConfig<T>::InterlockedConfig() : refCount_(0)
{
}
//---------------------------------------------------------------------------
template <typename T> inline
void InterlockedConfig<T>::addRef()
{
  if( this != NULL ) interlockedIncrement(refCount_,1);
}
//---------------------------------------------------------------------------
template <typename T> inline
void InterlockedConfig<T>::remRef()
{
  if( this != NULL && interlockedIncrement(refCount_,-1) == 1 ) deleteObject(this);
}
//---------------------------------------------------------------------------
typedef SPIARC<InterlockedConfig<InterlockedMutex> > ConfigSPi;
typedef SPIARC<InterlockedConfig<FiberInterlockedMutex> > ConfigSP;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EConfig : public Exception {
  public:
    EConfig() {}
    EConfig(Config * config,const char * what);
    EConfig(Config * config,const utf8::String & what);
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _conf_H_ */
//---------------------------------------------------------------------------
