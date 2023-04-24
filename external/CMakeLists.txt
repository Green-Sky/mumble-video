cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

if(VCPKG_TARGET_TRIPLET)
	find_package(SDL2 CONFIG REQUIRED)
else()
	find_package(SDL2 REQUIRED)
endif()

add_subdirectory("mumble_plugin_headers")

