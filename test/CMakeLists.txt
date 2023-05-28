
# Fetch Catch2
Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.2.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

set (TEST_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
file (GLOB_RECURSE TEST_SOURCE_FILES "${TEST_MODULE_PATH}/*.cpp")
include_directories(${PARSER_INCLUDE_PATH} ${SYNTHESIS_INCLUDE_PATH} ${EXT_INCLUDE_PATH})
add_executable(tests ${TEST_SOURCE_FILES} utils.hpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain ${PARSER_LIB_NAME} ${SYNTHESIS_LIB_NAME}  ${EXT_LIBRARIES_PATH})

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

catch_discover_tests(tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME tests COMMAND tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
