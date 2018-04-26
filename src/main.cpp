#include "State.hpp"
#include "Operations.hpp"
#include "OperationArguments.hpp"
#include "Window.hpp"

int main(int argc, char* args[])
{
	SDL_SetMainReady();

	Window window;
	State state;
	if (!window.init()) {
		return -1;
	}
	if (!state.load_rom("Pokemon Red.gb")) {
		return -2;
	}

	while (!state.is_error) {
		state.process();
	}
	getchar();
	return 0;
}
