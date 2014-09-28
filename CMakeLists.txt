cmake_minimum_required(VERSION 2.4)
project(OpenNotrium)

find_package(OpenGL REQUIRED)
find_package(SDL 1.2 REQUIRED)
find_package(SDL_mixer 1.2 REQUIRED)
find_package(SDL_image 1.2 REQUIRED)
find_package(PhysFS REQUIRED)

file(GLOB OpenNotrium_sources *.cpp)

include_directories(
    ${SDL_INCLUDE_DIR}
    ${SDL_MIXER_INCLUDE_DIR}
    ${SDL_IMAGE_INCLUDE_DIRS})

link_libraries(
    ${OPENGL_LIBRARY}
    ${SDL_LIBRARY}
    ${SDL_MIXER_LIBRARY}
    ${SDL_IMAGE_LIBRARIES}
    ${PHYSFS_LIBRARY})

add_executable(OpenNotrium ${OpenNotrium_sources})
