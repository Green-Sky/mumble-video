cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(video SHARED
	./queue.hpp
	./video-main.cpp
)

target_compile_features(video PUBLIC cxx_std_17)

target_link_libraries(video PUBLIC mumble_plugin)

if(VCPKG_TARGET_TRIPLET)
	target_link_libraries(video
		PUBLIC
			SDL2::SDL2
			SDL2::SDL2main
			#SDL2::SDL2-static
	)
else()
	target_include_directories(video PUBLIC "${SDL2_INCLUDE_DIR}")
	target_link_libraries(video PUBLIC ${SDL2_LIBRARY})
endif()

########################################

add_library(dummy_send_img_video SHARED
	./dummy_send_img_video.cpp
)

target_compile_features(dummy_send_img_video PUBLIC cxx_std_11)

target_link_libraries(dummy_send_img_video PUBLIC mumble_plugin)

