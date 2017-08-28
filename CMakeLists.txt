cmake_minimum_required(VERSION 3.0)

# Definitions
set(PROJ sasd-protocols)
set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Project name
project(${PROJ})

# SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# pthreads
find_package(Threads)

# Include export configuration
include(${SOURCE}/netutils/lib/netutils/netutils-targets.cmake)
include_directories(${SOURCE}/netutils/src)
include_directories(${SOURCE})

# Main executable
add_executable(${PROJ} ${SOURCE}/main.c)

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

add_executable(dr_server
        ${SOURCE}/dr_server.c
        ${SOURCE}/bvec.c
        ${SOURCE}/balls/balls.c)

add_executable(game_timewarp
    ${SOURCE}/game_timewarp.c
    ${SOURCE}/balls/balls.c
    ${SOURCE}/balls/draw.c
    ${SOURCE}/vec.c)

add_executable(game_dead_reckoning
        ${SOURCE}/game_dead_reckoning.c
        ${SOURCE}/balls/balls.c
        ${SOURCE}/balls/draw.c
        ${SOURCE}/vec.c)

add_executable(game
    ${SOURCE}/game.c
    ${SOURCE}/balls/balls.c
    ${SOURCE}/balls/draw.c)

target_link_libraries(server netutils recvpool buffer ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(dr_server netutils recvpool buffer ${CMAKE_THREAD_LIBS_INIT} ${SDL2_LIBRARIES})
target_link_libraries(client netutils recvpool buffer ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(game ${SDL2_LIBRARIES})
target_link_libraries(game_timewarp ${SDL2_LIBRARIES} netutils recvpool buffer)
target_link_libraries(game_dead_reckoning ${SDL2_LIBRARIES} netutils recvpool buffer)
target_link_libraries(recvpool ${CMAKE_THREAD_LIBS_INIT})
