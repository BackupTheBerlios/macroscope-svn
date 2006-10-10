IF (UNIX)
  FIND_LIBRARY(sha256 NAMES sha256)
  FIND_LIBRARY(lzo NAMES lzo PATHS /usr/lib /usr/local/lib)
  FIND_LIBRARY(lzo2 NAMES lzo2 PATHS /usr/lib /usr/local/lib)
ENDIF (UNIX)

IF (sha256)
  #INCLUDE_DIRECTORIES(${sha256_INCLUDE_DIR})
ELSE (sha256)
  SET (sha256_SOURCES sha256c.c)
ENDIF (sha256)

IF (lzo2)
  #INCLUDE_DIRECTORIES(${lzo2_INCLUDE_DIR})
ELSE (lzo2)
  SET (lzo_SOURCES_NAME lzo2)
  IF (lzo)
    #INCLUDE_DIRECTORIES(${lzo_INCLUDE_DIR})
  ELSE (lzo)
    SET (lzo_SOURCES_NAME lzo)
  ENDIF (lzo)
  SET (${lzo_SOURCES_NAME}_SOURCES
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
ENDIF (lzo2)

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
  ${sha256_SOURCES}
  ${lzo2_SOURCES}
  ${lzo_SOURCES}
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ../../include/libadicpp)

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
  )
ENDIF (WIN32)

IF (sha256)
  SET (ADD_LIBS ${ADD_LIBS} ${sha256})
ENDIF (sha256)

IF (lzo2)
  SET (ADD_LIBS ${ADD_LIBS} ${lzo2})
ENDIF (lzo2)

IF (lzo)
  SET (ADD_LIBS ${ADD_LIBS} ${lzo})
ENDIF (lzo)

#TARGET_LINK_LIBRARIES(libksys ${ADD_LIBS})
