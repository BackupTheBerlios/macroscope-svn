ADD_EXECUTABLE (macroscope
  macroscope.cpp
  htmlrep.cpp
)

TARGET_LINK_LIBRARIES (macroscope
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
  TARGETS macroscope RUNTIME
  DESTINATION "${INSTALL_BIN_DIR}"
  CONFIGURATIONS Release
)

INSTALL (FILES macroscope.conf DESTINATION "${INSTALL_ETC_DIR}")
