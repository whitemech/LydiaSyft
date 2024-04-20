cmake_minimum_required(VERSION 3.5)
project(LydiaSyft)

include(FetchContent)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions( -D__ROOT_DIRECTORY="${CMAKE_SOURCE_DIR}" )

set(CMAKE_MODULE_PATH
    "${CMAKE_MODULE_PATH}"
    "${CMAKE_ROOT_DIR}/cmake/Modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

# TODO: make it configurable
#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if (NOT DEFINED CUDD_USE_STATIC_LIBS)
  set(CUDD_USE_STATIC_LIBS ON)
endif()
find_package(cudd REQUIRED)

if (NOT DEFINED MONA_USE_STATIC_LIBS)
  set(MONA_USE_STATIC_LIBS ON)
endif()
find_package(mona REQUIRED)


if (NOT DEFINED Z3_FETCH)
  set(Z3_FETCH ON)
endif()

if (Z3_FETCH)
  message("-- since Z3_FETCH is ${Z3_FETCH}, the Z3 project will be fetched and built from source")
  set(Z3_BUILD_LIBZ3_SHARED OFF)
  FetchContent_Declare(z3
      GIT_REPOSITORY https://github.com/Z3Prover/z3
      GIT_TAG        z3-4.8.12
      )
  FetchContent_MakeAvailable(z3)
  set(Z3_INCLUDE_DIR ${z3_SOURCE_DIR}/src/api/c++)
  set(Z3_LIBRARY libz3)
else()
  message("-- since Z3_FETCH is ${Z3_FETCH}, the Z3 project is assumed to be found in the local filesystem")
  find_package(Z3 REQUIRED)
endif()


set(LYDIA_DIR ${PROJECT_SOURCE_DIR}/submodules/lydia)
set(WITH_SYFT OFF)
set(LYDIA_ENABLE_TESTS OFF)
set(LYDIA_ENABLE_BENCHMARK OFF)
add_subdirectory(${LYDIA_DIR})
set(LYDIA_LIBRARIES lydia)

set(LYDIA_INCLUDE_DIR ${LYDIA_DIR}/lib/include)
set(LYDIA_THIRD_PARTY_INCLUDE_PATH "${LYDIA_DIR}/third_party/spdlog/include;${LYDIA_DIR}/third_party/CLI11/include;/usr/local/include;/usr/local/include;/usr/local/include;${LYDIA_DIR}/third_party/google/benchmark/include;/usr/include")

set(EXT_LIBRARIES_PATH lydia ${CUDD_LIBRARIES} ${MONA_DFA_LIBRARIES} ${MONA_BDD_LIBRARIES} ${MONA_MEM_LIBRARIES} ${Z3_LIBRARY})
set(EXT_INCLUDE_PATH ${LYDIA_INCLUDE_DIR} ${LYDIA_THIRD_PARTY_INCLUDE_PATH} ${CUDD_INCLUDE_DIRS} ${MONA_MEM_INCLUDE_DIRS} ${MONA_BDD_INCLUDE_DIRS} ${MONA_DFA_INCLUDE_DIRS} ${Z3_INCLUDE_DIR})

message(STATUS EXT_LIBRARIES_PATH ${EXT_LIBRARIES_PATH})

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
