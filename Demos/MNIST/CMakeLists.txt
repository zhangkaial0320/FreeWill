cmake_minimum_required (VERSION 3.5)
project (FreeWill)

SET (Qt5Core_DIR /usr/lib/x86_64-linux-gnu/cmake/Qt5Core)
SET (Qt5Network_DIR /usr/lib/x86_64-linux-gnu/cmake/Qt5Network)

SET (CMAKE_PREFIX_PATH /usr/lib/x86_64-linux-gnu/qt5)
#SET (CMAKE_C_COMPILER clang)
SET (CMAKE_C_FLAGS          "-std=gnu++1z -stdlib=libstdc++ -march=x86-64 -m64 -Wno-c++1z-extensions")
#SET (CMAKE_CXX_COMPILER clang++)
SET (CMAKE_CXX_FLAGS        "-std=gnu++1z -stdlib=libstdc++ -march=x86-64 -m64 -fno-omit-frame-pointer -fPIC -I/usr/local/include -Wall -Wextra -Woverloaded-virtual -Wno-unused-local-typedefs -Wno-c++1z-extensions")

set (CMAKE_C_FLAGS          "${CMAKE_C_FLAGS}" CACHE STRING "c flags")
set (CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS}" CACHE STRING "c++ flags")
SET (CMAKE_EXPORT_COMPILE_COMMANDS 1)
set (CMAKE_EXPORT_COMPILE_COMMANDS "${CMAKE_EXPORT_COMPILE_COMMANDS}" CACHE STRING "export compile_commands.json")

SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")

option(DEBUG_PREPROCESSOR "Debug preprocessor" OFF)
SET(GCC_DEBUG_PRECOMPILER_FLAGS "-E")

if(DEBUG_PREPROCESSOR)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_DEBUG_PRECOMPILER_FLAGS}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_DEBUG_PRECOMPILER_FLAGS}")
endif(DEBUG_PREPROCESSOR)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
set (CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "build type")

SET (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")

find_package(Qt5Network REQUIRED)
include_directories(${Qt5Network_INCLUDE_DIRS})
add_definitions(${Qt5Network_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Network_EXECUTABLE_COMPILE_FLAGS}")


find_package(Qt5Test REQUIRED)
include_directories(${Qt5Test_INCLUDE_DIRS})

get_target_property(QtCore_Location Qt5::Core LOCATION)

MESSAGE("Using Qt version ${Qt5Core_VERSION} under ${QtCore_Location}")

include_directories("../../FreeWill")
include_directories("../../Utils/WebUI/Swiftly/Swiftly")
include_directories("../../Utils/WebUI/Swiftly/http-parser")
include_directories("../../Utils/WebUI/Swiftly/qt-mustache/src")
include_directories("/usr/include/x86_64-linux-gnu/qt5/QtWebSockets")

find_package(CUDA REQUIRED)
include_directories( ${CUDA_INCLUDE_DIRS} )

MESSAGE("Using Qt version ${Qt5Core_VERSION} under ${QtCore_Location}")
set(CUDA_HOST_COMPILER /usr/bin/gcc)
list(APPEND CUDA_NVCC_FLAGS "-arch=sm_52;-std=c++11;-O3;--use_fast_math;--expt-extended-lambda")
SET(CUDA_PROPAGATE_HOST_FLAGS OFF)


link_directories("/usr/local/cuda/lib64")

# get_directory_property(OUT_VAR LINK_DIRECTORIES)
# message(STATUS "DIR: ${OUT_VAR}")



set(MNIST_SOURCES 
    MNIST.cpp
    DemoBase.cpp
    DemoUI.cpp
    Session.cpp
    WebsocketServer.cpp
    main.cpp)


add_executable(MNIST ${MNIST_SOURCES})
target_link_libraries(MNIST Qt5::Core)
target_link_libraries(MNIST Qt5::Network)
target_link_libraries(MNIST Qt5WebSockets)
target_link_libraries(MNIST Swiftly)
target_link_libraries(MNIST cuda_kernel)
target_link_libraries(MNIST ${CUDA_LIBRARIES})
target_link_libraries(MNIST cudnn)
target_link_libraries(MNIST cublas)

