ADD_EXECUTABLE(msftpd main.cpp)

TARGET_LINK_LIBRARIES(msftpd
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

INSTALL(
  TARGETS msftpd RUNTIME
  DESTINATION ""
  CONFIGURATIONS Debug Release RelWithDebInf MinSizeRel
)

INSTALL(FILES msftpd.conf DESTINATION "")
