cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

add_library(video SHARED
	./video-main.cpp
)

target_link_libraries(video PUBLIC mumble_plugin)

if(VCPKG_TARGET_TRIPLET)
	find_package(SDL2 CONFIG REQUIRED)

	target_link_libraries(video
		PUBLIC
			SDL2::SDL2
			SDL2::SDL2main
			#SDL2::SDL2-static
	)
else()
	find_package(SDL2 REQUIRED)
	target_include_directories(video PUBLIC "${SDL2_INCLUDE_DIR}")
	target_link_libraries(video PUBLIC ${SDL2_LIBRARY})
endif()

