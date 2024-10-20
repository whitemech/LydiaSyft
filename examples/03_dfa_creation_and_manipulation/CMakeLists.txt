add_executable(dfa_creation_and_manipulation_example dfa_creation_and_manipulation.cpp)

target_include_directories(dfa_creation_and_manipulation_example PRIVATE  ${UTILS_INCLUDE_PATH} ${PARSER_INCLUDE_PATH} ${SYNTHESIS_INCLUDE_PATH} ${EXT_INCLUDE_PATH})
target_link_libraries(dfa_creation_and_manipulation_example ${PARSER_LIB_NAME} ${SYNTHESIS_LIB_NAME} ${UTILS_LIB_NAME} ${LYDIA_LIBRARIES})
