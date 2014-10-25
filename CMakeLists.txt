cmake_minimum_required(VERSION 2.6)

project(OpenNotrium)

find_package(OpenGL REQUIRED)
find_package(PhysFS REQUIRED)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)

file(GLOB OpenNotrium_sources *.cpp)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2MIXER_INCLUDE_DIRS}
    ${SDL2IMAGE_INCLUDE_DIRS}
    ${PHYSFS_INCLUDE_DIR})

link_libraries(
    ${OPENGL_LIBRARY}
    ${SDL2_LIBRARIES}
    ${SDL2MIXER_LIBRARIES}
    ${SDL2IMAGE_LIBRARIES}
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
