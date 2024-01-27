add_subdirectory(utils)
add_subdirectory(parser)
add_subdirectory(synthesis)

include_directories(cli ${UTILS_INCLUDE_PATH} ${PARSER_INCLUDE_PATH} ${SYNTHESIS_INCLUDE_PATH} ${EXT_INCLUDE_PATH})
add_executable(LydiaSyft Main.cpp cli/base.cpp cli/fairness.cpp cli/gr1.cpp cli/maxset.cpp cli/stability.cpp cli/synthesis.cpp)

target_link_libraries(LydiaSyft ${PARSER_LIB_NAME} ${SYNTHESIS_LIB_NAME} ${UTILS_LIB_NAME} ${LYDIA_LIBRARIES})


install(TARGETS LydiaSyft
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  )

INSTALL (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*")

#export vars
set(UTILS_INCLUDE_PATH  ${UTILS_INCLUDE_PATH} PARENT_SCOPE)
set(UTILS_LIB_NAME ${UTILS_LIB_NAME} PARENT_SCOPE)
set(PARSER_INCLUDE_PATH  ${PARSER_INCLUDE_PATH} PARENT_SCOPE)
set(PARSER_LIB_NAME ${PARSER_LIB_NAME} PARENT_SCOPE)
set(SYNTHESIS_INCLUDE_PATH  ${SYNTHESIS_INCLUDE_PATH} PARENT_SCOPE)
set(SYNTHESIS_LIB_NAME ${SYNTHESIS_LIB_NAME} PARENT_SCOPE)
