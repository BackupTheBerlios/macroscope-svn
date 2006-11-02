/**
  Utils to make debugging running application under Windows
  Copyright 2003 by Roger Rene Kommer (kommer@artefaktur.com) 

  dbgutils The sources either can be in a shared library (.dll) or
  has to be linked directly to the executable. 
  Compiling dbgutils as static library (.lib) won't work!

  Requirements dbghelp.h .dll .lib from microsoft
*/



/** 
  @mainpage PDBUtils Documentation 
  Copyright 2003 by Roger Rene Kommer (kommer@artefaktur.com)<p>

  @section intro What is PDBUtils
  
  PDBUtils add debugging and reflection mechanism to any VC 6 program.
  
  You (or another developer) had coded an application but you had too less time
  to test the application with all needed unit tests, stress tests, integration
  tests and so on, because your customer urgently need your application.<br>
  So your application installed on the customer production machine
  crashes occasionally (once the day, the week, the month) and you have no idea
  what iss going wrong, because on customers machine no debugger is installed.<p>

  PDBUtils is to designed to help in such a situation. Just linking your application
  with pdbutils.dll and insert one call into your main method you can receive
  a binary or 'source-level' text post mortem dump of the faulting application with
  all information available you normally also receive when your application was run
  under a debugger.<p>
  
  So you hopefully be able to identify the faulty code, fix it, provide
  a more stable version of your application and make your customer happy.
  
  @subsection debugging_features Debugging Features
  In case the application tends to crash DDBUtils can be used to generate binary and text dumps
  of the current scope for all threads with all current functions calls, parameter, local and global data types.
  <p>
  The text dump is a text file with a listing of all threads with functions calls, parameter types an values
  and the local variables of the functions.<p>

  The binary dump creates a "CrashDump" which can be viewed with WinDbg, a free Source GUI-Debugger from Microsoft.
  <p>

  @subsection throwable Enrich Exceptions classes with stack traces
    On Java an exception can provide the call stack from the point where
    the exception was thrown. With PDBUtils this is also available for 
    VC 6/7 Windows applications. It will be explained in @ref usage_secondgrade.

  @subsection premortem_debugging Create snapshot from application
    The post mortem debugging dumps are also available at normal runtime.
    It is possible to dump the state a single thread or all threads into readable
    text file. This makes sense, if the application may not crashed yet, but
    some assertion failed or run in a dead lock situtation.

  @subsection reflection_mechanism Using reflection
    On runtime you can receive reflection information on every valid memory
    address including nested user defined types and dynamic structures (using
    pointers). For example can this reflection mechanism can used to render any data
    structure into log-files.
      
  @section content Content
  <ul>
@if pdbutils_lgpl
      <li> @ref license </li>
      <li> @ref support </li>
@endif

    -<li> @ref requirements</li>
    <li> @ref usage</li>
    <ul>
      <li> @ref usage_firstgrade</li>
      <li> @ref usage_secondgrade</li>
      <li> @ref usage_thirdgrade</li>
      <li> @ref usage_fourthgrade</li>
    </ul>
  </ul>
    <b>Reference:</b>
  <ul>
      <li> <a class="el" HREF="modules.html"> Modules</A></li>
      <li> <a class="el" HREF="namespaces.html"> Namespace</A></li>
    </ul>
  */

/**
  @if pdbutils_lgpl
  @page license License
  <pre>
  pdbutils 

  Copyright (C) 2003 by Roger Rene Kommer

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
See also:
http://www.gnu.org/copyleft/lesser.html
</pre>
  See Next: @ref support

  @endif
*/
/**
  @if pdbutils_lgpl
  @page support Support and Dowloads
  
  You can obtain support at my project page for ACDK (which is not directly related
  to pdbutils):<br>
  http://sourceforge.net/projects/acdk<br>

  You can download the most current version on:<br>
  http://sourceforge.net/project/showfiles.php?group_id=15364&release_id=186882

  I'm also reachable via email: kommer@artefaktur.com<p>
  @section contributions Contributions
    The major inspiration for this library comes from Matt Pietrek MSDN article
    "Improved Error Reporting with DBGHELP 5.1 APIs".<p>
    You can read it online here:<br>

    http://msdn.microsoft.com/msdnmag/issues/02/03/hood/default.aspx<p>
    <p>
  See Next: @ref requirements
  @endif
*/

