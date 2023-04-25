#include <MumbleAPI_v_1_2_x.h>
#include <MumblePlugin_v_1_1_x.h>

#include <SDL.h>

#include "queue.hpp"

#include <string_view>

SDL_atomic_t should_quit;
SDL_Window* main_window = NULL;
SDL_Surface* main_surface = NULL;

FrameQueue q;

struct MumbleAPI_v_1_2_x mumbleAPI;
mumble_plugin_id_t ownID;

SDL_Window* init_sdl(const char* window_title, int window_pos_x, int window_pos_y, int window_width, int window_height, uint32_t flags){
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		return NULL;
	}

	main_window = SDL_CreateWindow(window_title, window_pos_x, window_pos_y, window_width, window_height, flags);

	if(main_window == NULL){
		return NULL;
	}

	main_surface = SDL_GetWindowSurface(main_window);

	if(main_surface == NULL){
		return NULL;
	}

	SDL_FillRect(main_surface, NULL, SDL_MapRGB(main_surface->format, 0xFF, 0xAA, 0xFF));

	SDL_UpdateWindowSurface(main_window);

	return main_window;
}

void mumble_shutdown() {
	SDL_AtomicSet(&should_quit, 1);
}

MumbleStringWrapper mumble_getName() {
	static const std::string_view name {"video"};

	return {
		name.data(),
		name.size(),
		false
	};
}

MumbleStringWrapper mumble_getAuthor() {
	static const std::string_view author {"Erik Scholz & David Zero"};

	return {
		author.data(),
		author.size(),
		false
	};
}

struct MumbleStringWrapper mumble_getDescription() {
	static const std::string_view description {"Video streaming for Mumble"};

	return {
		description.data(),
		description.size(),
		false
	};
}

mumble_version_t mumble_getVersion() {
	mumble_version_t version;

	version.major = 0;
	version.minor = 1;
	version.patch = 0;

	return version;
}

mumble_version_t mumble_getAPIVersion() {
	return MUMBLE_PLUGIN_API_VERSION;
}

void mumble_registerAPIFunctions(void* apiStruct) {
	mumbleAPI = MUMBLE_API_CAST(apiStruct);
}

void mumble_releaseResource(const void *) {
}

int video_main(void* data) {
	if(!init_sdl("Muh Video Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, SDL_WINDOW_BORDERLESS)){
		return -1;
	}

	SDL_Event e;

	while (!SDL_AtomicGet(&should_quit)) {
		//Blit frame
		auto surface = q.pop();

		if (surface.has_value()) {
			// TODO(green): SDL_BlitScaled
			SDL_BlitSurface(*surface, NULL, main_surface, NULL);

			SDL_FreeSurface(*surface);

			SDL_UpdateWindowSurface(main_window);
		}

		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
				case SDL_QUIT:
					// TODO(green): destroy instead
					SDL_HideWindow(main_window);

					break;
				case SDL_WINDOWEVENT:
					switch(e.window.event) {
						case SDL_WINDOWEVENT_CLOSE:
							// TODO(green): destroy instead ?
							SDL_HideWindow(main_window);
					}

					break;
			}
		}
	}

	SDL_Quit();

	return 0;
}

mumble_error_t mumble_init(mumble_plugin_id_t pluginID) {
	ownID = pluginID;

	SDL_AtomicSet(&should_quit, 0);

	SDL_Thread* t = SDL_CreateThread(video_main, "video main thread", NULL);

	if (!t) {
		return MUMBLE_EC_GENERIC_ERROR;
	}

	SDL_DetachThread(t);

	return MUMBLE_STATUS_OK;
}

bool PLUGIN_CALLING_CONVENTION mumble_onReceiveData(mumble_connection_t connection, mumble_userid_t sender, const uint8_t* data, size_t dataLength, const char* dataID) {
	if (std::string_view{dataID} != "video-001") {
		return false;
	}

	mumbleAPI.log(ownID, "got data");

	// TODO: do something with the data

	return true;
}

