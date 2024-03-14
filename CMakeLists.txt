cmake_minimum_required(VERSION 3.20.0)

project(SafeBrowserSample)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(SafeBrowserSample WIN32 ./src/main.cpp)


find_package(unofficial-webview2 CONFIG REQUIRED)
target_link_libraries(SafeBrowserSample PRIVATE unofficial::webview2::webview2)

