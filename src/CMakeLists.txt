cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

add_executable(sdl_main_test
	./sdl_main_test.cpp
)

if(VCPKG_TARGET_TRIPLET)
	find_package(SDL2 CONFIG REQUIRED)

	target_link_libraries(sdl_main_test
		PUBLIC
			SDL2::SDL2
			SDL2::SDL2main
			#SDL2::SDL2-static
	)
else()
	find_package(SDL2 REQUIRED)
	target_include_directories(sdl_main_test PUBLIC "${SDL2_INCLUDE_DIR}")
	target_link_libraries(sdl_main_test PUBLIC ${SDL2_LIBRARY})
endif()

