cmake_minimum_required(VERSION 3.14.0)
include(FetchContent)
FetchContent_Declare(
  libuv
  GIT_REPOSITORY https://github.com/libuv/libuv.git
  GIT_TAG        v1.44.2
)

FetchContent_MakeAvailable(libuv)
