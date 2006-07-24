ADD_EXECUTABLE(macrocope
  macroscope.cpp
  htmlrep.cpp
)

TARGET_LINK_LIBRARIES(macrocope
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)
