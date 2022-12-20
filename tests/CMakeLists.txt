cmake_minimum_required(VERSION 3.14.0)
project(remote_dictionary_test VERSION 1.0.0)

add_executable(${PROJECT_NAME}_client $<TARGET_OBJECTS:remote_dictionary_client_objects> "client_tests.cpp" "client_tests_main.cpp" "stream_test.h")
add_executable(${PROJECT_NAME}_server $<TARGET_OBJECTS:remote_dictionary_server_objects> "server_tests.cpp" "server_tests_main.cpp" "stream_test.h")

target_link_libraries(${PROJECT_NAME}_server PUBLIC remote_dictionary_server_objects gtest ev::ev)
target_link_libraries(${PROJECT_NAME}_client PUBLIC remote_dictionary_client_objects gtest ev::ev)

include_directories( SYSTEM
    ${GTest_SOURCE_DIR}/include
)

target_compile_features(${PROJECT_NAME}_client PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME}_server PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME}_client PRIVATE  -Wall -Wextra -Wpedantic  -Wno-vla -Wno-unused-local-typedefs)
target_compile_options(${PROJECT_NAME}_server PRIVATE  -Wall -Wextra -Wpedantic  -Wno-vla -Wno-unused-local-typedefs)

