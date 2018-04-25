#include "State.hpp"
#include "Operations.hpp"
#include "OperationArguments.hpp"
#include <iostream>


int main()
{
	State state;

	while (!state.is_error) {
		state.process();
	}
	getchar();
	return 0;
}
