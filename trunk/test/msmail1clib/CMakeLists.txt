ADD_LIBRARY (msmail1clib SHARED
  ../msmail/info.cpp
  client.cpp
  Registry.cpp
  msmail1cImpl.cpp
  msmail1clib.cpp 
  msmail1clib.def
  msmail1clib.idl
  msmail1clib.rc
  msmail1c.rgs
  msmail1clib.rgs
#  dlldata.c
  dlldatax.c
  msmail1clib_i.c
#  msmail1clib_p.c
  stdafx.cpp
)

ADD_DEFINITIONS (-DDISABLE_FIREBIRD_INTERFACE -DDISABLE_MYSQL_INTERFACE)

TARGET_LINK_LIBRARIES (msmail1clib
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

ADD_DEPENDENCIES (msmail1clib msmail)

IF (WIN32 AND NOT UNIX)
  SET (INSTALL_BIN_DIR "")
  SET (INSTALL_ETC_DIR "")
ELSE (WIN32 AND NOT UNIX)
  SET (INSTALL_BIN_DIR "/lib")
  SET (INSTALL_ETC_DIR "/etc")
ENDIF (WIN32 AND NOT UNIX)

INSTALL (
  TARGETS msmail1clib LIBRARY RUNTIME
  DESTINATION "${INSTALL_BIN_DIR}"
  CONFIGURATIONS Debug Release RelWithDebInf MinSizeRel
)

INSTALL (FILES msmail1c.conf DESTINATION "${INSTALL_ETC_DIR}")
