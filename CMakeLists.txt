cmake_minimum_required(VERSION 4.0)
project(SDL3_CR)

set(CMAKE_CXX_STANDARD 20)

#include manual - caile corecte
include_directories("C:/vcpkg/installed/x64-windows/include")
link_directories("C:/vcpkg/installed/x64-windows/lib")

add_executable(SDL3_CR main.cpp circle.h circle.cpp)

#link manual - doar numele bibliotecilor, fără .lib
target_link_libraries(SDL3_CR
        SDL3
)
#glm este header-only, nu trebuie linked