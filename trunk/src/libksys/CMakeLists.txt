IF (NOT LIB_MD)
  SET (ADD_SOURCES sha256c.c)
ENDIF (NOT LIB_MD)

IF (NOT LIB_LZO AND NOT LIB_LZO2)
  SET (ADD_SOURCES ${ADD_SOURCES}
    lzo1x_1.c
    lzo1x_1k.c
    lzo1x_1l.c
    lzo1x_1o.c
    lzo1x_9x.c
    lzo1x_d1.c
    lzo1x_d2.c
    lzo1x_d3.c
    lzo1x_o.c
    lzo_init.c
    lzo_crc.c
    lzo_util.c
  )
ENDIF (NOT LIB_LZO AND NOT LIB_LZO2)

IF (CMAKE_COMPILER_IS_GNUCXX)
  SET (ADD_SOURCES ${ADD_SOURCES} execinfo.cpp)
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

ADD_LIBRARY(ksys STATIC
  afd.cpp
  aio.cpp
  archive.cpp
  cgi.cpp
  conf.cpp
  except.cpp
  fetch.cpp
  fiber.cpp
  gd.cpp
  gdchart.cpp
  henum.cpp
  log.cpp
  lzo.cpp
  memmgr.cpp
  mstream.cpp
  mutant.cpp
  mutex.cpp
  object.cpp
  pcap.cpp
  perror.cpp
  profiler.cpp
  rnd.cpp
  service.cpp
  sha.cpp
  shobjs.cpp
  stat.cpp
  stmflt.cpp
  sysutils.cpp
  thread.cpp
  utime.cpp
  xalloc.cpp
  xtime.cpp
  address.cpp
  scktintf.cpp
  client.cpp
  server.cpp
  socket.cpp
  ${ADD_SOURCES}
)

IF (CMAKE_COMPILER_IS_GNUCXX)
  IF (CMAKE_SYSTEM_PROCESSOR MATCHES i386 AND CMAKE_SYSTEM_NAME MATCHES FreeBSD AND CMAKE_CXX_COMPILER_VERSION MATCHES " 3.4.6 ")
    SET_SOURCE_FILES_PROPERTIES (except.cpp PROPERTIES COMPILE_FLAGS "-O0")
  ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES i386 AND CMAKE_SYSTEM_NAME MATCHES FreeBSD AND CMAKE_CXX_COMPILER_VERSION MATCHES " 3.4.6 ")
  SET_SOURCE_FILES_PROPERTIES (execinfo.cpp PROPERTIES COMPILE_FLAGS "-O0")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

INCLUDE_DIRECTORIES (${INCLUDE_DIRECTORIES} ../../include/libadicpp)

IF (CMAKE_SYSTEM_NAME MATCHES Linux)
# detect kernel headers & platform
#  EXECUTE_PROCESS (
#    COMMAND $ENV{SHELL} -c "uname -r"
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    RESULT_VARIABLE CMAKE_SHELL_RESULT
#    OUTPUT_VARIABLE CMAKE_SHELL_OUTPUT
#    ERROR_VARIABLE CMAKE_SHELL_ERROR
#  )
#  SET (LINUX_KERNEL_INCLUDE_DIR "/lib/modules/${CMAKE_SHELL_OUTPUT}/build/include")
#  STRING (REPLACE "\n" "" LINUX_KERNEL_INCLUDE_DIR ${LINUX_KERNEL_INCLUDE_DIR})
#  STRING (REPLACE "\r" "" LINUX_KERNEL_INCLUDE_DIR ${LINUX_KERNEL_INCLUDE_DIR})
#  SET_SOURCE_FILES_PROPERTIES (atomic.cpp PROPERTIES COMPILE_FLAGS "-I${LINUX_KERNEL_INCLUDE_DIR}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES Linux)
                              