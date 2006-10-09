IF (UNIX)
  FIND_PACKAGE(sha256 lzo lzo2)
ENDIF (UNIX)

IF (sha256_FOUND)
  INCLUDE_DIRECTORIES(${sha256_INCLUDE_DIR})
ELSE (sha256_FOUND)
  SET (sha256_SOURCES sha256c.c)
ENDIF (sha256_FOUND)

IF (lzo2_FOUND)
  INCLUDE_DIRECTORIES(${lzo2_INCLUDE_DIR})
ELSE (lzo2_FOUND)
  SET (lzo_SOURCES_NAME lzo2)
  IF (lzo_FOUND)
    INCLUDE_DIRECTORIES(${lzo_INCLUDE_DIR})
  ELSE (lzo_FOUND)
    SET (lzo_SOURCES_NAME lzo)
  ENDIF (lzo_FOUND)
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
ENDIF (lzo2_FOUND)

ADD_LIBRARY(libksys STATIC
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

TARGET_LINK_LIBRARIES(libksys ${ADD_LIBS})

IF (sha256_FOUND)
  SET (ADD_LIBS ${ADD_LIBS} ${sha256_LIBRARY})
ENDIF (sha256_FOUND)

IF (lzo2_FOUND)
  SET (ADD_LIBS ${ADD_LIBS} ${lzo2_LIBRARY})
ENDIF (lzo2_FOUND)

IF (lzo_FOUND)
  SET (ADD_LIBS ${ADD_LIBS} ${lzo_LIBRARY})
ENDIF (lzo_FOUND)

TARGET_LINK_LIBRARIES(libksys ${ADD_LIBS})
