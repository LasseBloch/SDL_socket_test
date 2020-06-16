cmake_minimum_required(VERSION 3.15)
project(SDL_SocketTest)

set(CMAKE_CXX_STANDARD 20)

add_executable(SDL_SocketServer src/server/testServer.cpp src/common/message.h src/common/rect.h)

add_executable(SDL_SocketClient src/client/testClient.cpp src/common/message.h src/common/rect.h)