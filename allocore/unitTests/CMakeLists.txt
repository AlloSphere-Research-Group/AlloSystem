#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SRC_LIST ut*.cpp)

file(GLOB TEST_SRC_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "ut*.cpp")

set(TEST_ARGS "")

get_target_property(ALLOCORE_LIBRARY allocore${DEBUG_SUFFIX} LOCATION)
get_target_property(ALLOCORE_DEP_INCLUDE_DIRS allocore${DEBUG_SUFFIX} ALLOCORE_DEP_INCLUDE_DIRS)
get_target_property(ALLOCORE_LINK_LIBRARIES allocore${DEBUG_SUFFIX} ALLOCORE_LINK_LIBRARIES)

add_executable(allocoreTests unitTests.cpp ${TEST_SRC_LIST})
include_directories("${BUILD_ROOT_DIR}/build/include/")
target_link_libraries(allocoreTests ${ALLOCORE_LIBRARY} ${ALLOCORE_LINK_LIBRARIES})
add_dependencies(allocoreTests allocore${DEBUG_SUFFIX})
if(TRAVIS_BUILD)
add_definitions(-DALLOCORE_TESTS_NO_GUI)
endif(TRAVIS_BUILD)
add_test(NAME allocoreTests
         COMMAND $<TARGET_FILE:allocoreTests> ${TEST_ARGS})
add_memcheck_test(allocoreTests)

set_tests_properties(allocoreTests PROPERTIES DEPENDS allocore${DEBUG_SUFFIX})
