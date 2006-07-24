ADD_LIBRARY(msmail1clib SHARED
  client.cpp
  Registry.cpp
  msmail1cImpl.cpp
  msmail1clib.cpp 
  msmail1clib.def
  msmail1clib.idl
  msmail1clib.rc
  msmail1c.rgs
  msmail1clib.rgs
  dlldata.c
  dlldatax.c
  msmail1clib_i.c
  msmail1clib_p.c
  stdafx.cpp
)

TARGET_LINK_LIBRARIES(msmail1clib
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

ADD_DEPENDENCIES(msmail1clib msmail)

INSTALL(
  TARGETS msmail1clib LIBRARY
  DESTINATION ""
  CONFIGURATIONS Debug Release RelWithDebInf MinSizeRel
)

INSTALL(FILES msmail1c.conf DESTINATION "")
