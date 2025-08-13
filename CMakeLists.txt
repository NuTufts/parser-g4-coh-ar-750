cmake_minimum_required(VERSION 3.10)
project(g4_coh_ar_750_parsers VERSION 0.1)

# Set C++ standard based on ROOT's configuration
execute_process(
    COMMAND bash "-c" "root-config --features | awk '{print $1}'"
    OUTPUT_VARIABLE PARSE_ROOT_CXX_STANDARD )
if (${PARSE_ROOT_CXX_STANDARD} MATCHES "cxx17")
  message( STATUS "using cxx17")
  set(CMAKE_CXX_STANDARD 17)
elseif (${PARSE_ROOT_CXX_STANDARD} MATCHES "cxx14")
  message(STATUS "using cxx14")
  set(CMAKE_CXX_STANDARD 14)
else ()
  message( STATUS "using cxx11")
  set(CMAKE_CXX_STANDARD 11)
endif()

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find ROOT (required)
find_package(ROOT CONFIG REQUIRED COMPONENTS Core Tree RIO Hist)
if(ROOT_FOUND)
  include(${ROOT_USE_FILE})
  message(STATUS "Found ROOT: ${ROOT_VERSION}")
endif()

# Find the cenns package - this should be built and installed first
# Try to find cenns using environment variable first, then fallback to relative path
if(DEFINED ENV{CENNS10_LIBDIR})
    set( cenns_DIR $ENV{CENNS10_LIBDIR}/cmake/cenns/ )
    message(STATUS "Using cenns_DIR from environment: ${cenns_DIR}")
else()
    message(FATAL_ERROR "Cannot find cenns installation. Please build cenns first and/or setup cenns environment.")
endif()

find_package(cenns REQUIRED)
if(cenns_FOUND)
  message(STATUS "Found cenns package")
endif()

# Set runtime path for executables to find libraries
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(APPLE)
  set(CMAKE_MACOSX_RPATH TRUE)
  set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
else()
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

# Add subdirectories for individual parsers
add_subdirectory(src)

# Print configuration summary
message(STATUS "===================================")
message(STATUS "Build Configuration Summary:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  ROOT Version: ${ROOT_VERSION}")
message(STATUS "===================================")
