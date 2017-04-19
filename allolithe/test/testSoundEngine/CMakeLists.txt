file(GLOB SRCS *.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

ADD_EXECUTABLE(testSoundEngine ${SRCS})

TARGET_LINK_LIBRARIES(testSoundEngine
	allolithe
	libgtest
	libgmock
)

add_test(NAME testSoundEngine 
		COMMAND testSoundEngine)

add_dependencies(test_allolithe testSoundEngine)
