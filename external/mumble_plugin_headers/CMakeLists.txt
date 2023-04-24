cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

add_library(mumble_plugin INTERFACE)

target_include_directories(mumble_plugin INTERFACE ".")

