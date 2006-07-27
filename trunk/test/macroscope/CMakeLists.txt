ADD_EXECUTABLE (macroscope
  macroscope.cpp
  htmlrep.cpp
  version.c
)

TARGET_LINK_LIBRARIES (macroscope
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

SET_SOURCE_FILES_PROPERTIES (version.c PROPERTIES GENERATED YES)
ADD_DEPENDENCIES (macroscope version)
ADD_CUSTOM_COMMAND (
  TARGET macroscope PRE_BUILD
  COMMAND ${VERSION_EXECUTABLE} ARGS version.in version.c
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generate version file"
)

IF (SITE STREQUAL "${MAIN_SITE}")
  ADD_CUSTOM_COMMAND (
    TARGET macroscope POST_BUILD
    COMMAND ${VERSION_EXECUTABLE} ARGS version.in version.in
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Increase version level"
  )
ENDIF (SITE STREQUAL "${MAIN_SITE}")

IF (WIN32 AND NOT UNIX)
  SET (INSTALL_BIN_DIR "")
  SET (INSTALL_ETC_DIR "")
ELSE (WIN32 AND NOT UNIX)
  SET (INSTALL_BIN_DIR "/bin")
  SET (INSTALL_ETC_DIR "/etc")
ENDIF (WIN32 AND NOT UNIX)

INSTALL (
  TARGETS macroscope RUNTIME
  DESTINATION "${INSTALL_BIN_DIR}"
  CONFIGURATIONS Release
)

INSTALL (FILES macroscope.conf DESTINATION "${INSTALL_ETC_DIR}")
