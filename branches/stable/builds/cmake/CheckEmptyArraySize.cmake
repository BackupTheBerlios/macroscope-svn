# - Check sizeof a type
#  CHECK_EMPTY_ARRAY_SIZE(VARIABLE)
# Check if the type exists and determine size of type.  if the type
# exists, the size will be stored to the variable.
#  VARIABLE - variable to store size if the type exists.
#  HAVE_${VARIABLE} - does the variable exists or not
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories
#  CMAKE_REQUIRED_LIBRARIES = list of libraries to link

MACRO(CHECK_EMPTY_ARRAY_SIZE VARIABLE)
  SET(CMAKE_ALLOW_UNKNOWN_VARIABLE_READ_ACCESS 1)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET (MACRO_CHECK_EMPTY_ARRAY_SIZE_FLAGS
      "${CMAKE_REQUIRED_FLAGS} -DCHECK_EMPTY_ARRAY_SIZE"
    )
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/builds/cmake/CheckEmptyArraySize.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_EMPTY_ARRAY_SIZE_FLAGS}
      OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Check empty array size - done")
      SET (${VARIABLE} " " CACHE INTERNAL "Empty array size = empty string")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log 
        "Determining size of ${TYPE} passed with the following output:\n${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log 
        "Determining size of ${TYPE} failed with the following output:\n${OUTPUT}\nCheckEmptyArraySize.c:\n\n")
      SET (MACRO_CHECK_EMPTY_ARRAY_SIZE_FLAGS
        "${CMAKE_REQUIRED_FLAGS} -DCHECK_EMPTY_ARRAY_SIZE=0"
      )
      TRY_COMPILE(${VARIABLE}
        ${CMAKE_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/CheckEmptyArraySize.c
        COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
        CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_EMPTY_ARRAY_SIZE_FLAGS}
        OUTPUT_VARIABLE OUTPUT)
      IF(${VARIABLE})
        MESSAGE(STATUS "Check empty array size - done")
        SET (${VARIABLE} 0 CACHE INTERNAL "Empty array size = 0")
        FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log 
          "Determining size of ${TYPE} passed with the following output:\n${OUTPUT}\n\n")
      ELSE(${VARIABLE})
        MESSAGE(STATUS "Check empty array size - failed")
        SET (${VARIABLE} -1 CACHE INTERNAL "Empty array size not supported")
        FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log 
          "Determining size of ${TYPE} failed with the following output:\n${OUTPUT}\nCheckEmptyArraySize.c:\n\n")
      ENDIF(${VARIABLE})
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
  SET(CMAKE_ALLOW_UNKNOWN_VARIABLE_READ_ACCESS )
ENDMACRO(CHECK_EMPTY_ARRAY_SIZE)
