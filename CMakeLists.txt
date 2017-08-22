cmake_minimum_required(VERSION 3.0)

# Definitions
set(PROJ sasd-protocols)
set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Project name
project(${PROJ})

# SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Include export configuration
include(${SOURCE}/netutils/lib/netutils/netutils-targets.cmake)
include_directories(${SOURCE}/netutils/src)
include_directories(${SOURCE})

# Main executable
add_executable(${PROJ}
    ${SOURCE}/main.c
    ${SOURCE}/balls/balls.c
    ${SOURCE}/balls/draw.c)

add_library(recvpool
    ${SOURCE}/recvpool.c
    ${SOURCE}/packets.c
    ${SOURCE}/bvec.c)

add_library(buffer
    ${SOURCE}/buffer.c
)

add_executable(server
    ${SOURCE}/server.c
    ${SOURCE}/bvec.c)
add_executable(client ${SOURCE}/client.c)

add_executable(game ${SOURCE}/game.c)

target_link_libraries(server netutils recvpool buffer)
target_link_libraries(client netutils recvpool buffer)
target_link_libraries(game ${SDL2_LIBRARIES})
