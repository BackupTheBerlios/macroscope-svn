ADD_EXECUTABLE(macroscope
  macroscope.cpp
  htmlrep.cpp
)

TARGET_LINK_LIBRARIES(macroscope
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

INSTALL(
  TARGETS macroscope
  RUNTIME DESTINATION bin
)
