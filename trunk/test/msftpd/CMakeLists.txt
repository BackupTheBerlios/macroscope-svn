ADD_EXECUTABLE (msftpd main.cpp)

TARGET_LINK_LIBRARIES (msftpd
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
  TARGETS msftpd RUNTIME
  DESTINATION "${INSTALL_BIN_DIR}"
  CONFIGURATIONS Release
)

INSTALL (FILES msftpd.conf DESTINATION "${INSTALL_ETC_DIR}")
