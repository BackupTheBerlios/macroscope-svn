ADD_EXECUTABLE(msftpd main.cpp)

TARGET_LINK_LIBRARIES(msftpd
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

INSTALL(
  TARGETS msftpd
  RUNTIME DESTINATION bin
)
