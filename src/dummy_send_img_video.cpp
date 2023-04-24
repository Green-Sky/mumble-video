#include <MumbleAPI_v_1_2_x.h>
#include <MumblePlugin_v_1_1_x.h>

#include <cstring>

mumble_error_t mumble_init(uint32_t) {
    return MUMBLE_STATUS_OK;

    return MUMBLE_EC_GENERIC_ERROR;
}

void mumble_shutdown() {}

MumbleStringWrapper mumble_getName() {
    static const char name[] = "video-dummy_send_img";

    MumbleStringWrapper wrapper;
    wrapper.data           = name;
    wrapper.size           = std::strlen(name);
    wrapper.needsReleasing = false;

    return wrapper;
}

mumble_version_t mumble_getAPIVersion() {
    return MUMBLE_PLUGIN_API_VERSION;
}

void mumble_registerAPIFunctions(void *) {}

void mumble_releaseResource(const void *) {}
