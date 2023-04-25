#include <MumbleAPI_v_1_2_x.h>
#include <MumblePlugin_v_1_1_x.h>
#include <PluginComponents_v_1_0_x.h>

#include <cstring>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

// exposes:
//  unsigned char test_png_64x64_png[];
//  unsigned int test_png_64x64_png_len;
//#include "../res/test_png_64x64.png.c"


// exposes:
//  test_64x64
#include "../res/test_png_64x64.c"

struct MumbleAPI_v_1_2_x mumbleAPI;
mumble_plugin_id_t ownID;

std::atomic_bool main_should_quit;
std::thread main_thread;

// fwd
void main_loop(void);

mumble_error_t mumble_init(mumble_plugin_id_t pluginID) {
	ownID = pluginID;

	if (mumbleAPI.log(ownID, "dummy_send_img") != MUMBLE_STATUS_OK) {
		// Logging failed -> usually you'd probably want to log things like this in your plugin's
		// logging system (if there is any)
		return MUMBLE_EC_GENERIC_ERROR;
	}

	// start main thread
	main_should_quit = false;
	main_thread = std::thread{main_loop};

	if (mumbleAPI.log(ownID, "started main thread") != MUMBLE_STATUS_OK) {
		return MUMBLE_EC_GENERIC_ERROR;
	}

	return MUMBLE_STATUS_OK;
}

void mumble_shutdown() {
	main_should_quit = true;

	// should i block here?
}

void main_loop(void) {
	uint64_t next_byte = 0;
	uint16_t sequence_id = 0;
	while (!main_should_quit) {
		std::this_thread::sleep_for(std::chrono::milliseconds(250));

		// check connection
		//mumbleAPI.log(ownID, "check connection");

		mumble_connection_t con;
		if (mumbleAPI.getActiveServerConnection(ownID, &con) != MUMBLE_STATUS_OK) {
			// not connected
			continue;
		}

		bool con_connected = false;
		if (mumbleAPI.isConnectionSynchronized(ownID, con, &con_connected) != MUMBLE_STATUS_OK) {
			// ?
			continue;
		}
		if (!con_connected) {
			continue;
		}

		// get users in channel
		//mumbleAPI.log(ownID, "get users in channel");

		mumble_userid_t own_user_id;
		if (mumbleAPI.getLocalUserID(ownID, con, &own_user_id) != MUMBLE_STATUS_OK) {
			continue;
		}

		mumble_channelid_t channel;
		if (mumbleAPI.getChannelOfUser(ownID, con, own_user_id, &channel) != MUMBLE_STATUS_OK) {
			continue;
		}

		mumble_userid_t* channel_user_list = nullptr;
		size_t channel_user_list_size = 0;
		if (mumbleAPI.getUsersInChannel(ownID, con, channel, &channel_user_list, &channel_user_list_size) != MUMBLE_STATUS_OK) {
			continue;
		}

		// send to all users in channel
		mumbleAPI.log(ownID, "send data");
		std::vector<uint8_t> pkg_data;

		{ // sequence_id
			pkg_data.push_back((sequence_id >> 8*0) & 0xff);
			pkg_data.push_back((sequence_id >> 8*1) & 0xff);
		}

		{ // position in picture
			pkg_data.push_back((next_byte >> 8*0) & 0xff);
			pkg_data.push_back((next_byte >> 8*1) & 0xff);
			pkg_data.push_back((next_byte >> 8*2) & 0xff);
			pkg_data.push_back((next_byte >> 8*3) & 0xff);
			pkg_data.push_back((next_byte >> 8*4) & 0xff);
			pkg_data.push_back((next_byte >> 8*5) & 0xff);
			pkg_data.push_back((next_byte >> 8*6) & 0xff);
			pkg_data.push_back((next_byte >> 8*7) & 0xff);
		}

		//const size_t img_size = test_64x64.width * test_64x64.height * test_64x64.bytes_per_pixel;
		const size_t img_size = 64 * 64 * 3;

		size_t data_size = std::min<size_t>(900, img_size - next_byte);
		pkg_data.insert(
			pkg_data.end(),
			test_64x64.pixel_data + next_byte,
			test_64x64.pixel_data + next_byte + data_size
		);

		if (mumbleAPI.sendData(ownID, con, channel_user_list, channel_user_list_size, pkg_data.data(), pkg_data.size(), "video-001") != MUMBLE_STATUS_OK) {
			mumbleAPI.log(ownID, "failed to send data");
			continue;
		}

		next_byte += data_size;
		if (next_byte >= img_size) {
			next_byte = 0;
			sequence_id++;
		}
	}
}

MumbleStringWrapper mumble_getName() {
	static const char name[] = "video-dummy_send_img";
	return {
		name,
		std::strlen(name),
		false
	};
}

mumble_version_t mumble_getAPIVersion() {
	return MUMBLE_PLUGIN_API_VERSION;
}

void mumble_registerAPIFunctions(void* apiStruct) {
	mumbleAPI = MUMBLE_API_CAST(apiStruct);
}

void mumble_releaseResource(const void *) {}

mumble_version_t mumble_getVersion() {
	mumble_version_t version;
	version.major = 0;
	version.minor = 1;
	version.patch = 0;

	return version;
}

struct MumbleStringWrapper mumble_getAuthor() {
	static const char* author = "Erik Scholz & David Zero";
	return {
		author,
		std::strlen(author),
		false
	};
}

struct MumbleStringWrapper mumble_getDescription() {
	static const char* description = "video";
	return {
		description,
		std::strlen(description),
		false
	};
}

