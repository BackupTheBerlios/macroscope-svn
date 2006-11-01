IF (NOT LIB_MD)
  SET (ADD_SOURCES
    sha256c.c
  )
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

ADD_LIBRARY(ksys STATIC
  afd.cpp
  aio.cpp
  archive.cpp
  conf.cpp
  except.cpp
  fetch.cpp
  fhc.cpp
  fiber.cpp
  log.cpp
  lzo.cpp
  mstream.cpp
  mutant.cpp
  mutex.cpp
  perror.cpp
  profiler.cpp
  rnd.cpp
  service.cpp
  sha.cpp
  shobjs.cpp
  stat.cpp
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

#IF (CMAKE_COMPILER_IS_GNUCXX)
#  IF (CMAKE_SYSTEM_PROCESSOR MATCHES amd64)
#    SET_SOURCE_FILES_PROPERTIES (fiber.cpp PROPERTIES COMPILE_FLAGS "-fno-defer-pop -fno-omit-frame-pointer")
#  ENDIF (CMAKE_SYSTEM_PROCESSOR MATCHES amd64)
#ENDIF (CMAKE_COMPILER_IS_GNUCXX)

INCLUDE_DIRECTORIES (${INCLUDE_DIRECTORIES} ../../include/libadicpp)

IF (WIN32)
#  FIND_LIBRARY(ws2_32_LIBRARY
#    NAMES ws2_32
#    PATHS ${LIBPATHS}
#    DOC "ws2_32 library"
#  )
  SET (ADD_LIBS
    ws2_32
    mswsock
    iphlpapi
    dbghelp
  )
ENDIF (WIN32)
