/*-
 * Copyright 2008 Guram Dukashvili
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
  utf8::String defaultCompiler(config->text("default_cxx_compiler","auto"));
  bool autoCompiler = defaultCompiler.strcasecmp("auto") == 0, f = false;
  for( uintptr_t i = 0; i < config->section("cxx_compilers").sectionCount(); i++ ){
    utf8::String sectionPath("cxx_compilers." + (autoCompiler ? config->section("cxx_compilers").section(i).name() : defaultCompiler));
    utf8::String envSectionPath(sectionPath + ".environment");
    Array<utf8::String> env;
    for( uintptr_t j = 0; j < config->sectionByPath(envSectionPath).valueCount(); j++ ){
      utf8::String key, value(config->sectionByPath(envSectionPath).value(j,&key)), e(getEnv(key));
      env.add(key + "=" + value + (e.isNull() ? utf8::String() : ";" + e));
    }
    utf8::String compiler(config->textByPath(sectionPath + ".compiler"));
    AsyncFile tmpCxx(anyPathName2HostPathName(getTempPath() + getTempFileName("cxx")));
    tmpCxx.createIfNotExist(true).open();
    utf8::String compilerArgs(config->textByPath(sectionPath + ".compiler_args")), compilerArgs2(compilerArgs);
    compilerArgs = compilerArgs.replaceCaseAll("${source}",tmpCxx.fileName());
    utf8::String object(changeFileExt(tmpCxx.fileName(),".o"));
    compilerArgs = compilerArgs.replaceCaseAll("${object}",object);
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
    pid_t exitCode = execute(compiler,compilerArgs,&env,true,true,true);
    if( exitCode == 0 && stat(object) ){ // detected
      utf8::String compiler(config->textByPath(sectionPath + ".compiler"));
      type_ = config->textByPath(sectionPath + ".type");
      compiler_ = compiler;
      compilerArgs_ = compilerArgs2;
      compilerEnv_ = env;
      linker_ = config->textByPath(sectionPath + ".linker");
      linkerArgs_ = config->textByPath(sectionPath + ".linker_args");
      f = true;
      break;
    }
    if( !autoCompiler ) break;
  }
  if( !f )
    newObjectV1C2<Exception>(ENOENT,__PRETTY_FUNCTION__)->throwSP();
  keepStderr_ = config->value("keep_compiler_stderr",false);
  return *this;
}
//------------------------------------------------------------------------------
bool Compiler::testCxx(const utf8::String & config,const utf8::String & test,const utf8::String & tmpCxx)
{
  AsyncFile file(tmpCxx);
  file.createIfNotExist(true);
  utf8::String compilerArgs(compilerArgs_.replaceCaseAll("${source}",tmpCxx));
  utf8::String object(changeFileExt(tmpCxx,".o"));
  compilerArgs = compilerArgs.replaceCaseAll("${object}",object);
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
  pid_t exitCode = execute(params);
  testStderr << "\n";
  return exitCode == 0 && stat(object);
}
//------------------------------------------------------------------------------
intptr_t Compiler::testRunCxx(const utf8::String & config,const utf8::String & test,const utf8::String & tmpCxx)
{
  AsyncFile file(tmpCxx);
  file.createIfNotExist(true);
  utf8::String compilerArgs;
  compilerArgs = compilerArgs_.replaceCaseAll("${source}",tmpCxx);
  utf8::String object(changeFileExt(tmpCxx,".o"));
  compilerArgs = compilerArgs.replaceCaseAll("${object}",object);
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
  pid_t exitCode = execute(params);
  testStderr << "\n";
  if( exitCode == 0 && stat(object) ){
    utf8::String executable(changeFileExt(tmpCxx,".exe"));
    AutoFileRemove afr3(executable);
    utf8::String linkerArgs(linkerArgs_.replaceCaseAll("${object}",object));
    linkerArgs = linkerArgs.replaceCaseAll("${executable}",executable);
    params.name_ = linker_;
    params.args_ = linkerArgs;
    exitCode = execute(params);
    if( exitCode == 0 && stat(executable) )
      return execute(executable,utf8::String(),&compilerEnv_,true);
  }
  return -1;
}
//------------------------------------------------------------------------------
intptr_t Compiler::testCxxType(const utf8::String & config,const utf8::String & type,const utf8::String & tmpCxx)
{
  intptr_t size = testRunCxx(
    config,
    "#if HAVE_STDINT_H\n"
    "#include <stdint.h>\n"
    "#endif\n"
    "#if HAVE_INTTYPES_H\n"
    "#include <inttypes.h>\n"
    "#endif\n"
    "#if HAVE_SYS_TYPES_H\n"
    "#include <sys/types.h>\n"
    "#endif\n"
    "\n"
    "extern \"C\" int main(int /*argc*/,char * /*argv*/[])\n"
    "{\n"
    "  return (int) sizeof(" + type + ");\n"
    "}\n",
    tmpCxx
  );
  return size >= 0 ? size : 0;
}
//------------------------------------------------------------------------------
Compiler & Compiler::test(const utf8::String & config)
{
  /*if( !stat(config) )*/{
    AsyncFile out(config);
    out.createIfNotExist(true);
    utf8::String tmpCxx(anyPathName2HostPathName(getTempPath() + getTempFileName("cxx")));
    out.open();
    // detect includes
    out << "// includes\n";
    out << "#define HAVE_STDDEF_H " << (testCxx(config,"#include <stddef.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_STDINT_H " << (testCxx(config,"#include <stdint.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_INTTYPES_H " << (testCxx(config,"#include <inttypes.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_TYPES_H " << (testCxx(config,"#include <sys/types.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_PARAM_H " << (testCxx(config,"#include <sys/param.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_IPC_H " << (testCxx(config,"#include <sys/ipc.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_SEM_H " << (testCxx(config,"#include <sys/sem.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_STAT_H " << (testCxx(config,"#include <sys/stat.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_UTSNAME_H " << (testCxx(config,"#include <sys/utsname.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_SOCKET_H " << (testCxx(config,"#include <sys/socket.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_UIO_H " << (testCxx(config,"#include <sys/uio.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_MMAN_H " << (testCxx(config,"#include <sys/mman.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_EVENT_H " << (testCxx(config,"#include <sys/event.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_ENDIAN_H " << (testCxx(config,"#include <sys/endian.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_TIME_H " << (testCxx(config,"#include <sys/time.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_TIMEB_H " << (testCxx(config,"#include <sys/timeb.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_UTIME_H " << (testCxx(config,"#include <sys/utime.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_RESOURCE_H " << (testCxx(config,"#include <sys/resource.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_RTPRIO_H " << (testCxx(config,"#include <sys/rtprio.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_UUID_H " << (testCxx(config,"#include <sys/uuid.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_EVENT_H " << (testCxx(config,"#include <sys/event.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_IOCTL_H " << (testCxx(config,"#include <sys/ioctl.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_SYSCTL_H " << (testCxx(config,"#include <sys/sysctl.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYS_EPOLL_H " << (testCxx(config,"#include <sys/epoll.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_MACHINE_ATOMIC_H " << (testCxx(config,"#include <machine/atomic.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_MACHINE_CPUFUNC_H " << (testCxx(config,"#include <machine/cpufunc.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_MACHINE_SPECIALREG_H " << (testCxx(config,"#include <machine/specialreg.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_ARPA_INET_H " << (testCxx(config,"#include <arpa/inet.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NET_IF_H " << (testCxx(config,"#include <net/if.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NET_IF_DL_H " << (testCxx(config,"#include <net/if_dl.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NET_IF_TYPES_H " << (testCxx(config,"#include <net/if_types.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NET_ROUTE_H " << (testCxx(config,"#include <net/route.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NET_ETHERNET_H " << (testCxx(config,"#include <net/ethernet.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NETINET_IN_H " << (testCxx(config,"#include <netinet/in.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NETINET_IN_SYSTM_H " << (testCxx(config,"#include <netinet/in_systm.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NETINET_IP_H " << (testCxx(config,"#include <netinet/ip.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NETINET_TCP_H " << (testCxx(config,"#include <netinet/tcp.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_AIO_H " << (testCxx(config,"#include <sys/aio.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_NETDB_H " << (testCxx(config,"#include <netdb.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_MEMORY_H " << (testCxx(config,"#include <memory.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_STDIO_H " << (testCxx(config,"#include <stdio.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_STDLIB_H " << (testCxx(config,"#include <stdlib.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_ASSERT_H " << (testCxx(config,"#include <assert.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_STRING_H " << (testCxx(config,"#include <string.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_ERR_H " << (testCxx(config,"#include <err.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_ERRNO_H " << (testCxx(config,"#include <errno.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYSLOG_H " << (testCxx(config,"#include <syslog.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_EXECINFO_H " << (testCxx(config,"#include <execinfo.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_STDARG_H " << (testCxx(config,"#include <stdarg.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SYSEXITS_H " << (testCxx(config,"#include <sysexits.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SHA256_H " << (testCxx(config,"#include <sha256.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_PROCESS_H " << (testCxx(config,"#include <process.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_DLFCN_H " << (testCxx(config,"#include <dlfcn.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_PTHREAD_H " << (testCxx(config,"#include <pthread.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SIGNAL_H " << (testCxx(config,"#include <signal.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_FCNTL_H " << (testCxx(config,"#include <fcntl.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_UNISTD_H " << (testCxx(config,"#include <unistd.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SEMAPHORE_H " << (testCxx(config,"#include <semaphore.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_CTYPE_H " << (testCxx(config,"#include <ctype.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_PWD_H " << (testCxx(config,"#include <pwd.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_GRP_H " << (testCxx(config,"#include <grp.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_IO_H " << (testCxx(config,"#include <io.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_PATHS_H " << (testCxx(config,"#include <paths.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_TIME_H " << (testCxx(config,"#include <time.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_SCHED_H " << (testCxx(config,"#include <sched.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_UTIME_H " << (testCxx(config,"#include <utime.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_UUID_H " << (testCxx(config,"#include <uuid.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_MATH_H " << (testCxx(config,"#include <math.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_FLOAT_H " << (testCxx(config,"#include <float.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_UCONTEXT_H " << (testCxx(config,"#include <ucontext.h>\n",tmpCxx) ? "1" : "0") << "\n";
    out << "#define HAVE_DIRENT_H " << (testCxx(config,"#include <dirent.h>\n",tmpCxx) ? "1" : "0") << "\n";
    // detect type exists & sizes
    out << "\n// types\n";
    out << "#define SIZEOF_CHAR_T " << utf8::int2Str(testCxxType(config,"char",tmpCxx)) << "\n" <<
           "#define HAVE_CHAR_T (SIZEOF_CHAR_T > 0)\n";

  }
  return *this;
}
//------------------------------------------------------------------------------
} // namespace kvm
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
