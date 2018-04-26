#include "Window.hpp"
#include <iostream>

const uint32_t SCREEN_WIDTH = 800;
const uint32_t SCREEN_HEIGHT = 600;

void Window::cleanup()
{
	screenSurface.reset(nullptr);
	window.reset(nullptr);
}

Window::Window() :
	window(nullptr, SDL_DestroyWindow),
	screenSurface(nullptr, SDL_FreeSurface)
{
}

bool Window::init()
{
	this->cleanup();

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}
	window.reset(SDL_CreateWindow("GameBoy TMP",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN
	));
	if (window == nullptr)
	{
		std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}

	//Get window surface
	screenSurface.reset(
		SDL_GetWindowSurface(window.get())
	);

	//Fill the surface white
	SDL_FillRect(screenSurface.get(), NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

	//Update the surface
	SDL_UpdateWindowSurface(window.get());
	return true;
}

void Window::update(State & state)
{
	// TODO: Handle inputs and set them in the state
}

void Window::draw_frame(const State & state)
{
	// TODO: Load video information from the state and render it to a screen
}

Window::~Window()
{
	this->cleanup();
}
