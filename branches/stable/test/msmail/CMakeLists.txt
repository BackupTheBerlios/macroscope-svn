ADD_EXECUTABLE (msmail
  main.cpp
  server.cpp
  fiber.cpp
  info.cpp
  service.cpp
  version.c
)

IF (CMAKE_COMPILER_IS_GNUCXX)
# internal compiler error: in remove_insn, at emit-rtl.c:3746
  SET (CMAKE_COMPILER_BUGFIX_FLAGS_LIST0
    -O
    -fforce-mem
    -foptimize-sibling-calls
    -fstrength-reduce
    -fcse-follow-jumps
    -fcse-skip-blocks
    -frerun-cse-after-loop
    -frerun-loop-opt
    -fgcse
    -fgcse-lm
    -fgcse-sm
    -fgcse-las
    -fdelete-null-pointer-checks
    -fexpensive-optimizations
    -fregmove
#    -fschedule-insns
    -fschedule-insns2
    -fsched-interblock
    -fsched-spec
    -fcaller-saves
    -fpeephole2
    -freorder-blocks
    -freorder-functions
    -fstrict-aliasing
#    -funit-at-a-time
    -falign-functions
    -falign-jumps
    -falign-loops
    -falign-labels
    -fcrossjumping
    -finline-functions
    -fweb
    -frename-registers
    -funswitch-loops
  )
  SET (CMAKE_COMPILER_BUGFIX_FLAGS_LIST
    -fno-schedule-insns
    -fno-unit-at-a-time
  )
  LIST (LENGTH CMAKE_COMPILER_BUGFIX_FLAGS_LIST CMAKE_COMPILER_BUGFIX_FLAGS_LIST_LENGTH)
  WHILE (CMAKE_COMPILER_BUGFIX_FLAGS_LIST_LENGTH GREATER 0)
    LIST(GET CMAKE_COMPILER_BUGFIX_FLAGS_LIST 0 CMAKE_COMPILER_BUGFIX_FLAG)
    SET (CMAKE_COMPILER_BUGFIX_FLAGS "${CMAKE_COMPILER_BUGFIX_FLAGS} ${CMAKE_COMPILER_BUGFIX_FLAG}")
    LIST(REMOVE_AT CMAKE_COMPILER_BUGFIX_FLAGS_LIST 0)
    LIST (LENGTH CMAKE_COMPILER_BUGFIX_FLAGS_LIST CMAKE_COMPILER_BUGFIX_FLAGS_LIST_LENGTH)
  ENDWHILE (CMAKE_COMPILER_BUGFIX_FLAGS_LIST_LENGTH GREATER 0)

  IF (CMAKE_SYSTEM_NAME MATCHES FreeBSD AND CMAKE_CXX_COMPILER_VERSION MATCHES " 3.4.6 ")
    SET_SOURCE_FILES_PROPERTIES (server.cpp info.cpp PROPERTIES COMPILE_FLAGS "${CMAKE_COMPILER_BUGFIX_FLAGS}")
  ENDIF (CMAKE_SYSTEM_NAME MATCHES FreeBSD AND CMAKE_CXX_COMPILER_VERSION MATCHES " 3.4.6 ")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

ADD_DEFINITIONS(-DDISABLE_FIREBIRD_INTERFACE -DDISABLE_MYSQL_INTERFACE)

TARGET_LINK_LIBRARIES (msmail
  adicpp
  fbcpp
  ksys
  mycpp
  utf8
  ${LIBPDBUTILS}
)

SET_SOURCE_FILES_PROPERTIES (version.c PROPERTIES GENERATED YES)

ADD_CUSTOM_COMMAND (
  OUTPUT version.c
  COMMAND ${VERSION_EXECUTABLE} version.in version.c
  DEPENDS version.in
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
	
GET_TARGET_PROPERTY (MSMAIL_EXECUTABLE msmail LOCATION)
	
FIND_PROGRAM (ICCPATCH iccpatch)
IF (ICCPATCH AND CMAKE_CXX_COMPILER MATCHES icl)
  ADD_CUSTOM_COMMAND (
    TARGET msmail POST_BUILD
    COMMAND ${ICCPATCH} ARGS ${MSMAIL_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT ""
  )
ENDIF (ICCPATCH AND CMAKE_CXX_COMPILER MATCHES icl)

FIND_PROGRAM (UPX upx)
FIND_PROGRAM (ASPR NAMES asprotect PATHS "C:/Arx/ASProtect")

IF (PRIVATE_RELEASE AND CMAKE_BUILD_TYPE STREQUAL Release AND NOT CMAKE_GENERATOR MATCHES Win64 AND CMAKE_SYSTEM_NAME MATCHES Windows)
  IF (ASPR)
    STRING (REPLACE "/" "\\" MSMAIL_EXECUTABLE_WINPATH ${MSMAIL_EXECUTABLE})
    STRING (REPLACE "\\\\" "\\" MSMAIL_EXECUTABLE_WINPATH ${MSMAIL_EXECUTABLE_WINPATH})
    CONFIGURE_FILE (
      "${CMAKE_CURRENT_BINARY_DIR}/msmail.aspr.template"
      "${CMAKE_CURRENT_BINARY_DIR}/msmail.aspr"
      !ONLY
    )
    ADD_CUSTOM_COMMAND (
      TARGET msmail POST_BUILD
      COMMAND ${ASPR} ARGS -process "${CMAKE_CURRENT_BINARY_DIR}\\msmail.aspr"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT ""
    )
  ELSE (ASPR)
    IF (UPX)
      ADD_CUSTOM_COMMAND (
        TARGET msmail POST_BUILD
        COMMAND ${UPX} ARGS -q --brute ${MSMAIL_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT ""
      )
    ENDIF (UPX)
  ENDIF (ASPR)
ENDIF (PRIVATE_RELEASE AND CMAKE_BUILD_TYPE STREQUAL Release AND NOT CMAKE_GENERATOR MATCHES Win64 AND CMAKE_SYSTEM_NAME MATCHES Windows)

INSTALL (
  TARGETS msmail RUNTIME
  DESTINATION "${INSTALL_BIN_DIR}"
)

INSTALL (FILES msmail.conf DESTINATION "${INSTALL_ETC_DIR}" RENAME msmail.conf.template)
INSTALL (FILES machine.conf DESTINATION "${INSTALL_ETC_DIR}" RENAME machine.conf.template)

