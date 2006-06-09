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
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Config::~Config()
{
}
//---------------------------------------------------------------------------
Config::Config() : 
  ConfigSection(utf8::String()),
  mtime_(0),
  maxTryOpenCount_(3),
  minTimeBetweenTryOpen_(1),
  maxTimeBetweenTryOpen_(1000000),
  aheadi_(ahead_)
{
}
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
Mutant ConfigSection::value(uintptr_t i) const
{
  HashedObjectListItem< utf8::String,Mutant> *  item;
  item = values_.itemOfIndex(i);
  assert(item != NULL);
  return *item->object();
}
//---------------------------------------------------------------------------
utf8::String ConfigSection::text(const utf8::String & key, const utf8::String & defText) const
{
  utf8::String m(defText);
  HashedObjectListItem< utf8::String,Mutant> *  item;
  if( (item = values_.itemOfKey(key)) != NULL ) m = *item->object();
  return m;
}
//---------------------------------------------------------------------------
utf8::String ConfigSection::text(uintptr_t i) const
{
  HashedObjectListItem< utf8::String,Mutant> *  item;
  item = values_.itemOfIndex(i);
  assert(item != NULL);
  return *item->object();
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
  for( ; ; ){
    while( e.getChar() != '.' && e.next() );
    if( e - b < 1 )
      break;
    if( e.eof() ){
      return section->valueRef(utf8::String(b, e));
    }
    else{
      section = &section->section(utf8::String(b, e));
      b = ++e;
    }
  }
  throw ksys::ExceptionSP(new Exception(ENOENT, __PRETTY_FUNCTION__));
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
#ifdef __BCPLUSPLUS__
#pragma option push -w-8004
#endif
//---------------------------------------------------------------------------
utf8::String Config::getToken(TokenType & tt, bool throwUnexpectedEof)
{
  TokenType     t;
  bool          inQuoted      = false, screened = false;
  uintptr_t     commentLevel  = 0, prevChar = 0;
  uintptr_t     maxTokenLen   = 0;
  uintptr_t     c, ctype;
  utf8::String  token;
  t = tt = ttUnknown;
  for(;;){
    if( aheadi_.eof() ){
      uintptr_t size;
      if( isRunInFiber() ){
        size = afile_->gets(rs_);
      }
      else {
        size = file_.gets(rs_);
      }
      if( size == 0 ){
        ahead_.resize(0);
        t = ttEof;
      }
      else{
        line_++;
        ahead_ = utf8::String(rs_.ptr(), size);
      }
      aheadi_ = ahead_;
    }
    if( !aheadi_.eof() ){
      if( inQuoted ){
        if( screened ){
          screened = false;
          uintptr_t     screenLen;
          utf8::String  sc  = unScreenChar(aheadi_, screenLen);
          ahead_ = ahead_.replace(aheadi_, aheadi_ + screenLen + 1, sc);
        }
        else if( aheadi_.getChar() == '\"' ){
          inQuoted = false;
          aheadi_.next();
          continue;
        }
        else if( aheadi_.getChar() == '\\' ){
          screened = true;
          continue;
        }
      }
      else{
        ctype = utf8::getC1Type(c = aheadi_.getChar());
        if( commentLevel > 0 ){
          if( prevChar == '*' && c == '/' ) commentLevel--;
          aheadi_.next();
          t = tt = ttUnknown;
          continue;
        }
        if( (ctype & (C1_SPACE | C1_CNTRL)) != 0 || c == '\r' || c == '\n' ){
          if( tt != ttUnknown ) break;
          aheadi_.next();
          continue;
        }
        if( prevChar == '/' && c == '/' ){
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
        }
        else if( c == '}' ){
          t = ttRightBrace;
          maxTokenLen = 1;
        }
        else if( c == '=' ){
          t = ttEqual;
          maxTokenLen = 1;
        }
        else if( c == ';' ){
          t = ttSemicolon;
          maxTokenLen = 1;
        }
        else if( c == ',' ){
          t = ttColon;
          maxTokenLen = 1;
        }
        else if( (ctype & C1_CNTRL) == 0 ){
          if( tt != ttNumeric || (c != 'K' && c != 'k' && c != 'M' && c != 'm' && c != 'G' && c != 'g' && !isdigit((int) prevChar)) ){
            t = ttString;
            maxTokenLen = ~uintptr_t(0);
          }
        }
        else{
          t = ttUnknown;
          maxTokenLen = 0;
        }
        prevChar = c;
      }
    }
    if( t != tt && tt != ttUnknown )
      break;
    tt = t;
    if( tt == ttEof ) break;
    token += utf8::String(aheadi_, aheadi_ + 1);
    aheadi_.next();
    if( token.strlen() == maxTokenLen ) break;
  }
  lastTokenType_ = tt;
  if( throwUnexpectedEof && tt == ttEof )
    throw ksys::ExceptionSP(new EConfig(this, "unexpected end of file"));
  return token;
}
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
#pragma option pop
#endif
//---------------------------------------------------------------------------
Config & Config::putToken(const utf8::String & s, TokenType tt)
{
  switch( tt ){
    case ttQuotedString :
      ahead_ = ahead_.insert(aheadi_, "\"" + unScreenString(s) + "\"");
      break;
    default :
      ahead_ = ahead_.insert(aheadi_, s);
      break;
  }
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::parseSectionHeader(ConfigSection & root)
{
  TokenType     tt;
  utf8::String  token;
  utf8::String  param;
  for( ; ; ){
    // get section header values
    token = getToken(tt);
    if( tt == ttLeftBrace )
      break;
    if( tt != ttString && tt != ttQuotedString && tt != ttNumeric )
      throw ksys::ExceptionSP(new EConfig(this, "invalid section param"));
    if( param.strlen() > 0 ) param += ", ";
    if( tt == ttQuotedString ){
      param += screenString(token);
    }
    else{
      param += token;
    }
    token = getToken(tt);
    if( tt == ttLeftBrace )
      break;
    if( tt != ttColon )
      throw ksys::ExceptionSP(new EConfig(
        this, "unexpected token '" + token + "', expecting colon"));
  }
  if( param.strlen() > 0 )
    root.values_.add(new Mutant(param), utf8::String());
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::parseSectionBody(ConfigSection & root)
{
  TokenType     tt;
  utf8::String  token;
  for( ; ; ){
    token = getToken(tt);
    if( tt == ttEof ){
      if( &root != this ) getToken(tt, true);
      break;
    }
    if( tt == ttRightBrace ){
      if( &root == this )
        throw ksys::ExceptionSP(new EConfig(this, "unexpected token '" + token + "'"));
      break;
    }
    if( (tt != ttString && tt != ttQuotedString && tt != ttNumeric) || token.strlen() == 0 )
      throw ksys::ExceptionSP(new EConfig(this, "invalid section or key name"));
    utf8::String key(token);
    token = getToken(tt);
    if( tt != ttEqual ){
      // try new subsection
      HashedObjectListItem<utf8::String,ConfigSection> * item;
      root.subSections_.add(new ConfigSection(key),key,&item);
      putToken(token,tt);
      parseSectionHeader(*item->object());
      parseSectionBody(*item->object());
    }
    else{
      utf8::String value;
      for( ; ; ){
        // get key values
        token = getToken(tt);
        if( tt == ttSemicolon ) break;
        if( tt != ttString && tt != ttQuotedString && tt != ttNumeric )
          throw ksys::ExceptionSP(new EConfig(this, "invalid section key value"));
        if( value.strlen() > 0 ) value += ", ";
        if( tt == ttQuotedString ){
          value += screenString(token);
        }
        else {
          value += token;
        }
        token = getToken(tt);
        if( tt == ttSemicolon ) break;
        if( tt != ttColon )
          throw ksys::ExceptionSP(new EConfig(
            this, "unexpected token '" + token + "', expecting colon"));
      }
      root.values_.add(new Mutant(value), key);
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
      file_.fileName(changeFileExt(getExecutableName(), "conf"));
    }
    else{
      file_.fileName(defaultFileName());
    }
  }
  fileName = file_.fileName();
  if( isRunInFiber() ){
    if( afile_ == NULL ) afile_ = new AsyncFile;
    afile_->fileName(fileName);
  }
  struct Stat st;
  bool stf;
  if( isRunInFiber() ) stf = statAsync(fileName,st); else stf = stat(fileName,st);
  if( stf && mtime_ != st.st_mtime ){
    AutoPtr<Randomizer> rnd(new Randomizer);
    rnd->randomize();
    for( intptr_t i = isRunInFiber() ? 0 : maxTryOpenCount_ - 1; i >= 0; i-- ){
      try {
        if( isRunInFiber() ){
          afile_->detach();
          afile_->readOnly(true).open();
        }
        else {
          file_.readOnly(true).open();
        }
#ifndef NDEBUG
        fprintf(stderr,"config file %s used\n", (const char *) file_.fileName().getANSIString());
#endif
        line_ = 0;
        lastTokenType_ = ttUnknown;
        ahead_.resize(0);
        clear();
        parseSectionBody(*this);
        rs_.free();
        stdErr.log(
          lmINFO,
          utf8::String::Stream() << "config file " << file_.fileName() << " parsed\n"
        );
        mtime_ = st.st_mtime;
        break;
      }
      catch( ExceptionSP & e ){
        if( isRunInFiber() ){
          afile_->close();
        }
        else {
          file_.close();
        }
#if defined(__WIN32__) || defined(__WIN64__)
        if( e->code() != ERROR_SHARING_VIOLATION + errorOffset &&
            e->code() != ERROR_LOCK_VIOLATION + errorOffset )
#else
        if( e->code() != EWOULDBLOCK )
#endif
          throw;
      }
      if( isRunInFiber() ){
        afile_->close();
        afile_->detach();
      }
      else {
        file_.close();
      }
      sleep(rnd->random(maxTimeBetweenTryOpen_ - minTimeBetweenTryOpen_ + 1) + minTimeBetweenTryOpen_);
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
Config & Config::override(const Array< utf8::String> & oargv)
{
  const Array< utf8::String> &  pargv = oargv.count() > 0 ? oargv : argv();
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
EConfig::EConfig(Config * config, const utf8::String & what)
  : Exception(EINVAL, "[config error] " + 
                config->file_.fileName() + ", " +
                utf8::int2Str((intmax_t) config->line_) + " : " + what)
{
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

