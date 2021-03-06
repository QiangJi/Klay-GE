SET(LIB_NAME KlayGE_AudioDataSource_OggVorbis)

SET(OV_ADS_SOURCE_SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Src/AudioDataSource/OggVorbis/OggVorbisSource.cpp
	${KLAYGE_PROJECT_DIR}/Plugins/Src/AudioDataSource/OggVorbis/OggVorbisSourceFactory.cpp
)

SET(OV_ADS_SOURCE_HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OggVorbis/OggVorbisSource.hpp
	${KLAYGE_PROJECT_DIR}/Plugins/Include/KlayGE/OggVorbis/OggVorbisSourceFactory.hpp
)

SOURCE_GROUP("Source Files" FILES ${OV_ADS_SOURCE_SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${OV_ADS_SOURCE_HEADER_FILES})

ADD_DEFINITIONS(-DKLAYGE_BUILD_DLL -DKLAYGE_OV_ADS_SOURCE)


INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/boost)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/libogg/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/libvorbis/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Core/Include)
INCLUDE_DIRECTORIES(${KLAYGE_PROJECT_DIR}/Plugins/Include)
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/boost/lib_${KLAYGE_COMPILER_NAME}${KLAYGE_COMPILER_VERSION}_${KLAYGE_ARCH_NAME}/lib)
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/libogg/lib/${KLAYGE_PLATFORM_NAME})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../External/libvorbis/libs/${KLAYGE_PLATFORM_NAME})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/../KFL/lib/${KLAYGE_PLATFORM_NAME})
LINK_DIRECTORIES(${KLAYGE_PROJECT_DIR}/lib/${KLAYGE_PLATFORM_NAME})

ADD_LIBRARY(${LIB_NAME} SHARED
	${OV_ADS_SOURCE_SOURCE_FILES} ${OV_ADS_SOURCE_HEADER_FILES}
)
ADD_DEPENDENCIES(${LIB_NAME} ${KLAYGE_CORELIB_NAME})

IF(MSVC)
	SET(EXTRA_LINKED_LIBRARIES
		debug libogg_static_d optimized libogg_static
		debug libvorbis_static_d optimized libvorbis_static
		debug libvorbisfile_static_d optimized libvorbisfile_static)
ELSE()
	SET(EXTRA_LINKED_LIBRARIES
		debug KlayGE_Core_${KLAYGE_COMPILER_NAME}_d optimized KlayGE_Core_${KLAYGE_COMPILER_NAME}
		debug KFL_${KLAYGE_COMPILER_NAME}_${KLAYGE_ARCH_NAME}_d optimized KFL_${KLAYGE_COMPILER_NAME}_${KLAYGE_ARCH_NAME}
		debug boost_system-${KLAYGE_COMPILER_NAME}${GCC_VER}-mt-d-${BOOST_VER} optimized boost_system-${KLAYGE_COMPILER_NAME}${GCC_VER}-mt-${BOOST_VER}
		vorbisfile_static vorbis_static ogg_static)
ENDIF()

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_OUTPUT_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_NAME}${KLAYGE_NAME_POSTFIX}
)

ADD_PRECOMPILED_HEADER(${LIB_NAME} "KlayGE/KlayGE.hpp" "${KLAYGE_PROJECT_DIR}/Core/Include" "${KLAYGE_PROJECT_DIR}/Plugins/Src/AudioDataSource/OggVorbis/OggVorbisSourceFactory.cpp")

TARGET_LINK_LIBRARIES(${LIB_NAME}
	${EXTRA_LINKED_LIBRARIES}
)


ADD_POST_BUILD(${LIB_NAME} "Audio")

	
INSTALL(TARGETS ${LIB_NAME}
	RUNTIME DESTINATION ${KLAYGE_BIN_DIR}/Audio
	LIBRARY DESTINATION ${KLAYGE_BIN_DIR}/Audio
	ARCHIVE DESTINATION ${KLAYGE_OUTPUT_DIR}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER "Audio System")
