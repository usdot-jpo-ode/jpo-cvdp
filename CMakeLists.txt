# Minimum required version of CMake
cmake_minimum_required(VERSION 2.6)

# Project name
project(ppm)

# Set C++ standard to 11 and make it required
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags
set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_C_FLAGS "-O3")

# Set options for macOS
if (${APPLE})
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(MACPORTS_DIR "/opt")
endif ()

# Add coverage compiler option for GNU C++
if(CMAKE_COMPILER_IS_GNUCXX)
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
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/general-redaction")
include_directories("/usr/local/include")

# Add include directory for macOS
if (${APPLE})
    include_directories("${MACPORTS_DIR}/local/include")
    link_directories("${MACPORTS_DIR}/local/lib" "/usr/lib" "/usr/local/lib")
endif ()

#### Build target for the PPM
# List all the source files in project
set(SOURCES
    "src/general-redaction/redactionPropertiesManager.cpp"
    "src/general-redaction/rapidjsonRedactor.cpp"
    "src/bsm.cpp"
    "src/bsmHandler.cpp"
    "src/idRedactor.cpp"
    "src/tool.cpp"
    "src/velocityFilter.cpp"
    "src/ppmLogger.cpp"
)

# Create a library target for the shared sources
add_library(ppm-lib STATIC ${SOURCES})

# Link the library target with the Kafka libraries
target_link_libraries(ppm-lib PUBLIC 
    rdkafka
    rdkafka++ 
)

#### Create a target for the PPM executable
add_executable(ppm "src/ppm.cpp")

# Link the PPM executable with the PPM library target
target_link_libraries(ppm PUBLIC ppm-lib CVLib)

#### Create a target for the Kafka consumer executable
add_executable(kafka_consumer "src/kafka_consumer.cpp")

# Link the Kafka consumer executable with the PPM library target and the Kafka libraries
target_link_libraries(kafka_consumer PUBLIC ppm-lib 
    rdkafka
    rdkafka++ 
    CVLib
)

#### Build target for the PPM unit tests and code coverage
set(PPM_TEST_SRC "src/tests.cpp")   # unit tests

# Include the Catch header-only test framework
set(CATCH_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/catch")
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# Build the tests executable
add_executable(ppm_tests ${PPM_TEST_SRC} ${SOURCES})
target_link_libraries(ppm_tests pthread CVLib rdkafka++ Catch)
target_compile_definitions(ppm_tests PRIVATE _PPM_TESTS)

#### Build target for the Kafka test tool
add_subdirectory(kafka-test)

# Copy the data and config directories to the build directory
set(BSM_DATA_DIR $<TARGET_FILE_DIR:ppm>/unit-test-data)
set(BSM_CONFIG_DIR $<TARGET_FILE_DIR:ppm>/config)

# Make the base data directory and copy the data files
add_custom_command(TARGET ppm PRE_BUILD COMMAND ${CMAKE_COMMAND} 
                   -E make_directory ${BSM_DATA_DIR})
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
