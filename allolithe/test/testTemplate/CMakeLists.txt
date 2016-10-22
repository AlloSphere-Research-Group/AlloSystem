file(GLOB SRCS *.cpp)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

ADD_EXECUTABLE(testTemplate ${SRCS})

TARGET_LINK_LIBRARIES(testTemplate
	lithe_core
	libgtest
	libgmock
)

add_test(NAME testTemplate 
		COMMAND testTemplate)

add_dependencies(test_allolithe testTemplate)