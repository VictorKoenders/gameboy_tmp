#include "State.hpp"
#include "Operations.hpp"
#include "OperationArguments.hpp"
#include <iostream>


int main()
{
	State state;

	while (true) {
		state.process();
	}
	return 0;
}
