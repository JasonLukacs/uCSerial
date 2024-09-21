cmake_minimum_required(VERSION 3.15)
project(uCSerial)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Release)

include_directories(
  lib/valijson/include
  lib/rapidjson/include
  lib/JSONParser/include
  include
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)


if(APPLE)
  add_executable(serialreader    
    main.cpp
    
    src/msgparser/msgparser.cpp
    src/msgparser/parserengine.cpp
    src/serialreader/serialreader_macos.cpp
    src/utils/utils_macos.cpp

    lib/JSONParser/src/jsonparser.cpp  

    include/uCSerial/serialreader/serial_config_schema.json
    config/serialconfig.json

    )
else ()
  message(FATAL_ERROR "This project currently only supports MacOS.")
endif()

# Add non-code files:
# Copy non-C++ files to the build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/schema/)

add_custom_command(
    TARGET serialreader
    PRE_BUILD
    COMMAND cp ../include/uCSerial/serialreader/serial_config_schema.json ${CMAKE_BINARY_DIR}/bin/schema/
    COMMAND cp ../config/serialconfig.json ${CMAKE_BINARY_DIR}/bin
    DEPENDS ../include/uCSerial/serialreader/serial_config_schema.json
    DEPENDS ./config/serialconfig.json
)