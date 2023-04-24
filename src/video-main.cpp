#include <SDL.h>

#include <MumbleAPI_v_1_2_x.h>
#include <MumblePlugin_v_1_1_x.h>

SDL_Window* main_window = NULL;
SDL_Surface* main_surface = NULL;

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

    SDL_FillRect(main_surface, NULL, SDL_MapRGB(main_surface->format, 0xFF, 0x45, 0xFF));

    SDL_UpdateWindowSurface(main_window);

    return main_window;
}

mumble_error_t mumble_init(uint32_t) {
	if(init_sdl("Muh Video Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 400, SDL_WINDOW_BORDERLESS)){
        return MUMBLE_STATUS_OK;
	}

    return MUMBLE_EC_GENERIC_ERROR;
}

void mumble_shutdown() {}

MumbleStringWrapper mumble_getName() {
    static const char name[] = "video";

    MumbleStringWrapper wrapper;
    wrapper.data           = name;
    wrapper.size           = strlen(name);
    wrapper.needsReleasing = false;

    return wrapper;
}

mumble_version_t mumble_getAPIVersion() {
    return MUMBLE_PLUGIN_API_VERSION;
}

void mumble_registerAPIFunctions(void *) {}

void mumble_releaseResource(const void *) {}
