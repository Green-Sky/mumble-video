#include <MumbleAPI_v_1_0_x.h>
#include <MumblePlugin_v_1_0_x.h>

#include <SDL.h>

#include "queue.hpp"

#include <string_view>
#include <array>
#include <cstring>

SDL_atomic_t should_quit;
SDL_Window* main_window = NULL;
SDL_Thread* t = NULL;

FrameQueue g_q;

struct {
	uint16_t sequence_id = 0;
	uint64_t pos_in_img = 0;
	std::array<uint8_t, 64*64*3> img_data;
} g_current_frame {};

struct MumbleAPI_v_1_0_x mumbleAPI;
mumble_plugin_id_t ownID;

extern "C" {

SDL_Window* init_sdl(const char* window_title, int window_pos_x, int window_pos_y, int window_width, int window_height, uint32_t flags){
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		return NULL;
	}

	main_window = SDL_CreateWindow(window_title, window_pos_x, window_pos_y, window_width, window_height, flags);

	if(main_window == NULL){
		return NULL;
	}

	SDL_Surface* main_surface = SDL_GetWindowSurface(main_window);

	if(main_surface == NULL){
		return NULL;
	}

	SDL_FillRect(main_surface, NULL, SDL_MapRGB(main_surface->format, 0xFF, 0xAA, 0xFF));

	SDL_UpdateWindowSurface(main_window);

	return main_window;
}

PLUGIN_EXPORT void PLUGIN_CALLING_CONVENTION mumble_shutdown() {
	SDL_AtomicSet(&should_quit, 1);
	SDL_WaitThread(t, NULL);
}

PLUGIN_EXPORT MumbleStringWrapper PLUGIN_CALLING_CONVENTION mumble_getName() {
	static const std::string_view name {"video"};

	return {
		name.data(),
		name.size(),
		false
	};
}

PLUGIN_EXPORT MumbleStringWrapper PLUGIN_CALLING_CONVENTION mumble_getAuthor() {
	static const std::string_view author {"Erik Scholz & David Zero"};

	return {
		author.data(),
		author.size(),
		false
	};
}

PLUGIN_EXPORT struct MumbleStringWrapper PLUGIN_CALLING_CONVENTION mumble_getDescription() {
	static const std::string_view description {"Video streaming for Mumble"};

	return {
		description.data(),
		description.size(),
		false
	};
}

PLUGIN_EXPORT mumble_version_t PLUGIN_CALLING_CONVENTION mumble_getVersion() {
	mumble_version_t version;

	version.major = 0;
	version.minor = 1;
	version.patch = 0;

	return version;
}

PLUGIN_EXPORT mumble_version_t PLUGIN_CALLING_CONVENTION mumble_getAPIVersion() {
	return MUMBLE_PLUGIN_API_VERSION;
}

PLUGIN_EXPORT void PLUGIN_CALLING_CONVENTION mumble_registerAPIFunctions(void* apiStruct) {
	mumbleAPI = MUMBLE_API_CAST(apiStruct);
}

PLUGIN_EXPORT void PLUGIN_CALLING_CONVENTION mumble_releaseResource(const void *) {
}