/**
  @page requirements Requirements
  @section dbghelp DbgHelp Library
    
    Before you can compile pdutils you need the DbgHelp files provided
    by Microsoft. The dbhhelp are included in the Debugging Tools for Windows:<br>
    
    http://www.microsoft.com/whdc/ddk/debugging/default.mspx<p>

    Install WinDbg with complete SDK files using custom settings.
    Finally you need following files from the Debugging Tools for Windows:
    - Debugging Tools for Windows\dbghelp.dll
    - Debugging Tools for Windows\sdk\inc\dbghelp.h
    - Debugging Tools for Windows\sdk\lib\dbghelp.lib<p>

    - Copy the dbghelp.h and dbghelp.lib into the pdbutls\ext directory.
    - Copy the dbghelp.dll into your binary directory of the application
      which uses pdutils.

  @section platform Platforms
    - Windows XP Profession: Works with VC 6 and VC 7/.NET
    - Windows XP Home: Not tested
    - Windows 2003 Server: Using VC7/.NET on Windows 2003 works.<br>
    - Windows 2000 Server/Workstation: 
      Windows 2000 also provides a dbghelp.dll, but in a wrong version!<br>
      Unfortunately  it doesn't work to copy a new dbghelp.dll into the 
      windows\\system32 directory because the self-healing mechanism of Windows 2000.<p>
      The only solution I know is to copy the dbghelp.dll of Windows XP into the same directory of 
      the created application which uses the pdbutils.dll.
      
    - Windows NT 4, Windows 95, Windows 98: Not tested<br>
      Microsoft provides a dbghelp.dll for these older version of their OS, but 
      unfortunatelly only in a outdated version.
      But using the dbghelp of Windows XP and copy it into the system32 directory
      seems to be working.

  @section compiler Compiler Settings
    To use PDBUtils the application have to be compiled with debugging information.
  @section Installation Installation
    
  @subsection pdb_files .pdb files
    The VC compiler stores the debugging information in a separate file with the file suffix .pdb.<br>
    If the application should be installed outside the compile directory the corresponding .pdb file 
    has to be copied into the same directory of the executable file.<p>
    (For more details please refer to @ref usage).
    
  @subsection windbg WinDbg
    Use WinDbg to view the binary dumps.<br>
    WinDbg is included in the "Debugging Tools for Windows" and can be downloaded from Microsoft for free:
    http://www.microsoft.com/whdc/ddk/debugging/default.mspx<p>

    Although it is possible to load binary dump with VC 7/.NET (via File / Open Solution / Dump Files)
    I haven't found any way to view the dump with the call stack, local variables, etc.<p>

  See Next: @ref usage
*/
/**
  @page usage Usage
  @section compiling Compiling with pdbutils.
    You need to adjust the compiler setting for your executable:
    - In the C/C++ settings set debugging format Program Database
    - In the Linker settings select usage of generated program database file 
      (which generates the executable.pdb).
    - In the Linker settings select Microsoft debug format.

  
  
  @section deployment Deployment
    To deploy your executable using the pdbutils you have
    to copy:
    - dbghelp.dll (this is not needed on Windows XP and Windows 2003)
    - pdbutils.dll
    - your_dlls.dll (optional)
    - your_dlls.pdb (optional)
    - your_application.exe
    - your_application.pdb
    - vc60.pdb (see below)<br>

    The file your_application.pdb has to reside in the same directory
    as your_application.exe.<p>
    
    Using Visual Studio 6 unfortunately pdbutils doesn't need only the executable.pdb but also
    the file vc60.pdb. This file is generated in the object directory (by default 'Debug').<p>
    There 3 ways to deal with this vc60.pdb:

    -# Copy vc6.pdb in the same directory of your_application.exe.<br>
      Of course this has the disadvantage, that only one executable can make use
      of pdbutils.
    -# Replicate the identical directory structure (including drive letter) on your target machine
       including the directory of your_application.exe and Debug\vc60.pdb.<br>
    -# Create in the directory of your your_application.exe a directory named
       your_application_pdbs and copy all pdb's (your_application.pdb, vc60.pdb and dll pdb's) in this
       directory.


  @section coding Coding
  See more you to integrate pdbutils in your source code:
  <ul>
  <li> @ref usage_firstgrade </li>
  <li> @ref usage_secondgrade </li>
  <li> @ref usage_thirdgrade </li>
  <li> @ref usage_fourthgrade </li>
  </ul>
*/
/**
  @page usage_firstgrade First-Grade Usage: Making CrashDumps
    The most simple way to use pdbutils in your project is to include following code 
    into your main.cpp of your application:
    @code
      // your headers
      #include <pdbutils.h>
      // more code

      int main(int argc, char* argv[])
      {
        pdbutils::installCrashDumpHook();
        // your code here
      }
    @endcode
    In the case your application will crash (because invalid pointer access, uncatched exceptions, etc.)
    the pdbutils will create in the directory of the binaries following files:
    <pre>
      my_executable_name_2003_09_09_13_59_36.btc
      my_executable_name_2003_09_09_13_59_36.dmp
    </pre>
    The .btc file is just a plain text file with all stacktraces of all threads
    including the local variables.<p>
    The .dmp file is a binary dump file, which can be loaded with WinDbg.
    @section usage_firstgrade_btc_format The .btc format
      Following sample:

      Exception information:
      @code
Exception code: C0000005
ACCESS_VIOLATION// reason because the application crashes 
Fault address:  00401484 1 1156 C:\d\artefaktur\pdbutils\bin\pdbutils_Test.exe

Registers:// if you are an assembler freak it may useable for you
EAX: 0
EBX: 0
ECX: 6
EDX: 3745256
ESI: 26803832
EDI: 26803572
CS:EIP: 27:4199556
SS:ESP: 26803436:26803584
DS: 35
ES: 35
FS: 56
GS: 0
Flags: 66195
==================================================================
Crashing Thread: 
------------------------------------------------------------------
C:\d\artefaktur\pdbutils\bin\pdbutils_Test.exe:
c:\d\artefaktur\pdbutils\tests\src\pdbutils_test.cpp(111):
reportStack(int tc = 6)
{
                  
  <Unknown>* iptr(0x00000000) <invalid pointer>; // the underlying code is null pointer dereferencing
}
// etc.
    @endcode


    The Threads:
    @code

==================================================================
ThreadID: 1936  ThreadHandle: 00000058 // thread identifier
-------------------------------------------------------------------
C:\d\artefaktur\pdbutils\bin\pdbutils_Test.exe: // start of StackFrame, see below 
C:\d\artefaktur\pdbutils\tests\src\pdbutils_Test.cpp(154):
foo2(TestStruct* s(0x00CFFE4C)   TestStruct  = {
    @endcode
      etc...<p>


    The StackFrame:
    from the topmost stack frame (inner call) to main or thread starting function.
    @code
---------------------------------------------------------------
C:\d\artefaktur\pdbutils\bin\pdbutils_Test.exe: // module name (dll or exe)
C:\d\artefaktur\pdbutils\tests\src\pdbutils_Test.cpp(162): // source code position
foo(int tc = 6)// function including parameters
{
// start of local variables
  TestStruct ts = {
    char* _text = (0x004BAA50) "Just A Test";
    int _ivar = 41;
    int _ivarArray[3] = {
      [0] int  = 1;
      [1] int  = 2;
      [2] int  = 3;
    };
    SubStruct _substruct = {
      int _subI = 2;
    };
    TestStruct::MyUnion _myUnion = {
      int i = 41;
      <Unknown>* c(0x00000029) <invalid pointer>;
    };
    int _myEnum = 1;
    <Unknown>* _thisPtr(0x00000000) <invalid pointer>;
  }
} // end of function
// next stack frame
-------------------------------------------------------------------
C:\d\artefaktur\pdbutils\bin\pdbutils_Test.exe:
C:\d\artefaktur\pdbutils\tests\src\pdbutils_Test.cpp(171):
bar(int i = 42, char* text = (0x0055A04C) "This is Text", int tc = 1)
    @endcode
      etc...

    See next: @ref usage_secondgrade
 */
