SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/PlatformDeployer/PlatformDeployer.cpp
)

IF(MSVC)
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES})
ELSE()
	SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
		debug boost_program_options-${KLAYGE_COMPILER_NAME}${GCC_VER}-mt-d-${BOOST_VER} optimized boost_program_options-${KLAYGE_COMPILER_NAME}${GCC_VER}-mt-${BOOST_VER}
		debug boost_filesystem-${KLAYGE_COMPILER_NAME}${GCC_VER}-mt-d-${BOOST_VER} optimized boost_filesystem-${KLAYGE_COMPILER_NAME}${GCC_VER}-mt-${BOOST_VER})
ENDIF()

SETUP_TOOL(PlatformDeployer)
