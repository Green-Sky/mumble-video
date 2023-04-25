#pragma once

#include <SDL.h>

#include <optional>
#include <deque>
#include <mutex>

// simple threadsafe Queue (to be replaced)
struct FrameQueue {
	void push(const SDL_Surface& v) {
		std::lock_guard lg {_m};
		_queue.push_back(v); // forward?
	}

	void push(SDL_Surface&& v) {
		std::lock_guard lg {_m};
		_queue.push_back(v); // forward?
	}

	std::optional<SDL_Surface> pop() {
		std::lock_guard lg {_m};
		if (_queue.empty()) {
			return std::nullopt;
		}

		auto v = std::move(_queue.front());
		_queue.pop_front();
		return v;
	}

	std::deque<SDL_Surface> _queue;

	std::mutex _m;
};
