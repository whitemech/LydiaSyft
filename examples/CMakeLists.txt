
add_executable(quickstart_example quickstart/quickstart.cpp)

target_include_directories(quickstart_example PRIVATE  ${UTILS_INCLUDE_PATH} ${PARSER_INCLUDE_PATH} ${SYNTHESIS_INCLUDE_PATH} ${EXT_INCLUDE_PATH})
target_link_libraries(quickstart_example ${PARSER_LIB_NAME} ${SYNTHESIS_LIB_NAME} ${UTILS_LIB_NAME} ${LYDIA_LIBRARIES})
