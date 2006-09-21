ADD_EXECUTABLE (msmail
  main.cpp
  server.cpp
  fiber.cpp
  info.cpp
  service.cpp
  version.c
)

TARGET_LINK_LIBRARIES (msmail
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

SET_SOURCE_FILES_PROPERTIES (version.c PROPERTIES GENERATED YES)
ADD_DEPENDENCIES (msmail version)
ADD_CUSTOM_COMMAND (
  TARGET msmail PRE_BUILD
  COMMAND ${VERSION_EXECUTABLE} ARGS version.in version.c
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generate version file"
)

#IF (SITE STREQUAL "${MAIN_SITE}")
#  ADD_CUSTOM_COMMAND (
#    TARGET msmail POST_BUILD
#    COMMAND ${VERSION_EXECUTABLE} ARGS version.in version.in
#    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#    COMMENT "Increase version level"
#  )
#ENDIF (SITE STREQUAL "${MAIN_SITE}")

IF (SITE STREQUAL "${MAIN_SITE}" AND CMAKE_BUILD_TYPE STREQUAL "Release")
  ADD_CUSTOM_COMMAND (
    TARGET msmail POST_BUILD
    COMMAND "C:\\Program Files (x86)\\ASProtect 1.35 Demo\\ASProtect.exe" ARGS -process msmail.aspr
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT ""
  )
ENDIF (SITE STREQUAL "${MAIN_SITE}" AND CMAKE_BUILD_TYPE STREQUAL "Release")

IF (WIN32 AND NOT UNIX)
  SET (INSTALL_BIN_DIR "")
  SET (INSTALL_ETC_DIR "")
ELSE (WIN32 AND NOT UNIX)
  SET (INSTALL_BIN_DIR "/bin")
  SET (INSTALL_ETC_DIR "/etc")
ENDIF (WIN32 AND NOT UNIX)

INSTALL (
  TARGETS msmail RUNTIME
  DESTINATION "${INSTALL_BIN_DIR}"
)

INSTALL (FILES msmail.conf DESTINATION "${INSTALL_ETC_DIR}")