/**
  @page usage_secondgrade Second-Grade Usage: Creating BackTraces
  
  Beside Creating Dumps in case of a crash of the application pdbutils is 
  also able to create text base information of the current state of an application.

  @section create_dump Creating Dumps manually

  With the method pdbutils::createDump you can create dump manually.
  @code
  void pdbutils::createDump(const char *filename=0, int crashdumpflags = DbgCrashDumpCreateAllDump, int skipCount=1)
  @endcode
  With the parameter crashdumpflags - a combination DbgCrashDumpOptions bits - of you can 
  control what information you want to dump.<p>

  For example by default the global Variable will not be dumped (If you link C++ standard library or MFC library,
  there will be really many global variables), but with the flag DbgFrameGetGlobals you can also dump these 
  variables.<p>

  The variable skipCount is to control the number of frames from the top, which should not be dumped.
  This is useful if you want to create a dump inside the constructor of an exception and you don't want
  to display the constructor method itself in the dump.

  Please refer to the API documentation for usage of the parameters.


  @section create_backtrace Creating BackTrace
    In Java you have the possibility to receive a BackTrace for the throwing point of an exception.
    With pdbutils::getBackTrace you can receive the same information.<p>

    For performance reason it is reasonably not to call pdbutils::getBackTrace() in every exception
    class constructor, but only save the FrameAddresses of the current call stack in the constructor
    of the exception class with the call pdbutils::getProgramCounters.

    Here some sample code:
    @code
    #define MAX_CALLSTACK 32
    class MyException
    {
      pdbutils::FrameAddress _frameAddresses[MAX_CALLSTACK];
      int _frameAddressCount;
    public:
      MyException()
      , _frameAddressCount(0)
      {
        memset(_frameAddresses, 0, sizeof(_frameAddresses));
        // take snapshot of the current thread call stack
        _frameAddressCount = pdbutils::getProgramCounters(_frameAddresses, MAX_CALLSTACK);
      }

      // replace with your favorite String class 
      String getBackTrace() const
      {
       // returns declaration (without variable content) of the stack
       // skip topmost stack frames (which is MyException::MyException())
        return pdbutils::renderBackTrace(_frameAddresses + 1, _frameAddressCount - 1, 
                                         pdbutils::DbgFrameGetFuncName |
                                         pdbutils::DbgFrameGetLibary |
                                         pdbutils::DbgFrameGetSourcePos |
                                         pdbutils::DbgFrameGetArguments
                                           );
      }
    };
    
    @endcode

  See next: @ref usage_thirdgrade
*/
/**
  @page usage_thirdgrade Third-Grade Usage: Customizing Type Rendering
  For better parsing the DbgType hooks can rearrange 
           a parsed DbgType.<p>

           For example a std::vector with T* _First and T* _Last 
           as range description only one the pointer of _First will
           point to a valid element.<p>
           @code
           std::vector<int> = {
            int* _First = 42;
            int* _Last = <invalid pointer>;
            int* _End = <invalid pointer>;
           }
           @endcode

           The hooking function can rearange and fetch more sub variables.
           In the example of std::vector all pointers between >= _First and
           < _Last and represent std::vector as 'normal' vector with size
           and all elements.
           @code 
           !std::vector<int> = {
             int size = 3;
             int capacity = 11;
             int data[] = {
              [0] = 42;
              [1] = 43;
              [2] = 44;
            };
          @endcode
          Another sample of the DbgHook is to simplify types. std::string is
          normally an ugly template named :
          @code 
                  std::basic_string<char,std::char_traits<char>,std::allocator<char> >
                    {  std::allocator<char> allocator = {  };
                  char* _Ptr = (0x00393119) "of";
                  int _Len = 2;
                  int _Res = 31;
                  };
          @endcode
          The simplified version is:
          @code
          !std::string  = (0x00393119) "of";
          @endcode

          If a hook change something in the DbgType it should rename the
          type with a leading '!'.<p>

          Please refer to the examples in std_dbgtypes.cpp and mfc_dbgtypes.cpp as sample
          how to program own hooks.<p>
    See next: @ref usage_fourthgrade
*/
/**
  @page usage_fourthgrade Fourth-Grade Usage: Using pdbutils as reflection mechanism
        ... to be continued ...
*/
    