int video_main(void* data) {
	if(!init_sdl("Muh Video Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, SDL_WINDOW_SHOWN)){
		return -1;
	}

	SDL_Event e;

	while (!SDL_AtomicGet(&should_quit)) {
		//Blit frame
		auto surface = g_q.pop();

		if (surface.has_value()) {
			SDL_Surface* main_surface = SDL_GetWindowSurface(main_window);
			SDL_BlitScaled(*surface, NULL, main_surface, NULL);
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

		SDL_Delay(5);
	}

	SDL_DestroyWindow(main_window);

	SDL_Quit();

	return 0;
}

PLUGIN_EXPORT mumble_error_t PLUGIN_CALLING_CONVENTION mumble_init(mumble_plugin_id_t pluginID) {
	ownID = pluginID;

	SDL_AtomicSet(&should_quit, 0);

	t = SDL_CreateThread(video_main, "video main thread", NULL);

	if (!t) {
		return MUMBLE_EC_GENERIC_ERROR;
	}

	return MUMBLE_STATUS_OK;
}

PLUGIN_EXPORT bool PLUGIN_CALLING_CONVENTION mumble_onReceiveData(mumble_connection_t connection, mumble_userid_t sender, const uint8_t* data, size_t dataLength, const char* dataID) {
	if (std::string_view{dataID} != "video-001") {
		return false;
	}

	mumbleAPI.log(ownID, "got data");

	if (dataLength < 2+8+8) {
		mumbleAPI.log(ownID, "data too short");
	}

	// 64x64 3bpp
	const size_t img_size = 64 * 64 * 3; // hardcoded for "video-001"

	size_t curr_pos = 0; // index in pkg data

	uint16_t sequence_id = 0;
	{ // sequence_id
		sequence_id |= data[curr_pos++] << 8*0;
		sequence_id |= data[curr_pos++] << 8*1;
	}

	if (sequence_id != g_current_frame.sequence_id) {
		// discard old data
		g_current_frame.sequence_id = sequence_id;
		g_current_frame.pos_in_img = 0;
	}

	uint64_t pos_in_img = 0;
	{ // position in picture
		pos_in_img |= uint64_t(data[curr_pos++]) << 8*0;
		pos_in_img |= uint64_t(data[curr_pos++]) << 8*1;
		pos_in_img |= uint64_t(data[curr_pos++]) << 8*2;
		pos_in_img |= uint64_t(data[curr_pos++]) << 8*3;
		pos_in_img |= uint64_t(data[curr_pos++]) << 8*4;
		pos_in_img |= uint64_t(data[curr_pos++]) << 8*5;
		pos_in_img |= uint64_t(data[curr_pos++]) << 8*6;
		pos_in_img |= uint64_t(data[curr_pos++]) << 8*7;
	}

	if (g_current_frame.pos_in_img != pos_in_img) {
		// out of order data, drop
		g_current_frame.pos_in_img = 0;
		mumbleAPI.log(ownID, "out of order data, dropping (check your allowed throughput)");
	}

	{ // add data to tmp buffer
		const size_t img_data_size = dataLength-curr_pos;
		if (img_data_size == 0) {
			mumbleAPI.log(ownID, "no img data??");
			return true;
		}

		if (img_data_size+pos_in_img > g_current_frame.img_data.size()) {
			mumbleAPI.log(ownID, "img data too big??");
			return true;
		}

		std::memcpy(g_current_frame.img_data.data() + pos_in_img, data + curr_pos, img_data_size);

		g_current_frame.pos_in_img += img_data_size;
	}

	// frame complete
	if (g_current_frame.pos_in_img == g_current_frame.img_data.size()) {
		// push new frame
		auto new_frame = SDL_CreateRGBSurfaceWithFormat(
			0, // reseved
			64, 64, // w / h
			SDL_BITSPERPIXEL(SDL_PIXELFORMAT_RGB888), SDL_PIXELFORMAT_RGB888 // bpp / format (8+8+8=24)
		);
		//SDL_FillRect(new_frame, NULL, SDL_MapRGB(new_frame->format, 0xFF, 0x00, 0xFF));

		// this call converts the 3bytes per pixel to internal 4bytes per pixel!
		auto tmp_frame = SDL_CreateRGBSurfaceFrom(g_current_frame.img_data.data(), 64, 64, 24, 64*3, 0x0000ff, 0x00ff00, 0xff0000, 0);

		//SDL_LockSurface(new_frame);
		//std::memcpy(new_frame->pixels, g_current_frame.img_data.data(), g_current_frame.img_data.size());
		//SDL_UnlockSurface(new_frame);
		SDL_BlitSurface(tmp_frame, nullptr, new_frame, nullptr);

		SDL_FreeSurface(tmp_frame);

		g_q.push(new_frame);
	}

	return true;
}

} // extern "C"

