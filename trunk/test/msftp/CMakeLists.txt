ADD_EXECUTABLE (msftp main.cpp)

TARGET_LINK_LIBRARIES (msftp
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

INSTALL (
  TARGETS msftp RUNTIME
  DESTINATION ""
  CONFIGURATIONS Debug Release RelWithDebInf MinSizeRel
)

INSTALL (FILES msftp.conf DESTINATION "")