#ifndef win32_pdb_utils_h
#define win32_pdb_utils_h

#include <string.h> 
#include <new.h>

#if defined(WIN32_DPB_UTILS_EXPORTS)
# define _IN_DBGUTILS 1
#endif

#if !defined(_STATIC_DBGUTILS)
#if defined(_IN_DBGUTILS)
# define DBGUTILS_EXPORT __declspec(dllexport)
# define  DBGUTILS_EXPORT_TEMPLATE  extern
#else
# define DBGUTILS_EXPORT __declspec(dllimport)
# define  DBGUTILS_EXPORT_TEMPLATE  
#endif
#else
#define DBGUTILS_EXPORT
#define  DBGUTILS_EXPORT_TEMPLATE  
#endif

#pragma warning (disable : 4231)

#include "DbgString.h"
#include "DbgVector.h"
#include "DbgType.h"





namespace pdbutils {

/**
 @defgroup public_method_interface Public Method Interface
           The public methods to generate Dumps and retrieve
           reflection information.
 */
/*@{*/

/**
  Bit combination flags used to query and print 
  BackTraces, DbgFrames and DbgTypes for Argument, Locals and Global variables, 
*/
enum DbgGetFrameFlags
{
  /**
    Query the name of the function of the current Frame
  */
  DbgFrameGetFuncName       = 0x00010000,
  /**
    get the library name the function define
  */
  DbgFrameGetLibary         = 0x00020000,
  /**
    get the source filename and source line number
  */
  DbgFrameGetSourcePos      = 0x00040000,
  /**
    retrieve also the values
  */
  DbgFrameGetValues         = 0x00080000,
  /**
    get Parameter arguments
    Combine it with DbgFrameGetValues to retive the parameter values
  */
  DbgFrameGetArguments      = 0x00100000,
  /**
    return the local variables
  */
  DbgFrameGetLocals         = 0x00200000,
  /**
    return global variables.
    It is not recomented to retrive local frame with this flag.
    You can use pdbutils::getGlobalFrame() to retrieve all global variables.
  */
  DbgFrameGetGlobals        = 0x00400000,
  /**
    In the first byte the count of recursive following
    pointers is encoded.
  */
  DbgMaxPointerMask         = 0x000000FF,
  /**
    used pdbutils::DbgType::toString().
    Maximum type recursion.
  */
  DbgMaxRecursionMask       = 0x000000FF,

