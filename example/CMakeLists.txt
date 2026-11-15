cmake_minimum_required(VERSION 3.24)
project(SimpleSACN_Example)

include(FetchContent)
FetchContent_Declare(
    asio 
    GIT_REPOSITORY  https://github.com/chriskohlhoff/asio.git 
    GIT_TAG         asio-1-39-0
)
FetchContent_MakeAvailable(asio)

add_executable(SimpleSACN_Example src/main.cpp)
target_include_directories(SimpleSACN_Example PRIVATE ${asio_SOURCE_DIR}/include ../src)
target_compile_definitions(SimpleSACN_Example PRIVATE ASIO_STANDALONE _WIN32_WINNT=0x0A00)
