cmake_minimum_required(VERSION 2.6)
project(CVLIB)

# The version number.
set(CVLIB CVLIB)
set(CVLIB_VERSION_MAJOR 1)
set(CVLIB_VERSION_MINOR 0)
set(CVLIB_VERSION "${CVLIB_VERSION_MAJOR}.${CVLIB_VERSION_MINOR}")

set(CVLIB_CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CVLIB_INCLUDE_DIR "${CVLIB_CURRENT_DIR}/include")
set(CVLIB_OUT_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")

# set(CVLIB_TEST_DIR test)
# set(CVLIB_TOOL_DIR tool)

# Configure and copy the headers.
configure_file("${CVLIB_CURRENT_DIR}/cvlib.hpp.in" "${CVLIB_OUT_INCLUDE_DIR}/cvlib.hpp")
configure_file("${CVLIB_INCLUDE_DIR}/shapes.hpp" "${CVLIB_OUT_INCLUDE_DIR}/shapes.hpp" COPYONLY)
configure_file("${CVLIB_INCLUDE_DIR}/entity.hpp" "${CVLIB_OUT_INCLUDE_DIR}/entity.hpp" COPYONLY)
configure_file("${CVLIB_INCLUDE_DIR}/names.hpp" "${CVLIB_OUT_INCLUDE_DIR}/names.hpp" COPYONLY)
configure_file("${CVLIB_INCLUDE_DIR}/osm.hpp" "${CVLIB_OUT_INCLUDE_DIR}/osm.hpp" COPYONLY)
configure_file("${CVLIB_INCLUDE_DIR}/quad.hpp" "${CVLIB_OUT_INCLUDE_DIR}/quad.hpp" COPYONLY)
configure_file("${CVLIB_INCLUDE_DIR}/utilities.hpp" "${CVLIB_OUT_INCLUDE_DIR}/utilities.hpp" COPYONLY)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Just include the location where everything is copied to.
include_directories(${CVLIB_OUT_INCLUDE_DIR})

# Tell all
set(CVLIB_INCLUDE ${CVLIB_OUT_INCLUDE_DIR} PARENT_SCOPE)

# include_directories(${PROJECT_BINARY_DIR})
# include_directories(${CVLIB_INCLUDE_DIR})

set(CVLIB_SRC "src/quad.cpp" 
              "src/utilities.cpp" 
              "src/osm.cpp" 
              "src/entity.cpp" 
              "src/shapes.cpp")

# Make the library.
add_library(CVLib STATIC ${CVLIB_SRC})
set_target_properties(CVLib PROPERTIES POSITION_INDEPENDENT_CODE ON)

