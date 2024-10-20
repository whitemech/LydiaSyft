add_executable(dfa_representation_example dfa_representation.cpp)

target_include_directories(dfa_representation_example PRIVATE  ${UTILS_INCLUDE_PATH} ${PARSER_INCLUDE_PATH} ${SYNTHESIS_INCLUDE_PATH} ${EXT_INCLUDE_PATH})
target_link_libraries(dfa_representation_example ${PARSER_LIB_NAME} ${SYNTHESIS_LIB_NAME} ${UTILS_LIB_NAME} ${LYDIA_LIBRARIES})
