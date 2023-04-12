# Try to find Boost headers and libraries.
#
# Usage of this module as follows:
#
# find_package(Boost)
#
# Variables defined by this module:
#
# Boost_FOUND System has Boost libraries and headers

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions("-DHAS_BOOST")
endif()