  DbgFrameGetAll = DbgFrameGetFuncName | DbgFrameGetLibary | DbgFrameGetSourcePos | DbgFrameGetArguments | DbgFrameGetValues | DbgFrameGetLocals,
  DgbFrameGetFormalBackTrace = DbgFrameGetFuncName | DbgFrameGetArguments,
  DgbFrameGetTrace = DbgFrameGetFuncName | DbgFrameGetLibary | DbgFrameGetSourcePos | DbgFrameGetArguments | DbgFrameGetValues | DbgFrameGetLocals,
  /**
    Create binary (mini-) dump readable by WinDbg
    Used when createing dump.
  */
  DbgCrashDumpCreateBinaryDump = 0x01000000,
  /**
    Create text dump.
    Used when createing dump.
  */
  DbgCrashDumpCreateTextDump   = 0x02000000,
  /**
    Create text dump with all threads.
    Only used in combination with DbgCrashDumpCreateTextDump.
  */
  DbgCrashDumpAllThreads       = 0x04000000,
  DbgCrashDumpCreateAllDump = DbgFrameGetAll | DbgCrashDumpAllThreads | DbgCrashDumpCreateTextDump | DbgCrashDumpCreateBinaryDump,
  DbgCrashDumpCreateAllWithGlobalsDump = DbgCrashDumpCreateAllDump | DbgFrameGetGlobals,
  /**
    @internal used for internal rendering
  */
  DbgPrintInPointerRef         = 0x10000000,
  /**
    @internal used for internal rendering
  */
  DbgPrintContinueLine         = 0x20000000
};

/** @} */

/**
 @defgroup public_classes Public Class Interfaces
           The public classes represents StackFrames and types
 */
/*@{*/


/**
  A DbgFrame represents one statck frame: either a method call or global variable
*/
class /*DBGUTILS_EXPORT*/ DbgFrame
{
public:

