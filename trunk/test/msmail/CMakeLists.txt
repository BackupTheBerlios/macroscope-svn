ADD_EXECUTABLE(msmail
  main.cpp
  server.cpp
  fiber.cpp
  info.cpp
  service.cpp
)

TARGET_LINK_LIBRARIES(msmail
  libadicpp
  libfbcpp
  libksys
  libmycpp
  libutf8
)

INSTALL(
  TARGETS msmail
  RUNTIME DESTINATION bin
)
