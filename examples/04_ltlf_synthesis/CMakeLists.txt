add_executable(ltlf_synthesis_example ltlf_synthesis.cpp)

target_include_directories(ltlf_synthesis_example PRIVATE  ${UTILS_INCLUDE_PATH} ${PARSER_INCLUDE_PATH} ${SYNTHESIS_INCLUDE_PATH} ${EXT_INCLUDE_PATH})
target_link_libraries(ltlf_synthesis_example ${PARSER_LIB_NAME} ${SYNTHESIS_LIB_NAME} ${UTILS_LIB_NAME} ${LYDIA_LIBRARIES})
