# Try to find MONA headers and libraries.
#
# Usage of this module as follows:
#
# find_package(MONA)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
# MONA_ROOT Set this variable to the root installation of
# libmona if the module has problems finding the
# proper installation path.
#
# Variables defined by this module:
#
# MONA_FOUND System has MONA libraries and headers
# MONA_LIBRARIES The MONA library
# MONA_INCLUDE_DIRS The location of MONA headers

# Get hint from environment variable (if any)
if(NOT MONA_ROOT AND DEFINED ENV{MONA_ROOT})
    set(MONA_ROOT "$ENV{MONA_ROOT}" CACHE PATH "MONA base directory location (optional, used for nonstandard installation paths)")
    mark_as_advanced(MONA_ROOT)
endif()

#set(MONA_ROOT "$ENV{HOME}/install")
set(MONA_ROOT "/usr/local")
# Search path for nonstandard locations
if(MONA_ROOT)
    set(MONA_INCLUDE_PATH PATHS "${MONA_ROOT}/include/mona" NO_DEFAULT_PATH)
    set(MONA_LIBRARY_PATH PATHS "${MONA_ROOT}/lib" NO_DEFAULT_PATH)
endif()

function(_find_mona_lib prefix_name header_names lib_names include_path_hints library_path_hints)

    # Search for mona_mem
    find_path(${prefix_name}_INCLUDE_DIRS NAMES ${header_names} HINTS ${MONA_INCLUDE_PATH})
    find_library(${prefix_name}_LIBRARIES NAMES ${lib_names} HINTS ${MONA_LIBRARY_PATH})

    include(FindPackageHandleStandardArgs)

    find_package_handle_standard_args(mona DEFAULT_MSG ${prefix_name}_LIBRARIES ${prefix_name}_INCLUDE_DIRS)

    mark_as_advanced(MONA_ROOT ${prefix_name}_LIBRARIES ${prefix_name}_INCLUDE_DIRS)

endfunction()

_find_mona_lib("MONA_MEM" "mem.h" "monamem" "${MONA_INCLUDE_PATH}" "${MONA_LIBRARY_PATH}")
_find_mona_lib("MONA_BDD" "bdd.h" "monabdd" "${MONA_INCLUDE_PATH}" "${MONA_LIBRARY_PATH}")
_find_mona_lib("MONA_DFA" "dfa.h" "monadfa" "${MONA_INCLUDE_PATH}" "${MONA_LIBRARY_PATH}")