  /** program counter */
  FrameAddress frame;
  /** combination of DbgGetFrameFlags */
  intptr_t queryFlags;
  /** hold the function name without return type or parameters */
  DbgString functionName;
  /** hold the library name (DLL or EXE) */
  DbgString libraryName;
  /** hold the source file name */
  DbgString sourceFileName;
  /** hold the source line */
  intptr_t sourceFileLine;
  /** internal variable */
  __int64 displacement;
  /** hold the variables of the current frame (method) */
  DbgTypeArray params;
  /** hold the local defined variables of the current method frame */
  DbgTypeArray locals;
  /** hold the global defined variables if this frame is the global frame */
  DbgTypeArray globals;
  
  DbgFrame()
  : sourceFileLine(-1)
  , displacement(0)
  {
    frame.offset = 0;
    frame.address = 0;
  }
  bool operator<(const DbgFrame& other) const { return frame.address < frame.address; }
  bool operator==(const DbgFrame& other) const { return frame.offset == other.frame.offset &&
                                                        frame.address == other.frame.address; }
  /**
    @param printflags combination of DbgGetFrameFlags flags
  */
  //DbgString toString(int printflags) const;
  void toString(DbgStream& out, intptr_t printflags) const;

  /**
    Try to find the type of an address.
    The method searches in current for parameters or local 
    defined variables.
    The frame must be requested with correspoinding flags
    (DbgFrameGetLocals | DbgFrameGetArguments | DbgFrameGetValues)
    before.
    @param addr the variable memory address
    @param type outparam will be filled if varialbe was found
    @return true if an variable on given addr was found the the param type was set
  */
  bool findElementForAddress(void* addr, DbgType& type) const;

};

/** @} */


/**
 @addtogroup public_method_interface
 */
/*@{*/

/**
  get the Program Counters for a thread.
  @param frames inout buffer for the FrameAddress. 
         has to be at least storage for FrameAddress[vecsize] 
  @param frameCount number of maximum FrameAddress to fill.
  @param threadHandle HANDLE to thread. If threadHandle == 0 get frames of 
         current thread
  @return number of returned FrameAddresses in param frames
*/
intptr_t /*DBGUTILS_EXPORT*/ getProgramCounters(FrameAddress* frames, intptr_t frameCount, void* threadHandle = 0);

