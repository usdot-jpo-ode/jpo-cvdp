cmake_minimum_required(VERSION 2.6)

project(ppm)

# need to set these prior to setting any targets.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_C_FLAGS "-O3")

if (${APPLE})
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(MACPORTS_DIR "/opt")
endif ()

if(CMAKE_COMPILER_IS_GNUCXX)                                           # add coverage compiler option
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
endif()

# Build the library.
add_subdirectory("cv-lib")

# Include the cvlib headers; defined in cv-lib/CMakeLists.txt
include_directories(${CVLIB_INCLUDE})

# Include application, JSON parser headers.
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/rapidjson")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/spdlog")
include_directories("/usr/local/include")

if (${APPLE})
    include_directories( "${MACPORTS_DIR}/local/include")
    link_directories("${MACPORTS_DIR}/local/lib" "/usr/lib" "/usr/local/lib")
endif ()

#### BUILD TARGET FOR THE PPM ####

set(PPM_SRC "src/ppm.cpp" 
            "src/tool.cpp" 
            "src/bsmfilter.cpp" )

add_executable(ppm ${PPM_SRC})
target_link_libraries(ppm pthread CVLib rdkafka++)

#### BUILD TARGET FOR THE PPM UNIT TESTS AND CODE COVERAGE ####

set(PPM_TEST_SRC "src/tests.cpp")                                      # unit tests

set(CATCH_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/catch")
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})       # catch is header only; tell where to find header.


add_executable(ppm_tests ${PPM_TEST_SRC} ${PPM_SRC})                   # need everything to build tests.
target_link_libraries(ppm_tests pthread CVLib rdkafka++ Catch)
target_compile_definitions(ppm_tests PRIVATE _PPM_TESTS)               # flag to exclude the tool's main.

#### BUILD TARGET FOR THE KAFKA TEST TOOL ####

add_subdirectory(kafka-test)

# Copy the data to the build. TODO make this part of the test or data target.
set(BSM_DATA_DIR $<TARGET_FILE_DIR:ppm>/unit-test-data)
set(BSM_CONFIG_DIR $<TARGET_FILE_DIR:ppm>/config)

# Make the base data directory.
add_custom_command(TARGET ppm PRE_BUILD COMMAND ${CMAKE_COMMAND} 
                   -E make_directory ${BSM_DATA_DIR})
# Copy the data files.
add_custom_command(TARGET ppm PRE_BUILD COMMAND echo "Copying the data directory")
add_custom_command(TARGET ppm PRE_BUILD COMMAND ${CMAKE_COMMAND} 
                   -E copy_directory ${PROJECT_SOURCE_DIR}/unit-test-data
                   ${BSM_DATA_DIR})

# Make the base data directory.
add_custom_command(TARGET ppm POST_BUILD COMMAND ${CMAKE_COMMAND} 
                   -E make_directory ${BSM_CONFIG_DIR})
# Copy the data files.
add_custom_command(TARGET ppm POST_BUILD COMMAND echo "Copying the config directory")
add_custom_command(TARGET ppm POST_BUILD COMMAND ${CMAKE_COMMAND} 
                   -E copy_directory ${PROJECT_SOURCE_DIR}/config
                   ${BSM_CONFIG_DIR})

# option(BUILD_TESTS "Determines whether to build tests." ON)
# if(BUILD_TESTS)
#     enable_testing()
#     add_test(NAME mytest1 COMMAND ppm_tests)
# endif()
