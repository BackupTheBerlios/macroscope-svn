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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
Mutant ConfigSection::value(const utf8::String & key, const Mutant & defValue) const
{
  Mutant m(defValue);
  HashedObjectListItem< utf8::String,Mutant> *  item;
  item = values_.itemOfKey(key);
  if( item != NULL ) m = *item->object();
  return m;
}
//---------------------------------------------------------------------------
Mutant ConfigSection::value(uintptr_t i,utf8::String * pKey) const
{
  HashedObjectListItem<utf8::String,Mutant> * item;
  item = values_.itemOfIndex(i);
  assert( item != NULL );
  if( pKey != NULL ) *pKey = item->key();
  return *item->object();
}
//---------------------------------------------------------------------------
utf8::String ConfigSection::text(const utf8::String & key,const utf8::String & defText) const
{
  utf8::String m(defText);
  HashedObjectListItem< utf8::String,Mutant> *  item;
  if( (item = values_.itemOfKey(key)) != NULL ) m = *item->object();
  return m;
}
//---------------------------------------------------------------------------
utf8::String ConfigSection::text(uintptr_t i,utf8::String * pKey) const
{
  HashedObjectListItem<utf8::String,Mutant> * item;
  item = values_.itemOfIndex(i);
  assert(item != NULL);
  if( pKey != NULL ) *pKey = item->key();
  return *item->object();
}
//---------------------------------------------------------------------------
ConfigSection & ConfigSection::sectionByPath(const utf8::String & path) const
{
  utf8::String::Iterator b(path), e(path);
  const ConfigSection * section = this;
  for(;;){
    while( e.getChar() != '.' && e.next() );
    if( e - b < 1 ) break;
    section = &section->section(utf8::String(b,e));
    b = ++e;
  }
  return *const_cast<ConfigSection *>(section);
}
//---------------------------------------------------------------------------
Mutant ConfigSection::valueByPath(const utf8::String & path, const Mutant & defValue) const
{
  Mutant &  m = valueRefByPath(path);
  return m.type() == mtNull && defValue.type() != mtNull ? defValue : m;
}
//---------------------------------------------------------------------------
Mutant & ConfigSection::valueRefByPath(const utf8::String & path) const
{
  utf8::String::Iterator b(path), e(path);
  const ConfigSection * section = this;
  for(;;){
    while( e.getChar() != '.' && e.next() );
    if( e - b < 1 ) break;
    if( e.eof() ){
      return section->valueRef(utf8::String(b,e));
    }
    else {
      section = &section->section(utf8::String(b,e));
      b = ++e;
    }
  }
  newObject<Exception>(ENOENT,__PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
static void saveSectionHelper(
  utf8::String::Stream & stream,
  utf8::String key,
  utf8::String value,
  uintptr_t level)
{
  if( key.strlen() > 0 ){
    utf8::String::Iterator keyIt(key);
    while( !keyIt.eof() ){
      if( keyIt.isBlank() || keyIt.isCntrl() || keyIt.getChar() == '\"' ){
        key = "\"" + screenString(key) + "\"";
        break;
      }
      keyIt.next();
    }
  }
  utf8::String::Iterator valueIt(value);
  while( !valueIt.eof() ){
    if( valueIt.isBlank() || valueIt.isCntrl() || valueIt.getChar() == '\"' ){
      value = "\"" + screenString(value) + "\"";
      break;
    }
    valueIt.next();
  }
  if( key.strlen() > 0 ){
    for( uintptr_t j = 0; j < level; j++ ) stream << "  ";
    stream << key << " = ";
  }
  stream << value;
  if( key.strlen() > 0 ) stream << ";\n";
}
//---------------------------------------------------------------------------
ConfigSection & ConfigSection::saveSection(uintptr_t codePage,AsyncFile & file,bool recursive,uintptr_t level)
{
  if( level == 0 ){
    if( !file.isOpen() ) file.readOnly(false).exclusive(true).createIfNotExist(true).open();
    file.resize(0);
  }
  uintptr_t i;
  utf8::String::Stream stream;
  if( name_.strlen() > 0 ){
    for( i = 0; i < level - 1; i++ ) stream << "  ";
    stream << name_ << " ";
    Mutant * m = values_.objectOfKey(utf8::String());
    saveSectionHelper(stream,utf8::String(),m == NULL ? utf8::String() : (utf8::String) *m,level);
    stream << " {\n";
  }
  for( i = 0; i < values_.count(); i++ ){
    HashedObjectListItem<utf8::String,Mutant> * item = values_.itemOfIndex(i);
    utf8::String key(item->key());
    if( key.strlen() == 0 ) continue;
    utf8::String value(*item->object());
    saveSectionHelper(stream,key,value,level);
  }
  if( name_.strlen() > 0 ){
    for( i = 0; i < level - 1; i++ ) stream << "  ";
    stream << "}\n";
  }
  AutoPtr<uint8_t> s;
  i = stream.string().getMBCSString(codePage,s,false);
  file.writeBuffer(s,i);
  if( recursive ){
    for( i = 0; i < subSections_.count(); i++ )
      subSections_[i]->saveSection(codePage,file,recursive,level + 1);
  }
  if( level == 0 ) file.close();
  return *this;
}
//---------------------------------------------------------------------------
ConfigSection & ConfigSection::setValue(const utf8::String & key,const Mutant & value) const
{
  HashedObjectListItem<utf8::String,Mutant> * item = values_.itemOfKey(key);
  if( item != NULL ) values_.removeByKey(key);
  AutoPtr<Mutant> m(newObject<Mutant>(value));
  values_.add(m,key);
  m.ptr(NULL);
  return *const_cast<ConfigSection *>(this);
}
//---------------------------------------------------------------------------
ConfigSection & ConfigSection::setValue(uintptr_t i,const Mutant & value) const
{
  HashedObjectListItem<utf8::String,Mutant> * item = values_.itemOfIndex(i);
  assert( item != NULL );
  utf8::String key(item->key());
  values_.removeByIndex(i);
  AutoPtr<Mutant> m(newObject<Mutant>(value));
  values_.add(m,key);
  m.ptr(NULL);
  return *const_cast<ConfigSection *>(this);
}
//---------------------------------------------------------------------------
ConfigSection & ConfigSection::addSection(const ConfigSection & section)
{
  intptr_t i;
  for( i = section.subSections_.count() - 1; i >= 0; i-- ){
    ConfigSection * subSection = section.subSections_.objectOfIndex(i);
    ConfigSection * subSection2 = subSections_.objectOfKey(subSection->name_);
    if( subSection2 == NULL ){
      AutoPtr<ConfigSection> p(newObject<ConfigSection>(subSection->name_));
      subSections_.add(p,subSection->name_);
      subSection2 = p.ptr(NULL);
    }
    subSection2->addSection(*subSection);
  }  
  for( i = section.values_.count() - 1; i >= 0; i-- ){
    HashedObjectListItem<utf8::String,Mutant> * item = section.values_.itemOfIndex(i);
    if( item->key().strlen() > 0 ) setValue(item->key(),*item->object());
  }  
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
uint8_t Config::defaultFileName_[sizeof(utf8::String)];
//---------------------------------------------------------------------------
void Config::initialize()
{
  new (defaultFileName_) utf8::String;
}
//---------------------------------------------------------------------------
void Config::cleanup()
{
  using utf8::String;
  defaultFileName().~String();
}
//---------------------------------------------------------------------------
utf8::String Config::defaultFileName(const utf8::String & name)
{
  utf8::String oldName(*reinterpret_cast<const utf8::String *>(defaultFileName_));
  *reinterpret_cast< utf8::String *>(defaultFileName_) = name;
  return oldName;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Config::~Config()
{
}
//---------------------------------------------------------------------------
Config::Config() : 
  ConfigSection(utf8::String()),
  codePage_(CP_ACP),
  mtime_(0),
  maxTryOpenCount_(3),
  minTimeBetweenTryOpen_(1),
  maxTimeBetweenTryOpen_(1000000),
  aheadi_(ahead_),
  silent_(false)
{
  file_.createPath(false);
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma option push -w-8004
#endif
//---------------------------------------------------------------------------
utf8::String Config::getToken(TokenType & tt, bool throwUnexpectedEof)
{
  TokenType t;
  bool inQuoted = false, screened = false;
  uintptr_t commentLevel = 0, prevChar = 0;
  uintptr_t maxTokenLen = 0;
  uintptr_t c, ctype;
  utf8::String token;
  t = tt = ttUnknown;
  for(;;){
    if( aheadi_.eof() ){
      if( file_.gets(ahead_,buffer_) ){
        ahead_.resize(0);
        t = ttEof;
      }
      else {
        line_++;
      }
      aheadi_ = ahead_;
    }
    if( !aheadi_.eof() ){
      if( inQuoted ){
        if( screened ){
          screened = false;
          uintptr_t screenLen;
          utf8::String sc = unScreenChar(aheadi_,screenLen);
          ahead_.replace(aheadi_,aheadi_ + screenLen + 1,sc);
        }
        else if( aheadi_.getChar() == '\"' ){
          inQuoted = false;
          aheadi_.next();
	  prevChar = 0;
          continue;
        }
        else if( aheadi_.getChar() == '\\' ){
          screened = true;
          continue;
        }
      }
      else {
        ctype = utf8::getC1Type(c = aheadi_.getChar());
        if( commentLevel > 0 ){
          if( prevChar == '*' && c == '/' ) commentLevel--;
	  prevChar = c;
          aheadi_.next();
          t = tt = ttUnknown;
          continue;
        }
        if( (ctype & (C1_SPACE | C1_CNTRL)) != 0 || c == '\r' || c == '\n' ){
	  prevChar = 0;
          if( tt != ttUnknown ) break;
          aheadi_.next();
          continue;
        }
        if( prevChar == '/' && c == '/' && commentLevel == 0 ){
          token.resize(token.size() - (aheadi_ - 1).seqLen());
          aheadi_.last();
          t = tt = ttUnknown;
          prevChar = 0;
          if( token.strlen() > 0 ) break; else continue;
        }
        else if( prevChar == '/' && c == '*' ){
          t = tt = ttUnknown;
          prevChar = c;
          commentLevel++;
          token.resize(token.size() - (aheadi_ - 1).seqLen());
          continue;
        }
        else if( (ctype & C1_DIGIT) != 0 && (tt == ttUnknown || tt == ttNumeric) ){
          t = ttNumeric;
          maxTokenLen = ~(uintptr_t) 0;
        }
        else if( c == '\"' ){
          inQuoted = true;
          t = ttQuotedString;
          maxTokenLen = ~(uintptr_t) 0;
          aheadi_.next();
          continue;
        }
        else if( c == '{' ){
          t = ttLeftBrace;
          maxTokenLen = 1;
	  c = 0;
        }
        else if( c == '}' ){
          t = ttRightBrace;
          maxTokenLen = 1;
	  c = 0;
        }
        else if( c == '=' ){
          t = ttEqual;
          maxTokenLen = 1;
	  c = 0;
        }
	else if( c == ';' ){
          t = ttSemicolon;
          maxTokenLen = 1;
	  c = 0;
        }
        else if( c == ',' ){
          t = ttColon;
          maxTokenLen = 1;
	  c = 0;
        }
        else if( (ctype & C1_CNTRL) == 0 ){
          if( tt != ttNumeric ||
              (c != 'K' && c != 'k' && c != 'M' && c != 'm' &&
               c != 'G' && c != 'g' && !isdigit((int) prevChar)) ){
            t = ttString;
            maxTokenLen = ~uintptr_t(0);
          }
	  else {
            c = 0;
	  }
        }
        else {
          t = ttUnknown;
          maxTokenLen = 0;
        }
        prevChar = c;
      }
    }
    if( t != tt && tt != ttUnknown )  break;
    tt = t;
    if( tt == ttEof ) break;
    token += utf8::String(aheadi_, aheadi_ + 1);
    aheadi_.next();
    if( token.strlen() == maxTokenLen ) break;
  }
  lastTokenType_ = tt;
  if( throwUnexpectedEof && tt == ttEof )
    newObject<EConfig>(this, "unexpected end of file")->throwSP();
  return token;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma option pop
#endif
//---------------------------------------------------------------------------
Config & Config::putToken(const utf8::String & s,TokenType tt)
{
  switch( tt ){
    case ttQuotedString :
      ahead_.insert(aheadi_,"\"" + screenString(s) + "\"");
      break;
    default :
      ahead_.insert(aheadi_,s);
      break;
  }
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::parseSectionHeader(ConfigSection & root)
{
  TokenType tt;
  utf8::String token;
  utf8::String param;
  for(;;){
    // get section header values
    token = getToken(tt);
    if( tt == ttLeftBrace ) break;
    if( tt != ttString && tt != ttQuotedString && tt != ttNumeric )
      newObject<EConfig>(this, "invalid section param")->throwSP();
    if( param.strlen() > 0 ) param += ",";
    if( tt == ttQuotedString ){
      param += unScreenString(token);
    }
    else {
      param += token;
    }
    token = getToken(tt);
    if( tt == ttLeftBrace ) break;
    if( tt != ttColon )
      newObject<EConfig>(this, "unexpected token '" + token + "', expecting colon")->throwSP();
  }
  if( param.strlen() > 0 ) root.setValue(utf8::String(),param);
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::parseSectionBody(ConfigSection & root)
{
  TokenType tt;
  utf8::String  token;
  for(;;){
    token = getToken(tt);
    if( tt == ttEof ){
      if( &root != this ) getToken(tt, true);
      break;
    }
    if( tt == ttRightBrace ){
      if( &root == this )
        newObject<EConfig>(this, "unexpected token '" + token + "'")->throwSP();
      break;
    }
    if( (tt != ttString && tt != ttQuotedString && tt != ttNumeric) || token.strlen() == 0 )
      newObject<EConfig>(this, "invalid section or key name")->throwSP();
    utf8::String key(token);
    token = getToken(tt);
    if( tt != ttEqual ){
      if( key.strcasecmp("#include") == 0 && (tt == ttString || tt == ttQuotedString || tt == ttNumeric) ){ // pragma
        Config config;
        config.fileName(token);
        if( !stat(token) )
          config.fileName(includeTrailingPathDelimiter(getPathFromPathName(file_.fileName())) + token);
        config.silent(true).parse();
        root.addSection(config);
      }
      else { // try new subsection
        HashedObjectListItem<utf8::String,ConfigSection> * item = root.subSections_.itemOfKey(key);
        if( item == NULL ){
          AutoPtr<ConfigSection> p(newObject<ConfigSection>(key));
          root.subSections_.add(p,key,&item);
          p.ptr(NULL);
        }
        putToken(token,tt);
        parseSectionHeader(*item->object());
        parseSectionBody(*item->object());
      }
    }
    else {
      utf8::String value;
      for(;;){
        // get key values
        token = getToken(tt);
        if( tt == ttSemicolon ) break;
        if( tt != ttString && tt != ttQuotedString && tt != ttNumeric )
          newObject<EConfig>(this, "invalid section key value")->throwSP();
        if( value.strlen() > 0 ) value += ",";
        if( tt == ttQuotedString ){
          value += unScreenString(token);
        }
        else {
          value += token;
        }
        token = getToken(tt);
        if( tt == ttSemicolon ) break;
        if( tt != ttColon )
          newObject<EConfig>(this, "unexpected token '" + token + "', expecting colon")->throwSP();
      }
      root.setValue(key,value);
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::parse()
{
  utf8::String fileName;
  if( file_.fileName().strlen() == 0 ){
    if( defaultFileName().strlen() == 0 ){
      file_.fileName(changeFileExt(getExecutableName(),".conf"));
    }
    else{
      file_.fileName(defaultFileName());
    }
  }
  fileName = file_.fileName();
  struct Stat st;
  if( stat(fileName,st) && mtime_ != st.st_mtime ){
    AutoPtr<Randomizer> rnd(newObject<Randomizer>());
    rnd->randomize();
    for( intptr_t i = maxTryOpenCount_ - 1; i >= 0; i-- ){
      try {
        file_.readOnly(true).exclusive(true).createIfNotExist(false).open();
/*#ifndef NDEBUG
        fprintf(stderr,"config file %s used\n", (const char *) file_.fileName().getANSIString());
#endif*/
        buffer_ = newObjectV<AsyncFile::LineGetBuffer>(file_);
        buffer_->codePage_ = codePage_;
        line_ = 0;
        lastTokenType_ = ttUnknown;
        ahead_.resize(0);
        clear();
        parseSectionBody(*this);
        if( !silent_ )
          stdErr.debug(9,
            utf8::String::Stream() <<
            "config file " <<
            file_.fileName() <<
            (mtime_ == 0 ? " loaded\n" : " reloaded\n")
          );
        i = 0;
        mtime_ = st.st_mtime;
      }
      catch( ExceptionSP & e ){
        buffer_ = NULL;
        file_.close();
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_SHARING_VIOLATION + errorOffset &&
            e->code() != ERROR_LOCK_VIOLATION + errorOffset )
#else
        if( e->code() != EWOULDBLOCK )
#endif
          throw;
      }
      buffer_ = NULL;
      file_.close();
      if( i > 0 )
        ksleep(rnd->random(
          maxTimeBetweenTryOpen_ - minTimeBetweenTryOpen_ + 1) + minTimeBetweenTryOpen_
        );
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::override(const Array< utf8::String> & oargv)
{
  const Array< utf8::String> & pargv = oargv.count() > 0 ? oargv : argv();
  for( uintptr_t arg = 0; arg < pargv.count(); arg++ ){
    if( pargv[arg].strcmp("-co") == 0 && arg + 2 < pargv.count() ){
      valueRefByPath(pargv[arg + 1]) = pargv[arg + 2];
      arg += 2;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::minTimeBetweenTryOpen(int64_t min)
{
  if( min < 1 )
    min = 1;
  if( min > maxTimeBetweenTryOpen_ ){
    minTimeBetweenTryOpen_ = maxTimeBetweenTryOpen_;
    maxTimeBetweenTryOpen_ = min;
  }
  else{
    minTimeBetweenTryOpen_ = min;
  }
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::maxTimeBetweenTryOpen(int64_t max)
{
  if( max < 1 )
    max = 1;
  if( max < minTimeBetweenTryOpen_ ){
    maxTimeBetweenTryOpen_ = minTimeBetweenTryOpen_;
    minTimeBetweenTryOpen_ = max;
  }
  else{
    maxTimeBetweenTryOpen_ = max;
  }
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
EConfig::EConfig(Config * config,const char * what)
  : Exception(EINVAL, "[config error] " + 
                config->file_.fileName() + ", " +
                utf8::int2Str((intmax_t) config->line_) + " : " + what)
{
}
//---------------------------------------------------------------------------
EConfig::EConfig(Config * config,const utf8::String & what)
  : Exception(EINVAL, "[config error] " + 
                config->file_.fileName() + ", " +
                utf8::int2Str((intmax_t) config->line_) + " : " + what)
{
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

