ADD_EXECUTABLE (macroscope
  macroscope.cpp
  htmlrep.cpp
  bpfthtml.cpp
  sniffer.cpp
  version.c
)
 
TARGET_LINK_LIBRARIES (macroscope
  adicpp
  ksys
  fbcpp
  mycpp
  odbcpp
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

IF (CMAKE_COMPILER_IS_GNUCXX)
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
    SET_SOURCE_FILES_PROPERTIES (bpfthtml.cpp PROPERTIES COMPILE_FLAGS "${CMAKE_COMPILER_BUGFIX_FLAGS}")
  ENDIF (CMAKE_SYSTEM_NAME MATCHES FreeBSD AND CMAKE_CXX_COMPILER_VERSION MATCHES " 3.4.6 ")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

GET_TARGET_PROPERTY (MACROSCOPE_EXECUTABLE macroscope LOCATION)

FIND_PROGRAM (ICCPATCH iccpatch)
IF (ICCPATCH AND CMAKE_CXX_COMPILER MATCHES icl)
  ADD_CUSTOM_COMMAND (
    TARGET macroscope POST_BUILD
    COMMAND ${ICCPATCH} ARGS ${MACROSCOPE_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT ""
  )
ENDIF (ICCPATCH AND CMAKE_CXX_COMPILER MATCHES icl)

FIND_PROGRAM (UPX upx)
FIND_PROGRAM (ASPR NAMES asprotect PATHS "C:/Arx/ASProtect")

IF (PRIVATE_RELEASE AND CMAKE_BUILD_TYPE STREQUAL Release AND NOT CMAKE_GENERATOR MATCHES Win64 AND CMAKE_SYSTEM_NAME MATCHES Windows)
  IF (ASPR)
    STRING (REPLACE "/" "\\" MACROSCOPE_EXECUTABLE_WINPATH ${MACROSCOPE_EXECUTABLE})
    STRING (REPLACE "\\\\" "\\" MACROSCOPE_EXECUTABLE_WINPATH ${MACROSCOPE_EXECUTABLE_WINPATH})
    CONFIGURE_FILE (
      "${CMAKE_CURRENT_BINARY_DIR}/macroscope.aspr.template"
      "${CMAKE_CURRENT_BINARY_DIR}/macroscope.aspr"
      !ONLY
    )
    ADD_CUSTOM_COMMAND (
      TARGET macroscope POST_BUILD
      COMMAND ${ASPR} ARGS -process "${CMAKE_CURRENT_BINARY_DIR}\\macroscope.aspr"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT ""
    )
  ELSE (ASPR)
    IF (UPX)
      ADD_CUSTOM_COMMAND (
        TARGET macroscope POST_BUILD
        COMMAND ${UPX} ARGS -q -k --best --lzma ${MACROSCOPE_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT ""
      )
    ENDIF (UPX)
  ENDIF (ASPR)
ENDIF (PRIVATE_RELEASE AND CMAKE_BUILD_TYPE STREQUAL Release AND NOT CMAKE_GENERATOR MATCHES Win64 AND CMAKE_SYSTEM_NAME MATCHES Windows)

IF (NOT PRIVATE_RELEASE AND CMAKE_BUILD_TYPE STREQUAL Release AND NOT CMAKE_GENERATOR MATCHES Win64 AND CMAKE_SYSTEM_NAME MATCHES Windows)
  IF (UPX)
    ADD_CUSTOM_COMMAND (
      TARGET macroscope POST_BUILD
      COMMAND ${UPX} ARGS -q -k --best --lzma ${MACROSCOPE_EXECUTABLE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT ""
    )
  ENDIF (UPX)
ENDIF (NOT PRIVATE_RELEASE AND CMAKE_BUILD_TYPE STREQUAL Release AND NOT CMAKE_GENERATOR MATCHES Win64 AND CMAKE_SYSTEM_NAME MATCHES Windows)

INSTALL (
  TARGETS macroscope RUNTIME
  DESTINATION "${INSTALL_BIN_DIR}"
)

INSTALL (FILES macroscope.conf DESTINATION "${INSTALL_ETC_DIR}" RENAME macroscope.conf.template)
IF (NOT CMAKE_NOT_INSTALL_DOC)
  INSTALL (FILES mysql-user DESTINATION "${INSTALL_DOC_DIR}" RENAME mysql-user)
ENDIF (NOT CMAKE_NOT_INSTALL_DOC)
