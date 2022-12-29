cmake_minimum_required(VERSION 3.14.0)
project(remote_dictionary VERSION 1.0.0)

include("${CMAKE_SOURCE_DIR}/third_party/rapidjson.cmake")
include("${CMAKE_SOURCE_DIR}/third_party/cli11.cmake")
include("${CMAKE_SOURCE_DIR}/third_party/libev.cmake")
include_directories( SYSTEM
    ${rapidjson_SOURCE_DIR}/include
)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
find_package(Threads)
find_package(RapidJSON REQUIRED QUIET)

file(GLOB_RECURSE CPP_FILES    "source/network/*.cpp" "source/json/*.cpp")
file(GLOB_RECURSE H_FILES      "include/network/*.h" "include/json/*.h")
file(GLOB_RECURSE SERVER_CPP_FILES    "source/server/*.cpp")
file(GLOB_RECURSE SERVER_H_FILES      "include/server/*.h")
file(GLOB_RECURSE CLIENT_CPP_FILES    "source/client/*.cpp")
file(GLOB_RECURSE CLIENT_H_FILES      "include/client/*.h")

add_library(${PROJECT_NAME}_server_objects OBJECT ${CPP_FILES} ${H_FILES} ${SERVER_CPP_FILES} ${SERVER_H_FILES})
add_library(${PROJECT_NAME}_client_objects OBJECT ${CPP_FILES} ${H_FILES} ${CLIENT_CPP_FILES} ${CLIENT_H_FILES})
add_executable(${PROJECT_NAME}_server $<TARGET_OBJECTS:${PROJECT_NAME}_server_objects> "server_main.cpp" )
add_executable(${PROJECT_NAME}_client $<TARGET_OBJECTS:${PROJECT_NAME}_client_objects> "client_main.cpp" )

target_compile_options(${PROJECT_NAME}_client PRIVATE  -Wall -Wextra -Wpedantic  -Wno-vla -Wno-unused-local-typedefs)
target_compile_options(${PROJECT_NAME}_server PRIVATE  -Wall -Wextra -Wpedantic  -Wno-vla -Wno-unused-local-typedefs)

option(ASAN "Enable AddressSanitizer (ASan)" OFF)

if(MSAN AND NOT CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang")
  message(SEND_ERROR "MemorySanitizer requires clang. Try again with -DCMAKE_C_COMPILER=clang")
endif()

if(ASAN)
  list(APPEND uv_defines __ASAN__=1)
  if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|GNU|Clang")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
  elseif(MSVC)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fsanitize=address")
  else()
    message(SEND_ERROR "AddressSanitizer support requires clang, gcc, or msvc. Try again with -DCMAKE_C_COMPILER.")
  endif()
endif()


target_compile_features(${PROJECT_NAME}_server_objects PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME}_client_objects PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME}_server PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME}_client PUBLIC cxx_std_17)

option(BUILD_TESTS "Build test programs" OFF)
if(BUILD_TESTS)
    include("${CMAKE_SOURCE_DIR}/third_party/gtest.cmake")
    add_subdirectory(tests)
endif(BUILD_TESTS)

target_link_libraries(${PROJECT_NAME}_server_objects ev::ev)
target_link_libraries(${PROJECT_NAME}_client_objects ev::ev)

target_link_libraries(${PROJECT_NAME}_server Threads::Threads CLI11::CLI11 ev::ev)
target_link_libraries(${PROJECT_NAME}_client Threads::Threads CLI11::CLI11 ev::ev)

target_include_directories(${PROJECT_NAME}_server PUBLIC
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
    )
