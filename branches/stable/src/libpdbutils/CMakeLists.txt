ADD_LIBRARY(pdbutils STATIC
  DbgString.cpp
  DbgType.cpp
  core_dbgtypes.cpp
  mfc_dbgtypes.cpp
  pdbutils.cpp
  stl_dbgtypes.cpp
)

INCLUDE_DIRECTORIES (${INCLUDE_DIRECTORIES} ../../include/libadicpp)

IF (WIN32)
  LINK_LIBRARIES(dbghelp)
ENDIF (WIN32)
