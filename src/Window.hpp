#pragma once
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "State.hpp"
#include <memory>

class Window
{
	std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window;
	std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> screenSurface;
	void cleanup();
public:
	Window();
	bool init();
	void update(State& state);
	void draw_frame(const State& state);
	~Window();
};
