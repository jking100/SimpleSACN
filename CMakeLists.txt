cmake_minimum_required(VERSION 3.21)
project(SimpleSACN VERSION 0.2.0 LANGUAGES CXX)

add_library(SimpleSACN INTERFACE)
add_library(SimpleSACN::SimpleSACN ALIAS SimpleSACN)

target_include_directories(SimpleSACN INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(SimpleSACN INTERFACE cxx_std_17)

# std::mutex required for 'src/Node/SafeNode.hpp'
find_package(Threads REQUIRED)
target_link_libraries(SimpleSACN INTERFACE Threads::Threads)

# test harness
# ctest --test-dir build --output-on-failure
if(PROJECT_IS_TOP_LEVEL)
    enable_testing()

    file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/test_*.cpp)
    foreach(src ${test_sources})
        get_filename_component(name ${src} NAME_WE)
        add_executable(${name} ${src})
        target_link_libraries(${name} PRIVATE SimpleSACN)
        # -UNDEBUG builds assert() in always
        target_compile_options(${name} PRIVATE -Wall -Wextra -UNDEBUG)
        add_test(NAME ${name} COMMAND ${name})
    endforeach()
endif()
