cmake_minimum_required(VERSION 2.6)

project(OpenNotrium)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}")

find_package(OpenGL REQUIRED)
find_package(SDL2 2.0 REQUIRED)
find_package(SDL2_mixer 2.0 REQUIRED)
find_package(SDL2_image 2.0 REQUIRED)
find_package(PhysFS REQUIRED)
find_package(PhysFS REQUIRED)

file(GLOB OpenNotrium_sources *.cpp)

include_directories(
    ${SDL2_INCLUDE_DIR}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${PHYSFS_INCLUDE_DIR})

link_libraries(
    ${OPENGL_LIBRARY}
    ${SDL2_LIBRARY}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${PHYSFS_LIBRARY})

# Add C++11 support flags to GCC
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

add_executable(OpenNotrium ${OpenNotrium_sources})
