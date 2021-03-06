PROJECT(3DSMaxMeshMLPlugin)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(KLAYGE_3DSMAX_PATH "" CACHE STRING "Path of 3DSMax.")
SET(KLAYGE_3DSMAX_SDK_PATH "" CACHE STRING "Path of 3DSMax SDK.")
OPTION(KLAYGE_3DSMAX_UNICODE "3DSMax is unicode version (>=2013)." FALSE)

FIND_PATH(KLAYGE_PROJECT_DIR klayge_logo.ico PATHS ../../../../)

INCLUDE(${KLAYGE_PROJECT_DIR}/../cmake/Common.cmake)
INCLUDE(${KLAYGE_PROJECT_DIR}/../cmake/Platform.cmake)
INCLUDE(${KLAYGE_PROJECT_DIR}/../cmake/Compiler.cmake)
IF(NOT KLAYGE_3DSMAX_UNICODE)
	REMOVE_DEFINITIONS(-DUNICODE -D_UNICODE)
ENDIF()

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(KLAYGE_3DSMAX_PLUGIN_DIR "${KLAYGE_3DSMAX_PATH}/plugins")

SET(PLUGIN_NAME "MeshML")

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/export_main.cpp
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/mesh_extractor.cpp
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/meshml.cpp
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/util.cpp
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/export.def
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/export_main.hpp
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/mesh_extractor.hpp
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/meshml.hpp
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/resource.h
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/util.hpp
)

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Exporters/src/3DSMax/meshml.rc
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Resource Files" FILES ${RESOURCE_FILES})

INCLUDE_DIRECTORIES(${KLAYGE_3DSMAX_SDK_PATH}/maxsdk/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/boost)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Core/Include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../MeshMLLib/include)
LINK_DIRECTORIES(${KLAYGE_3DSMAX_SDK_PATH}/maxsdk/${KLAYGE_ARCH_NAME}/lib)
IF(MSVC)
	IF(MSVC_VERSION LESS 1699)
		LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/boost/lib_${KLAYGE_COMPILER_NAME}${KLAYGE_COMPILER_VERSION}_${KLAYGE_ARCH_NAME}/lib)
	ENDIF()
ELSE()
	LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/boost/lib_${KLAYGE_COMPILER_NAME}${KLAYGE_COMPILER_VERSION}_${KLAYGE_ARCH_NAME}/lib)
ENDIF()
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/lib/${KLAYGE_PLATFORM_NAME})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../MeshMLLib/lib/${KLAYGE_PLATFORM_NAME})

ADD_LIBRARY(${PLUGIN_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

SET_TARGET_PROPERTIES(${PLUGIN_NAME} PROPERTIES
	PROJECT_LABEL ${PLUGIN_NAME}
	DEBUG_POSTFIX ""
	OUTPUT_NAME ${PLUGIN_NAME}
)

SET(EXTRA_LINKED_LIBRARIES core maxutil geom mesh)

TARGET_LINK_LIBRARIES(${PLUGIN_NAME}
	debug MeshMLLib_${KLAYGE_COMPILER_NAME}_${KLAYGE_ARCH_NAME}${CMAKE_DEBUG_POSTFIX}
	optimized MeshMLLib_${KLAYGE_COMPILER_NAME}_${KLAYGE_ARCH_NAME}
	${EXTRA_LINKED_LIBRARIES})

ADD_CUSTOM_COMMAND(TARGET ${PLUGIN_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:${PLUGIN_NAME}>/$<TARGET_FILE_NAME:${PLUGIN_NAME}>
		${KLAYGE_3DSMAX_PLUGIN_DIR}/${PLUGIN_NAME}.dle)


INSTALL(TARGETS ${PLUGIN_NAME}
	RUNTIME DESTINATION ${KLAYGE_3DSMAX_PLUGIN_DIR}
	LIBRARY DESTINATION ${KLAYGE_3DSMAX_PLUGIN_DIR}
)

CREATE_VCPROJ_USERFILE(${PLUGIN_NAME})
