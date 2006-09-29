ADD_EXECUTABLE (msupdater main.cpp version.c)

TARGET_LINK_LIBRARIES (msupdater
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

SET_SOURCE_FILES_PROPERTIES (version.c PROPERTIES GENERATED YES)
ADD_DEPENDENCIES (msupdater version)
ADD_CUSTOM_COMMAND (
  TARGET msupdater PRE_BUILD
  COMMAND ${VERSION_EXECUTABLE} ARGS version.in version.c
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generate version file"
)

#IF (SITE STREQUAL "${MAIN_SITE}")
#  ADD_CUSTOM_COMMAND (
#    TARGET msupdater POST_BUILD
#    COMMAND ${VERSION_EXECUTABLE} ARGS version.in version.in
#    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#    COMMENT "Increase version level"
#  )
#ENDIF (SITE STREQUAL "${MAIN_SITE}")

IF (WIN32 AND NOT UNIX)
  SET (INSTALL_BIN_DIR "")
  SET (INSTALL_ETC_DIR "")
ELSE (WIN32 AND NOT UNIX)
  SET (INSTALL_BIN_DIR "/bin")
  SET (INSTALL_ETC_DIR "/etc")
ENDIF (WIN32 AND NOT UNIX)

INSTALL (
  TARGETS msupdater RUNTIME
  DESTINATION "${INSTALL_BIN_DIR}"
)

INSTALL (FILES msupdater.conf DESTINATION "${INSTALL_ETC_DIR}" RENAME msupdater.conf.template)
