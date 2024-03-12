cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

project(ProfilerCacheRates)

add_library(cacherates SHARED cacherates.cc)
target_include_directories(cacherates PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
