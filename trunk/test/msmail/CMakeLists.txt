ADD_EXECUTABLE (msmail
  main.cpp
  server.cpp
  fiber.cpp
  info.cpp
  service.cpp
)

TARGET_LINK_LIBRARIES (msmail
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

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
  CONFIGURATIONS Release
)

INSTALL (FILES msmail.conf DESTINATION "${INSTALL_ETC_DIR}")
