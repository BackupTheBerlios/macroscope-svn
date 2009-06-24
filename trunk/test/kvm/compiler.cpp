/*-
 * Copyright 2008-2009 Guram Dukashvili
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
//------------------------------------------------------------------------------
#include <adicpp/adicpp.h>
#include "compiler.h"
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
namespace kvm {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
Compiler::~Compiler()
{
}
//------------------------------------------------------------------------------
Compiler::Compiler() : keepStderr_(false)
{
}
//------------------------------------------------------------------------------
Compiler & Compiler::detect(const ConfigSP config)
{
  keepStderr_ = config->value("keep_compiler_stderr",false);
  utf8::String defaultCompiler(config->text("default_cxx_compiler","auto"));
  bool autoCompiler = defaultCompiler.casecompare("auto") == 0, f = false;
  for( uintptr_t i = 0; i < config->section("cxx_compilers").sectionCount(); i++ ){
    utf8::String sectionPath("cxx_compilers." + (autoCompiler ? config->section("cxx_compilers").section(i).name() : defaultCompiler));
    utf8::String envSectionPath(sectionPath + ".environment");
    Array<utf8::String> env;
    for( uintptr_t j = 0; j < config->sectionByPath(envSectionPath).valueCount(); j++ ){
      utf8::String key, value(config->sectionByPath(envSectionPath).value(j,&key)), e(getEnv(key));
      env.add(key + "=" + value + (e.isNull() ? utf8::String() : ";" + e));
    }
    utf8::String compiler(anyPathName2HostPathName(config->textByPath(sectionPath + ".compiler")));
    AsyncFile tmpCxx(anyPathName2HostPathName(getTempPath() + getTempFileName("cxx")));
    tmpCxx.createIfNotExist(true).open();
    utf8::String compilerArgs(config->textByPath(sectionPath + ".compiler_args")), compilerArgs2(compilerArgs);
    compilerArgs = compilerArgs.replaceCaseAll("${source}",anyPathName2HostPathName(tmpCxx.fileName()));
    utf8::String object(changeFileExt(tmpCxx.fileName(),".o"));
    compilerArgs = compilerArgs.replaceCaseAll("${object}",anyPathName2HostPathName(object));
    compilerArgs = compilerArgs.replaceCaseAll("${include_directories}",utf8::String());
    tmpCxx <<
      "namespace ksys {\n"
      "namespace kvm {\n"
      "class Test {\n"
      "  public:\n"
      "};\n"
      "class Test1 : virtual public Test {\n"
      "  public:\n"
      "};\n"
      "class Test2 : virtual public Test {\n"
      "  public:\n"
      "};\n"
      "class Test3 : public Test1, public Test2 {\n"
      "  public:\n"
      "};\n"
      "} // namespace kvm\n"
      "} // namespace ksys\n"
    ;
    tmpCxx.close();
    AutoFileRemove afr1(tmpCxx.fileName());
    AutoFileRemove afr2(object);
    
    ExecuteProcessParameters params;
    params.name_ = compiler;
    params.args_ = compilerArgs;
    params.env_ = env;
    params.wait_ = true;
    params.noThrow_ = true;
    pid_t exitCode = execute(params);
    if( exitCode == 0 && stat(object) ){ // detected
      utf8::String compiler(anyPathName2HostPathName(config->textByPath(sectionPath + ".compiler")));
      type_ = config->textByPath(sectionPath + ".type");

      // get version
      AsyncFile compilerVersion(changeFileExt(tmpCxx.fileName(),".ver"));
      params.stderr_ = params.stdout_ = compilerVersion.createIfNotExist(true).removeAfterClose(true).open().descriptor_;
      params.args_ = "--version";
      exitCode = execute(params);
      if( exitCode == 0 ){
        utf8::String version;
        compilerVersion_.resize(0);
        AsyncFile::LineGetBuffer b(compilerVersion.seek(0));
        while( !compilerVersion.getString(version,&b) ) compilerVersion_ += "// ***WARNING*** Don't edit. This is machine generated line. " + version;
      }

      compiler_ = compiler;
      compilerArgs_ = compilerArgs2;
      compilerEnv_ = env;
      linker_ = anyPathName2HostPathName(config->textByPath(sectionPath + ".linker"));
      linkerArgs_ = config->textByPath(sectionPath + ".linker_args");
      linkerDlArgs_ = config->textByPath(sectionPath + ".linker_dl_args");
      f = true;
      break;
    }
    if( !autoCompiler ) break;
  }
  if( !f )
    newObjectV1C2<Exception>(ENOENT,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//------------------------------------------------------------------------------
bool Compiler::testCxx(const utf8::String & config,const utf8::String & test,const utf8::String & tmpCxx)
{
  AsyncFile file(tmpCxx);
  file.createIfNotExist(true);
  utf8::String compilerArgs(compilerArgs_.replaceCaseAll("${source}",tmpCxx));
  utf8::String object(anyPathName2HostPathName(changeFileExt(tmpCxx,".o")));
  compilerArgs = compilerArgs.replaceCaseAll("${object}","\"" + anyPathName2HostPathName(object) + "\"");
  compilerArgs = compilerArgs.replaceCaseAll("${include_directories}",utf8::String());
  file.open().resize(0);
  if( stat(config) ){
    AsyncFile cfg(config);
    cfg.open();
    file.copy(cfg);
  }
  file << test;
  AutoFileRemove afr(tmpCxx);
  AutoFileRemove afr2(object);
  ExecuteProcessParameters params;
  params.name_ = compiler_;
  params.args_ = compilerArgs;
  params.env_ = compilerEnv_;
  params.wait_ = true;
  AsyncFile testStderr(changeFileExt(tmpCxx,".err"));
  params.stderr_ = params.stdout_ = testStderr.createIfNotExist(true).removeAfterClose(!keepStderr_).open().descriptor_;
  testStderr.seek(testStderr.size());
  file.seek(0);
  testStderr.copy(file);
  file.close();
  testStderr << "\n";
  testStderr << params.name_ + " " + params.args_ + "\n";
  pid_t exitCode = execute(params);
  testStderr << "\n";
  if( exitCode == 0 && stat(object) ){
    testStderr.removeAfterClose(true);
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
bool Compiler::testLinkCxx(
  const utf8::String & config,
  const utf8::String & test,
  const utf8::String & tmpCxx,
  const utf8::String & libraries)
{
  AsyncFile file(tmpCxx);
  file.createIfNotExist(true);
  utf8::String compilerArgs(compilerArgs_.replaceCaseAll("${source}",tmpCxx));
  utf8::String object(anyPathName2HostPathName(changeFileExt(tmpCxx,".o")));
  compilerArgs = compilerArgs.replaceCaseAll("${object}","\"" + anyPathName2HostPathName(object) + "\"");
  compilerArgs = compilerArgs.replaceCaseAll("${include_directories}",utf8::String());
  file.open().resize(0);
  if( stat(config) ){
    AsyncFile cfg(config);
    cfg.open();
    file.copy(cfg);
  }
  file << test;
  AutoFileRemove afr(tmpCxx);
  AutoFileRemove afr2(object);
  ExecuteProcessParameters params;
  params.name_ = compiler_;
  params.args_ = compilerArgs;
  params.env_ = compilerEnv_;
  params.wait_ = true;
  AsyncFile testStderr(changeFileExt(tmpCxx,".err"));
  params.stderr_ = params.stdout_ = testStderr.createIfNotExist(true).removeAfterClose(!keepStderr_).open().descriptor_;
  testStderr.seek(testStderr.size());
  file.seek(0);
  testStderr.copy(file);
  file.close();
  testStderr << "\n";
  testStderr << params.name_ + " " + params.args_ + "\n";
  pid_t exitCode = execute(params);
  testStderr << "\n";
  if( exitCode == 0 && stat(object) ){
    utf8::String executable(
      anyPathName2HostPathName(
#if defined(__WIN32__) || defined(__WIN64__)
        changeFileExt(
#endif
          tmpCxx
#if defined(__WIN32__) || defined(__WIN64__)
          ,".exe"
#endif
        )
      )
    );
    utf8::String linkerArgs(linkerArgs_.replaceCaseAll("${object}","\"" + anyPathName2HostPathName(object) + "\""));
    linkerArgs = linkerArgs.replaceCaseAll("${executable}",executable);
    utf8::String libs;
    for( uintptr_t k = enumStringParts(libraries,",",false), i = 0; i < k; i++ ){
      utf8::String s(stringPartByNo(libraries,i,",",false));
      libs += libs.isNull() ? "" : " ";
      if( type_.casecompare("gnu") == 0 ){
        libs += "\"-l" + s + "\"";
      }
      else if( type_.casecompare("msvc") == 0 || type_.casecompare("intel") == 0 ){
        libs += "\"" + s.right(4).casecompare(".lib") == 0 ? s : s + ".lib" + "\"";
      }
      else {
        libs += s;
      }
    }
    linkerArgs = linkerArgs.replaceCaseAll("${libraries}",libs);
    linkerArgs = linkerArgs.replaceCaseAll("${lib_directories}","");
    params.name_ = linker_;
    params.args_ = linkerArgs;
    testStderr << params.name_ + " " + params.args_ + "\n";
    exitCode = execute(params);
    if( exitCode == 0 && stat(executable) ){
      testStderr.removeAfterClose(true);
      return true;
    }
  }
  return false;
}
//------------------------------------------------------------------------------
intptr_t Compiler::testRunCxx(
  const utf8::String & config,
  const utf8::String & test,
  const utf8::String & tmpCxx,
  const utf8::String & libraries)
{
  if( testLinkCxx(config,test,tmpCxx,libraries) ){
    utf8::String executable(anyPathName2HostPathName(changeFileExt(tmpCxx,".exe")));
    AutoFileRemove afr3(executable);
    return execute(executable,utf8::String(),&compilerEnv_,true);
  }
  return -1;
}
//------------------------------------------------------------------------------
utf8::String Compiler::testCxxHeaderHelper(const utf8::String & config,const utf8::String & header,const utf8::String & tmpCxx,const utf8::String & headers)
{
  bool r = testCxx(config,headers + "#include <" + header + ">\n",tmpCxx);
  return "#define HAVE_" + header.replaceAll(".","_").replaceAll("/","_").upper() + (r ? " 1" : " 0") + "\n";
}
//------------------------------------------------------------------------------
intptr_t Compiler::testCxxCode(
  const utf8::String & config,
  const utf8::String & tmpCxx,
  const utf8::String & header,
  const utf8::String & body)
{
  intptr_t size = testRunCxx(
    config,
    header + body,
    tmpCxx
  );
  return size >= 0 ? size : 0;
}
//------------------------------------------------------------------------------
utf8::String Compiler::testCxxTypeHelper(
  const utf8::String & config,
  const utf8::String & type,
  const utf8::String & member,
  const utf8::String & tmpCxx,
  const utf8::String & header,
  const utf8::String & body)
{
  utf8::String s((type + (member.isNull() ? utf8::String() : " " + member)).
    replaceAll(" ","_").replaceAll("::","_").replaceAll("*","P").upper()
  );
  s += s.right(2).casecompare("_T") == 0 || s.right(2).casecompare("_P") == 0 ? " " : "_T ";
  return 
    "#define SIZEOF_" + s +
    utf8::int2Str(testCxxCode(config,tmpCxx,header,body)) + "\n"
  ;
}
//------------------------------------------------------------------------------
utf8::String Compiler::testCxxTypeEqualCheck(
  const utf8::String & config,
  const utf8::String & type1,
  const utf8::String & type2,
  const utf8::String & tmpCxx,
  const utf8::String & header)
{
  utf8::String t1(type1.replaceAll(" ","_").replaceAll("::","_").replaceAll("*","P").upper());
  t1 += t1.right(2).casecompare("_T") == 0 || t1.right(2).casecompare("_P") == 0 ? "" : "_T";
  utf8::String t2(type2.replaceAll(" ","_").replaceAll("::","_").replaceAll("*","P").upper());
  t2 += t2.right(2).casecompare("_T") == 0 || t1.right(2).casecompare("_P") == 0 ? "" : "_T";
  return 
    "#define HAVE_" + t1 + "_AS_" + t2 +
    (testCxx(config,header +
    "\n"
    "class Test {\n"
    "  public:\n"
    "#if SIZEOF_" + t1 + " > 0\n"
    "    void testType(" + type1 + " &);\n"
    "#endif\n"
    "#if SIZEOF_" + t2 + " > 0\n"
    "    void testType(" + type2 + " &);\n"
    "#endif\n"
    "};\n"
    "\n"
    "int main(int /*argc*/,char * /*argv*/[])\n"
    "{\n"
    "  Test t;\n"
    "#if SIZEOF_" + t1 + " > 0\n"
    "  " + type1 + " v1;\n"
    "#endif\n"
    "#if SIZEOF_" + t2 + " > 0\n"
    "  " + type2 + " v2;\n"
    "#endif\n"
    "#if SIZEOF_" + t1 + " > 0\n"
    "  t.testType(v1);\n"
    "#endif\n"
    "#if SIZEOF_" + t2 + " > 0\n"
    "  t.testType(v2);\n"
    "#endif\n"
    "  return 0;\n"
    "}\n",
    tmpCxx) ? " 0" : " 1") + "\n"
  ;
}
//------------------------------------------------------------------------------
utf8::String Compiler::testCxxFuncExists(
  const utf8::String & config,
  const utf8::String & func,
  const utf8::String & tmpCxx,
  const utf8::String & header)
{
  utf8::String f(func.replaceAll("::","_").upper());
  utf8::String name(createGUIDAsBase32String());
  return 
    "#define HAVE_" + f +
    (testCxx(config,header +
    "\n"
    "void * funcPtr" + name + " = (void *) " + func + ";\n"
    "\n"
    "int main(int /*argc*/,char * /*argv*/[])\n"
    "{\n"
    "  return funcPtr" + name + " == NULL ? 1 : 0;\n"
    "}\n",
    tmpCxx) ? " 1" : " 0") + "\n"
  ;
}
//------------------------------------------------------------------------------
utf8::String Compiler::testCxxSymbolExists(
  const utf8::String & config,
  const utf8::String & symbol,
  const utf8::String & tmpCxx,
  const utf8::String & header)
{
  utf8::String s(symbol.replaceAll("::","_").upper());
  return 
    "#define HAVE_" + s +
    (testCxx(config,header +
    "void requireSymbol(int /*dummy*/,...)\n"
    "{\n"
    "}\n"
    "\n"
    "int main(int /*argc*/,char * /*argv*/[])\n"
    "{\n"
    "#ifdef " + symbol + "\n"
    "  return 0;\n"
    "#else\n"
    "#error Symbol " + symbol + " not exists\n"
    "#endif\n"
    "}\n",
    tmpCxx) ? " 1" : " 0") + "\n"
  ;
}
//------------------------------------------------------------------------------
utf8::String Compiler::testCxxLibExists(
  const utf8::String & library,
  const utf8::String & symbol,
  const utf8::String & mod,
  const utf8::String & tmpCxx,
  utf8::String * existsLibraries)
{
  utf8::String lib(library.upper());
  utf8::String name(createGUIDAsBase32String());
  bool r =
    testLinkCxx(
      utf8::String(),
      "extern \"C\" {\n"
      "extern " + (mod.isNull() ? "void " + symbol + "(void)" : mod.replaceAll("${symbol}",symbol)) + ";\n"
      "void * symbol" + name + " = (void *) " + symbol + ";\n"
      "}\n"
      "\n"
      "int main(int /*argc*/,char * /*argv*/[])\n"
      "{\n"
      "  return symbol" + name + " == (void *) 0 ? 1 : 0;\n"
      "}\n",
      tmpCxx,
      library
    );
  utf8::String executable(changeFileExt(tmpCxx,".exe"));
  AutoFileRemove afr3(executable);
  if( existsLibraries != NULL && r ) *existsLibraries += existsLibraries->isNull() ? library : ", " + library;
  return "#define HAVE_" + lib + "_LIB" + (r ? " 1" : " 0") + "\n";
}
//------------------------------------------------------------------------------
Compiler & Compiler::test(const utf8::String & config)
{
  /*if( !stat(config) )*/{
    AsyncFile out(config);
    out.createIfNotExist(true).removeAfterClose(true);
    utf8::String tmpCxx(anyPathName2HostPathName(getTempPath() + getTempFileName("cxx")));
    out.open().resize(0);

    out <<
      "// ***WARNING*** Don't edit. This is machine generated line. Compiler version:\n" +
      compilerVersion_
    ;

    // detect includes
    out << "\n// includes\n";
    static const char header[] = {
      "#if HAVE_STDINT_H\n"
      "#include <stdint.h>\n"
      "#endif\n"
      "#if HAVE_INTTYPES_H\n"
      "#include <inttypes.h>\n"
      "#endif\n"
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_TIME_H\n"
      "#include <sys/time.h>\n"
      "#endif\n"
      "#if HAVE_SYS_UUID_H\n"
      "#include <sys/uuid.h>\n"
      "#endif\n"
      "#if HAVE_SYS_EVENT_H\n"
      "#include <sys/event.h>\n"
      "#endif\n"
      "#if HAVE_SYS_UTIME_H\n"
      "#include <sys/utime.h>\n"
      "#endif\n"
      "#if HAVE_SYS_ENDIAN_H\n"
      "#include <sys/endian.h>\n"
      "#endif\n"
      "#if HAVE_UCONTEXT_H\n"
      "#include <ucontext.h>\n"
      "#endif\n"
      "#if HAVE_UTIME_H\n"
      "#include <utime.h>\n"
      "#endif\n"
      "#if HAVE_UUID_H\n"
      "#include <uuid.h>\n"
      "#endif\n"
      "#if HAVE_AIO_H\n"
      "#include <aio.h>\n"
      "#endif\n"
      "#if HAVE_STDDEF_H\n"
      "#include <stddef.h>\n"
      "#endif\n"
      "#if HAVE_SIGNAL_H\n"
      "#include <signal.h>\n"
      "#endif\n"
    };

    static const char * const headers[] = {
      "stddef.h",
      "stdint.h",
      "inttypes.h",
      "sys/types.h",
      "sys/param.h",
      "sys/ipc.h",
      "sys/sem.h",
      "sys/stat.h",
      "sys/utsname.h",
      "sys/socket.h",
      "sys/uio.h",
      "sys/mman.h",
      "sys/endian.h",
      "sys/time.h",
      "sys/utime.h",
      "sys/resource.h",
      "sys/ioctl.h",
      "sys/epoll.h",
      "arpa/inet.h",
      "netinet/in.h",
      "aio.h",
      "netdb.h",
      "memory.h",
      "stdio.h",
      "stdlib.h",
      "assert.h",
      "string.h",
      "err.h",
      "errno.h",
      "syslog.h",
      "execinfo.h",
      "stdarg.h",
      "sysexits.h",
      "sha256.h",
      "process.h",
      "dlfcn.h",
      "pthread.h",
      "signal.h",
      "fcntl.h",
      "unistd.h",
      "semaphore.h",
      "ctype.h",
      "pwd.h",
      "grp.h",
      "io.h",
      "paths.h",
      "time.h",
      "sched.h",
      "utime.h",
      "uuid.h",
      "math.h",
      "float.h",
      "ucontext.h",
      "dirent.h",
      "windows.h",
      "winsock.h",
      "winsock2.h",
      "winternl.h",
      "ntstatus.h",
      "ntdll.h",
      // Standart C++ headers
      "new",
      "new.h",
      "typeinfo",
      "typeinfo.h",
      "stdexcept",
      "stdexcept.h",
    };
    for( uintptr_t i = 0; i < sizeof(headers) / sizeof(headers[0]); i++ )
      out << testCxxHeaderHelper(config,headers[i],tmpCxx);
    out << testCxxHeaderHelper(config,"sys/timeb.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"sys/event.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"sys/uuid.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"machine/atomic.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"machine/cpufunc.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"machine/specialreg.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"machine/specialreg.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"sys/sysctl.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"sys/rtprio.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"netinet/in_systm.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_SYSTM_H\n"
      "#include <netinet/in_systm.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IP_H\n"
      "#include <netinet/ip.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"netinet/ip.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_SYSTM_H\n"
      "#include <netinet/in_systm.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IP_H\n"
      "#include <netinet/ip.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"netinet/tcp.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_SYSTM_H\n"
      "#include <netinet/in_systm.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IP_H\n"
      "#include <netinet/ip.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"net/if.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_SYSTM_H\n"
      "#include <netinet/in_systm.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IP_H\n"
      "#include <netinet/ip.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"net/if_types.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_SYSTM_H\n"
      "#include <netinet/in_systm.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IP_H\n"
      "#include <netinet/ip.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"net/route.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_SYSTM_H\n"
      "#include <netinet/in_systm.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IP_H\n"
      "#include <netinet/ip.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"net/ethernet.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_SYSTM_H\n"
      "#include <netinet/in_systm.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IP_H\n"
      "#include <netinet/ip.h>\n"
      "#endif\n"
    );      
    out << testCxxHeaderHelper(config,"net/if_dl.h",tmpCxx,
      "#if HAVE_SYS_TYPES_H\n"
      "#include <sys/types.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_SYSTM_H\n"
      "#include <netinet/in_systm.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IP_H\n"
      "#include <netinet/ip.h>\n"
      "#endif\n"
    );      
      
    out << testCxxHeaderHelper(config,"guiddef.h",tmpCxx,
      "#if HAVE_WINDOWS_H\n"
      "#include <windows.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"ws2tcpip.h",tmpCxx,
      "#if HAVE_WINSOCK2_H\n"
      "#include <winsock2.h>\n"
      "#elif HAVE_WINSOCK_H\n"
      "#include <winsock.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"tlhelp32.h",tmpCxx,
      "#if HAVE_WINDOWS_H\n"
      "#include <windows.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"iphlpapi.h",tmpCxx,
      "#if HAVE_WINDOWS_H\n"
      "#include <windows.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"wbemidl.h",tmpCxx,
      "#if HAVE_WINDOWS_H\n"
      "#include <windows.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"dbghelp.h",tmpCxx,
      "#if HAVE_WINDOWS_H\n"
      "#include <windows.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"sql.h",tmpCxx,
      "#if HAVE_WINDOWS_H\n"
      "#include <windows.h>\n"
      "#endif\n"
    );
    out << testCxxHeaderHelper(config,"mswsock.h",tmpCxx,
      "#if HAVE_WINDOWS_H\n"
      "#include <windows.h>\n"
      "#endif\n"
    );

    // detect type exists & sizes
    out << "\n// types\n";
    static const char * const types[] = {
      "char",
      "bool",
      "void *",
      "ulong",
      "wchar_t",
      "short",
      "int",
      "long",
      "__int8",
      "__int16",
      "__int32",
      "__int64",
      "long long",
      "long int",
      "long double",
      "ptrdiff_t",
      "intptr_t",
      "intmax_t",
      "pid_t",
      "uid_t",
      "gid_t",
      "uuid_t",
      "int8_t",
      "int16_t",
      "int32_t",
      "int64_t",
      "intmax_t",
      "ucontext_t",
      "struct timeval",
      "struct timezone",
      "struct aiocb",
      "struct uuid",
      "struct kevent",
      "struct utimbuf"
    };
    for( uintptr_t i = 0; i < sizeof(types) / sizeof(types[0]); i++ )
      out << testCxxTypeHelper(config,types[i],utf8::String(),tmpCxx,header,
        "\n"
        "int main(int /*argc*/,char * /*argv*/[])\n"
        "{\n"
        "  return (int) sizeof(" + utf8::String(types[i]) + ");\n"
        "}\n"
      );
    out << testCxxTypeHelper(config,"struct stat64",utf8::String(),tmpCxx,
      utf8::String(header) +
      "#if HAVE_SYS_STAT_H\n"
      "#include <sys/stat.h>\n"
      "#endif\n"
      ,
      "\n"
      "int main(int /*argc*/,char * /*argv*/[])\n"
      "{\n"
      "  return (int) sizeof(struct stat64);\n"
      "}\n"
    );
    out << testCxxTypeHelper(config,"struct _stat64",utf8::String(),tmpCxx,
      utf8::String(header) +
      "#if HAVE_SYS_STAT_H\n"
      "#include <sys/stat.h>\n"
      "#endif\n"
      ,
      "\n"
      "int main(int /*argc*/,char * /*argv*/[])\n"
      "{\n"
      "  return (int) sizeof(struct _stat64);\n"
      "}\n"
    );
    out << testCxxTypeHelper(config,"GUID",utf8::String(),tmpCxx,
      utf8::String(header) +
      "#if HAVE_WINDOWS_H\n"
      "#include <windows.h>\n"
      "#endif\n"
      "#if HAVE_GUIDDEF_H\n"
      "#include <guiddef.h>\n"
      "#endif\n"
      ,
      "\n"
      "int main(int /*argc*/,char * /*argv*/[])\n"
      "{\n"
      "  return (int) sizeof(GUID);\n"
      "}\n"
    );
    out << testCxxTypeHelper(config,"struct sockaddr_in6",utf8::String(),tmpCxx,
      utf8::String(header) +
      "#if HAVE_WINSOCK2_H\n"
      "#include <winsock2.h>\n"
      "#elif HAVE_WINSOCK_H\n"
      "#include <winsock.h>\n"
      "#endif\n"
      "#if HAVE_WS2TCPIP_H\n"
      "#include <ws2tcpip.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_ARPA_INET_H\n"
      "#include <arpa/inet.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NET_IF_DL_H\n"
      "#include <net/if_dl.h>\n"
      "#endif\n"
      ,
      "\n"
      "int main(int /*argc*/,char * /*argv*/[])\n"
      "{\n"
      "  return (int) sizeof(struct sockaddr_in6);\n"
      "}\n"
    );
    out << testCxxTypeHelper(config,"struct sockaddr_dl",utf8::String(),tmpCxx,
      utf8::String(header) +
      "#if HAVE_WINSOCK2_H\n"
      "#include <winsock2.h>\n"
      "#elif HAVE_WINSOCK_H\n"
      "#include <winsock.h>\n"
      "#endif\n"
      "#if HAVE_WS2TCPIP_H\n"
      "#include <ws2tcpip.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_ARPA_INET_H\n"
      "#include <arpa/inet.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NET_IF_DL_H\n"
      "#include <net/if_dl.h>\n"
      "#endif\n"
      ,
      "\n"
      "int main(int /*argc*/,char * /*argv*/[])\n"
      "{\n"
      "  return (int) sizeof(struct sockaddr_dl);\n"
      "}\n"
    );
    out << testCxxTypeHelper(config,"socklen_t",utf8::String(),tmpCxx,
      utf8::String(header) +
      "#if HAVE_WINSOCK2_H\n"
      "#include <winsock2.h>\n"
      "#elif HAVE_WINSOCK_H\n"
      "#include <winsock.h>\n"
      "#endif\n"
      "#if HAVE_WS2TCPIP_H\n"
      "#include <ws2tcpip.h>\n"
      "#endif\n"
      "#if HAVE_NETINET_IN_H\n"
      "#include <netinet/in.h>\n"
      "#endif\n"
      "#if HAVE_ARPA_INET_H\n"
      "#include <arpa/inet.h>\n"
      "#endif\n"
      "#if HAVE_SYS_SOCKET_H\n"
      "#include <sys/socket.h>\n"
      "#endif\n"
      "#if HAVE_NET_IF_DL_H\n"
      "#include <net/if_dl.h>\n"
      "#endif\n"
      ,
      "\n"
      "int main(int /*argc*/,char * /*argv*/[])\n"
      "{\n"
      "  return (int) sizeof(socklen_t);\n"
      "}\n"
    );

    static struct {
      const char * const type_;
      const char * const member_;
    } typesMembers[] = {
      { "union sigval", "sigval_ptr" },
      { "union sigval", "sival_ptr" }
    };
    for( uintptr_t i = 0; i < sizeof(typesMembers) / sizeof(typesMembers[0]); i++ )
      out << testCxxTypeHelper(config,typesMembers[i].type_,typesMembers[i].member_,tmpCxx,header,
        "\n"
        "int main(int /*argc*/,char * /*argv*/[])\n"
        "{\n"
        "  " + utf8::String(typesMembers[i].type_) + " v;\n"
        "\n"
        "  return (int) sizeof(v." + typesMembers[i].member_ + ");\n"
        "}\n"
      );

    // detect type1 equals type2
    out << "\n// type equivalents\n";
    static struct {
      const char * const type1_;
      const char * const type2_;
    } types2Equ[] = {
      { "int","intptr_t" },
      { "int","int32_t" },
      { "int","int64_t" },
      { "int","intmax_t" },
      { "int","long int" },
      { "int","long long" },

      { "long","intptr_t" },
      { "long","int32_t" },
      { "long","int64_t" },
      { "long","intmax_t" },
      { "long","long int" },
      { "long","long long" },

      { "long int","intptr_t" },
      { "long int","int32_t" },
      { "long int","int64_t" },
      { "long int","intmax_t" },

      { "long long","intptr_t" },
      { "long long","int32_t" },
      { "long long","int64_t" },
      { "long long","intmax_t" },

      { "intptr_t","int" },
      { "intptr_t","long" },
      { "intptr_t","long int" },
      { "intptr_t","long long" },
      { "intptr_t","int32_t" },
      { "intptr_t","int64_t" },
      { "intptr_t","intmax_t" },

      { "int32_t","int" },
      { "int32_t","long" },
      { "int32_t","long int" },
      { "int32_t","long long" },
      { "int32_t","intptr_t" },
      { "int32_t","int64_t" },
      { "int32_t","intmax_t" },

      { "int64_t","int" },
      { "int64_t","long" },
      { "int64_t","long int" },
      { "int64_t","long long" },
      { "int64_t","intptr_t" },
      { "int64_t","int32_t" },
      { "int64_t","intmax_t" },

      { "intmax_t","int" },
      { "intmax_t","long" },
      { "intmax_t","long int" },
      { "intmax_t","long long" },
      { "intmax_t","intptr_t" },
      { "intmax_t","int32_t" },
      { "intmax_t","int64_t" },
    };
    for( uintptr_t i = 0; i < sizeof(types2Equ) / sizeof(types2Equ[0]); i++ )
      out << testCxxTypeEqualCheck(config,types2Equ[i].type1_,types2Equ[i].type2_,tmpCxx,header);

    // detect functions
    out << "\n// functions\n";
    static const char * const functions[] = {
      "chown",
      "opendir",
      "closedir",
      "malloc",
      "realloc",
      "memcmp",
      "mktime",
      "timegm",
      "stat",
      "strerror_r",
      "utime",
      "_utime",
      "vprintf",
      "localtime",
      "gmtime_s",
      "gmtime",
      "getpwnam",
      "getgrnam",
      "floor",
      "floorl",
      "ftruncate",
      "getcwd",
      "getpagesize",
      "gettimeofday",
      "memmove",
      "memset",
      "memcpy",
      "bcopy",
      "casecompare",
      "strchr",
      "strncasecmp",
      "strstr",
      "sprintf",
      "_sprintf",
      "vsprintf",
      "snprintf",
      "_snprintf",
      "vsnprintf",
      "sscanf",
      "_sscanf",
      "snscanf",
      "_snscanf",
      "_chsize",
      "chsize",
      "fstat",
      "_fseeki64",
      "utime",
      "utimes",
      "nanosleep",
      "sleep",
      "usleep",
      "pread",
      "pwrite",
      "getuid",
      "getgid",
      "getpid",
      "gettid",
      "sem_init",
      "sem_close",
      "sem_open",
      "sem_unlink",
      "sem_timedwait",
      "uname",
      "umask",
      "readdir_r",
      "socket",
      "mkdir",
      "inet_ntoa",
      "gethostbyname",
      "gethostbyname2",
      "gethostbyaddr",
      "getnameinfo",
      "getaddrinfo",
      "strerror",
      "sysconf",
      "sigwait",
      "rmdir",
      "_isatty",
      "isatty",
      "_fileno",
      "fileno",
      "kqueue",
      "kevent",
      "aio_read",
      "aio_write",
      "aio_return",
      "aio_error",
      "mmap",
      "munmap",
      "mlock",
      "munlock",
      "mlockall",
      "munlockall",
      "shm_open",
      "shm_unlink",
      "daemon",
      "uuidgen",
      "uuid_create",
      "uuid_from_string",
      "semget",
      "semop",
      "semctl",
      "ftok",
      "exit",
      "_tzset",
      "tzset",
      "nice",
      "clock_getres",
      "clock_gettime",
      "setpriority",
      "getpriority",
      "rtprio",

      "pthread_create",
      "pthread_detach",
      "pthread_self",
      "pthread_yield",
      "pthread_join",
      "pthread_setcancelstate",
      "pthread_attr_setguardsize",
      "pthread_attr_getstack",
      "pthread_getprio",
      "pthread_setprio",
      "pthread_attr_getschedparam",
      "pthread_attr_setschedparam",
      "pthread_setschedparam",
      "pthread_getschedparam",
      "pthread_rwlock_init",
      "pthread_rwlock_destroy",
      "pthread_rwlock_rdlock",
      "pthread_rwlock_tryrdlock",
      "pthread_rwlock_wrlock",
      "pthread_rwlock_trywrlock",
      "pthread_rwlock_unlock",
      "getcontext",
      "setcontext",
      "makecontext",
      "swapcontext",
    };
    utf8::String h;
    for( uintptr_t i = 0; i < sizeof(headers) / sizeof(headers[0]); i++ ){
      if( strcmp(headers[i],"windows.h") == 0 ) continue;
      if( strcmp(headers[i],"winsock.h") == 0 ) continue;
      if( strcmp(headers[i],"winsock2.h") == 0 ) continue;
      if( strcmp(headers[i],"ntstatus.h") == 0 ) continue;
      h +=
        "#if HAVE_" + utf8::String(headers[i]).replaceAll(".","_").replaceAll("/","_").upper()+ "\n"
        "#include <" + utf8::String(headers[i]) + ">\n"
        "#endif\n"
      ;
    }
    utf8::String hw(h +
        "#if HAVE_WINSOCK2_H\n"
        "#include <winsock2.h>\n"
        "#elif HAVE_WINSOCK_H\n"
        "#include <winsock.h>\n"
        "#endif\n"
        "#if HAVE_MSWSOCK_H\n"
        "#include <mswsock.h>\n"
        "#endif\n"
        "#if HAVE_WS2TCPIP_H\n"
        "#include <ws2tcpip.h>\n"
        "#endif\n"
        "#if HAVE_WINDOWS_H\n"
        "#include <windows.h>\n"
        "#endif\n"
        "#if HAVE_TLHELP32_H\n"
        "#include <tlhelp32.h>\n"
        "#endif\n"
        "#if HAVE_IPHLPAPI_H\n"
        "#include <tlhelp32.h>\n"
        "#endif\n"
    );
    for( uintptr_t i = 0; i < sizeof(functions) / sizeof(functions[0]); i++ )
      out << testCxxFuncExists(config,functions[i],tmpCxx,hw);
    
    // detect symbols
    out << "\n// symbols\n";
    static const char * const symbols[] = {
      "be16enc",
      "be32enc",
      "be64enc",
      "be16toh",
      "be32toh",
      "be64toh",
      "le16enc",
      "le32enc",
      "le64enc",
      "le16toh",
      "le32toh",
      "le64toh",
      "be16dec",
      "be32dec",
      "be64dec",
      "htobe16",
      "htobe32",
      "htobe64",
      "le16dec",
      "le32dec",
      "le64dec",
      "htole16",
      "htole32",
      "htole64",
      "_malloc_options",
      "do_cpuid",
      "atomic_fetchadd_32",
    };
    for( uintptr_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++ )
      out << testCxxSymbolExists(config,symbols[i],tmpCxx,hw);

    // detect libraries
    out << "\n// libraries\n";
    utf8::String existsLibraries;
    static struct {
      const char * const library_;
      const char * const symbol_;
      const char * const mod_;
    } libraries[] = {
      { "md", "SHA256_Init", "" },
      { "lzo", "lzo1x_999_compress_level", "" },
      { "lzo2", "lzo1x_999_compress_level", "" },
      { "pthread", "pthread_create", "" },
      { "rtkaio", "rtprio", "" },
      //{ "rpcrt4", "UuidCreate", "long __stdcall ${symbol}(void *)" }
    };
    for( uintptr_t i = 0; i < sizeof(libraries) / sizeof(libraries[0]); i++ )
      out << testCxxLibExists(libraries[i].library_,libraries[i].symbol_,libraries[i].mod_,tmpCxx,&existsLibraries);

    if( !existsLibraries.isNull() )
      out << "\n// ***WARNING*** Don't edit. This is machine generated line. Using libraries. " << "{[NLOSJVNK0SU2UD4CFBWP2UT10H]}: " << existsLibraries << "\n";

    //detect machine byte order
    out <<
      "\n// machine byte order\n"
      "#if HAVE_SYS_ENDIAN_H\n"
      "#include <sys/endian.h>\n"
      "#endif\n"
      "\n"
      "#ifndef LITTLE_ENDIAN\n"
      "#define LITTLE_ENDIAN 1\n"
      "#endif\n"
      "#ifndef BIG_ENDIAN\n"
      "#define BIG_ENDIAN 2\n"
      "#endif\n"
      "#ifndef BYTE_ORDER\n"
      "#define BYTE_ORDER " +
      (utf8::String(testCxxCode(
        config,
        tmpCxx,
        hw,
        "\n"
        "int main(int /*argc*/,char * /*argv*/[])\n"
        "{\n"
        "  union {\n"
        "    unsigned char s[8];\n"
        "    unsigned int u;\n"
        "  };\n"
        "  u = 0x11223344;\n"
        "  return s[0] == 0x44 ? 0 : 1;\n"
        "}\n"
        ) == 0 ? "LITTLE" : "BIG"
      )) +
      "_ENDIAN\n"
      "#endif\n\n"
    ;

    out.removeAfterClose(false);
  }
  return *this;
}
//------------------------------------------------------------------------------
Compiler & Compiler::compile(
  const utf8::String & config,
  const utf8::String & source,
  const utf8::String & object)
{
  utf8::String compilerArgs(compilerArgs_.replaceCaseAll("${source}",anyPathName2HostPathName(source)));
  compilerArgs = compilerArgs.replaceCaseAll("${object}","\"" + anyPathName2HostPathName(object) + "\"");
  utf8::String value;
  for( uintptr_t i = 0; i < enumStringParts(includeDirectories_); i++ ){
    utf8::String v(anyPathName2HostPathName(stringPartByNo(includeDirectories_,i)));
    if( v.isNull() ) continue;
    value += (value.isNull() ? "\"-I" : " \"-I") + excludeTrailingPathDelimiter(v) + "\"";
  }
  compilerArgs = compilerArgs.replaceCaseAll("${include_directories}",value);
  ExecuteProcessParameters params;
  params.name_ = compiler_;
  params.args_ = compilerArgs;
  params.env_ = compilerEnv_;
  params.wait_ = true;
  AsyncFile testStderr(changeFileExt(source,".err"));
  params.stderr_ = params.stdout_ = testStderr.createIfNotExist(true).removeAfterClose(!keepStderr_).open().descriptor_;
  //testStderr.seek(testStderr.size());
  testStderr.resize(0);
  testStderr << params.name_ + " " + params.args_ + "\n";
  pid_t exitCode = execute(params);
  if( exitCode != 0 )
    newObjectV1C2<Exception>(exitCode,__PRETTY_FUNCTION__)->throwSP();
  testStderr.removeAfterClose(true);
  return *this;
}
//------------------------------------------------------------------------------
Compiler & Compiler::link(
  const utf8::String & module,
  const utf8::String & objects,
  bool dlm)
{
  utf8::String m(module);
#if defined(__WIN32__) || defined(__WIN64__)
  m = module + ".dll";
#else
  m = "lib" + module + ".so";
#endif
  utf8::String linkerArgs(linkerArgs_.replaceCaseAll("${object}",objects));
  linkerArgs = linkerArgs.replaceCaseAll("${executable}",m);
  utf8::String libs;
  bool isGNU = type_.casecompare("gnu") == 0;
  for( uintptr_t k = enumStringParts(libraries_,",",false), i = 0; i < k; i++ ){
    utf8::String s(stringPartByNo(libraries_,i,",",false));
    libs += libs.isNull() ? "" : " ";
    if( isGNU ){
      libs += "\"-l" + s + "\"";
    }
    else if( type_.casecompare("msvc") == 0 || type_.casecompare("intel") == 0 ){
      libs += "\"" + s.right(4).casecompare(".lib") == 0 ? s : s + ".lib" + "\"";
    }
    else {
      libs += s;
    }
  }
  linkerArgs = linkerArgs.replaceCaseAll("${libraries}",libs);
  libs.resize(0);
  for( uintptr_t k = enumStringParts(libDirectories_,",",false), i = 0; i < k; i++ ){
    utf8::String s(stringPartByNo(libDirectories_,i,",",false));
    libs += (libs.isNull() ? "-L" : " -L") + excludeTrailingPathDelimiter(anyPathName2HostPathName(s));
  }
  linkerArgs = linkerArgs.replaceCaseAll("${lib_directories}",libs);
  if( dlm ){
    linkerArgs = 
      linkerDlArgs_.replaceCaseAll("${implib}",
        changeFileExt(
          isGNU ? "lib" + m : m,
          isGNU ?
#if defined(__WIN32__) || defined(__WIN64__)
            ".dll.a"
#else
            ".a"
#endif
            : ".lib"
        )
      ).replaceCaseAll("${def}",changeFileExt(m,".def"))
      + " " + linkerArgs
    ;
  }
  ExecuteProcessParameters params;
  params.name_ = linker_;
  params.args_ = linkerArgs;
  params.env_ = compilerEnv_;
  params.wait_ = true;
  AsyncFile testStderr(changeFileExt(module,".err"));
  params.stderr_ = params.stdout_ = testStderr.createIfNotExist(true).removeAfterClose(!keepStderr_).open().descriptor_;
  testStderr << params.name_ + " " + params.args_ + "\n";
  pid_t exitCode = execute(params);
  if( exitCode != 0 )
    newObjectV1C2<Exception>(exitCode,__PRETTY_FUNCTION__)->throwSP();
  testStderr.removeAfterClose(true);
  return *this;
}
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