  /**
    fill the frames on given vectors
    @param frameAddress of the frame you want to receive
    @param frame return value.
    @param flags combination of DbgGetFrameFlags
  */
void /*DBGUTILS_EXPORT*/ getFrame(const FrameAddress& frameAddress, DbgFrame& frame, intptr_t flags);

/**
  Search in current thread for reachable params and local 
  variables for given addr.
  @param addr a memory address
  @return if no DbgType was found the DbgType::type is DbgType::Unset
*/
DbgType /*DBGUTILS_EXPORT*/ getTypeFromAddress(void* addr);

/**
  return full backtrace (functions, parameter, local variables
  of given thread
  @param skipCount skip the topmost StackFrames
  @param threadHandle a HANDLE of a thread or 0 for current thread
*/
DbgString /*DBGUTILS_EXPORT*/ getFullBackTrace(intptr_t skipCount = 0, void* threadHandle = 0);

/**
  get BackTrace as string
  @param flags combination of DbgGetFrameFlags
  @param skipCount frames to skip. See also @ref usage_secondgrade
  @param threadHandle a HANDLE to the thread which should be received.
         By default this value is 0 and will return the BackTrace of the current
         Thread.
*/
DbgString /*DBGUTILS_EXPORT*/ getBackTrace(intptr_t flags, intptr_t skipCount = 0, void* threadHandle = 0);

/**
  Render for the given frame addresses a BackTrace string
  @param frameAddresses array of FrameAddress
  @param frameCount element count of frameAddresses
  @param flags combination of DbgGetFrameFlags
*/
DbgString /*DBGUTILS_EXPORT*/ renderBackTrace(FrameAddress* frameAddresses, intptr_t frameCount, intptr_t flags);

/**
  return a the frame of global variables
  @param frame the return value
  @param flags combination of DbgGetFrameFlags
*/
void /*DBGUTILS_EXPORT*/ getGlobalFrame(DbgFrame& frame, intptr_t flags);
  /**
    creates a mini crash dump file
    @param filename either a fully qualified file name (extension dmp is recommended)
           or null for a crashdump with the name [date_time]_[modulefile].dmp
    @param crashdumpflags combination of DbgCrashDumpOptions flags
    @param skipCount Number of frames which should be skipped from the top 
           of the stack.
           See also @ref usage_secondgrade
  */
void /*DBGUTILS_EXPORT*/ createDump(const char* filename = 0, intptr_t crashdumpflags = DbgCrashDumpCreateAllDump, intptr_t skipCount = 1);

/**
    Generates binary and/or CrashDump file 
    in case of unexpected program termination
    @param crashdumpflags combination of DbgCrashDumpOptions flags
*/
void /*DBGUTILS_EXPORT*/ installCrashDumpHook(intptr_t crashdumpflags = DbgCrashDumpCreateAllDump);
/**
  removed the previously installed CrashDumpHook
*/
void /*DBGUTILS_EXPORT*/ uninstallCrashDumpHook();
  /**
    Set maximum recursion of pointer resolution
  */
extern intptr_t /*DBGUTILS_EXPORT*/ MaxPointerRecursion;
  
/**
  Try to query a symbol
  @param modBase module frame base 
  @param dwTypeIndex symbol index type of
  @param offset 
  @param size size of type
  @param dbgType out-paramater which will be filled
  @param queryFlags combination of DbgGetFrameFlags
*/
bool /*DBGUTILS_EXPORT*/ querySymbolType(__int64 modBase, uintptr_t dwTypeIndex, intptr_t offset, 
                                    int64_t size, DbgType& dbgType, intptr_t queryFlags);

/*@}*/


/**
 @defgroup parse_hooks DbgType parse hooks 
           
           Please refer to: @ref usage_thirdgrade
 */
/*@{*/

typedef bool (*DebugTypeHandler)(DbgType& dbgType, bool& discarge);

void /*DBGUTILS_EXPORT*/ registerDebugTypePreHandler(DebugTypeHandler handler);
void /*DBGUTILS_EXPORT*/ registerDebugTypePostHandler(DebugTypeHandler handler);

struct RegisterDebugPreTypeHandler
{
  RegisterDebugPreTypeHandler(DebugTypeHandler handler)
  {
    registerDebugTypePreHandler(handler);
  }
};

struct RegisterDebugPostTypeHandler
{
  RegisterDebugPostTypeHandler(DebugTypeHandler handler)
  {
    registerDebugTypePostHandler(handler);
  }
};

/** @} */

} // namespace pdbutils

#endif //win32_pdb_utils_h