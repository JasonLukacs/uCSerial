cmake_minimum_required(VERSION 3.15)
project(uCSerial)

set(CMAKE_CXX_STANDARD 17)

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
    lib/JSONParser/src/jsonparser.cpp
    src/msgparser/msgparser.cpp
    src/serialreader/serialreader_macos.cpp
    src/utils/utils_macos.cpp
    main.cpp
    )
else ()
  message(FATAL_ERROR "This project currently only supports MacOS.")
endif()