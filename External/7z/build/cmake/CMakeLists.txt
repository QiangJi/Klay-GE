PROJECT(7z)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

OPTION(KLAYGE_WITH_WINRT "WinRT support." FALSE)

SET(SEVENZIP_PROJECT_DIR "${CMAKE_HOME_DIRECTORY}/../../")

INCLUDE(${SEVENZIP_PROJECT_DIR}/../../cmake/Common.cmake)
INCLUDE(${SEVENZIP_PROJECT_DIR}/../../cmake/Platform.cmake)
INCLUDE(${SEVENZIP_PROJECT_DIR}/../../cmake/Compiler.cmake)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(SEVENZIP_OUTPUT_DIR "${SEVENZIP_PROJECT_DIR}/lib/${KLAYGE_PLATFORM_NAME}")

IF(MSVC)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /WX-")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX-")
ENDIF()

ADD_SUBDIRECTORY(7zxa)
ADD_SUBDIRECTORY(LZMA)
