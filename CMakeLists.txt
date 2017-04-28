cmake_minimum_required(VERSION 2.6)

project(bsmjson_privacy)

# need to set these prior to setting any targets.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "-O3")
# set(CMAKE_C_FLAGS "-O3")

if (${APPLE})
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(MACPORTS_DIR "/opt")
endif ()

# Build the library.
add_subdirectory("cv-lib")

# Incldue the cvlib headers.
include_directories(${CVLIB_INCLUDE})

# Include application and JSON parser headers.
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/rapidjson")

if (${APPLE})
    include_directories( "${MACPORTS_DIR}/local/include")
    link_directories("${MACPORTS_DIR}/local/lib" "/usr/lib" "/usr/local/lib")
endif ()

add_executable(bsmjson_privacy "src/kafka_consumer.cpp" "src/bsmfilter.cpp")
# Link with cvlib.
target_link_libraries(bsmjson_privacy CVLib rdkafka++)

# Add the kafka-test tool.
add_subdirectory(kafka-test)

# Copy the data to the build. TODO make this part of the test or data target.
set(BSM_DATA_DIR $<TARGET_FILE_DIR:bsmjson_privacy>/data)
# Make the base chrome-app directory.
add_custom_command(TARGET bsmjson_privacy POST_BUILD COMMAND ${CMAKE_COMMAND} 
                   -E make_directory ${BSM_DATA_DIR})
# Copy the app files.
add_custom_command(TARGET bsmjson_privacy POST_BUILD COMMAND ${CMAKE_COMMAND} 
                   -E copy_directory ${PROJECT_SOURCE_DIR}/data 
                   ${BSM_DATA_DIR})
