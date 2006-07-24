ADD_EXECUTABLE(msftp main.cpp)

TARGET_LINK_LIBRARIES(msftp
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)
