#include <SDL.h>

#include <MumbleAPI_v_1_2_x.h>
#include <MumblePlugin_v_1_1_x.h>

#include "queue.hpp"

SDL_Window* main_window = NULL;
SDL_Surface* main_surface = NULL;

extern FrameQueue q;

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

void mumble_shutdown() {}

MumbleStringWrapper mumble_getName() {
    static const char name[] = "video";

    MumbleStringWrapper wrapper;
    wrapper.data = name;
    wrapper.size = strlen(name);
    wrapper.needsReleasing = false;

    return wrapper;
}

MumbleStringWrapper mumble_getAuthor() {
    static const char author[] = "Erik Scholz & David Zero";

    MumbleStringWrapper wrapper;
    wrapper.data = author;
    wrapper.size = strlen(author);
    wrapper.needsReleasing = false;

    return wrapper;
}

struct MumbleStringWrapper mumble_getDescription() {
	static const char description[] = "Video streaming for Mumble";

	MumbleStringWrapper wrapper;
	wrapper.data = description;
	wrapper.size = strlen(description);
	wrapper.needsReleasing = false;

	return wrapper;
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

void mumble_registerAPIFunctions(void *) {}

void mumble_releaseResource(const void *) {
    SDL_Quit();
}

int video_main(void* data) {
    if(!init_sdl("Muh Video Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, SDL_WINDOW_BORDERLESS)){
        return -1;
    }

    SDL_Event e;

    while(true){
        //Blit frame
        //std::optional<SDL_Surface> surface = q.pop();

        //if(surface.has_value()){
        //    SDL_BlitSurface(surface, NULL, main_surface, NULL);

        //    SDL_UpdateWindowSurface(main_window);
        //}

        while(SDL_PollEvent(&e) != 0) {
            switch(e.type){
                case SDL_QUIT:
                    SDL_HideWindow(main_window);

                    break;
                case SDL_WINDOWEVENT:
                    switch(e.window.event){
                        case SDL_WINDOWEVENT_CLOSE:
                            SDL_HideWindow(main_window);
                    }

                    break;
            }
        }
    }
}

mumble_error_t mumble_init(uint32_t) {
    SDL_Thread* t = SDL_CreateThread(video_main, "video main thread", NULL);

    if(!t){
        return MUMBLE_EC_GENERIC_ERROR;
    }

    SDL_DetachThread(t);

    return MUMBLE_STATUS_OK;
}
