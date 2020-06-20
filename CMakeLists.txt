cmake_minimum_required(VERSION 3.15)
project(SDL_SocketTest)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


add_executable(SDL_SocketServer src/server/testServer.cpp src/common/message.h src/common/rect.h)
target_link_libraries(SDL_SocketServer ${SDL2_LIBRARIES})
add_executable(SDL_SocketClient src/client/testClient.cpp src/common/message.h src/common/rect.h)
target_link_libraries(SDL_SocketClient ${SDL2_LIBRARIES})