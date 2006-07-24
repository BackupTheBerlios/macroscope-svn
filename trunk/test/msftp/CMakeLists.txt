ADD_EXECUTABLE(msftp main.cpp)

TARGET_LINK_LIBRARIES(msftp
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

INSTALL(
  TARGETS msftp
  RUNTIME DESTINATION bin
)
