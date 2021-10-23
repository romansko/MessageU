/**
 * MessageU Client
 * @file main.cpp
 * @brief Client program entry point.
 * Compiled with: Visual Studio 2019. C++14.
 * Multi-threaded Debug (/MTd).
 * Boost Library 1.77.0 (static linkage)
 * Crypto++ Library 8.5 (static linkage).
 * For more info, please refer to https://github.com/Romansko/MessageU#readme
 * @author Roman Koifman
 * https://github.com/Romansko/MessageU/blob/main/client/src/main.cpp
 */
#include "CClientMenu.h"

int main(int argc, char* argv[])
{
	CClientMenu menu;
	menu.initialize();
	
	for (;;)
	{
		menu.display();
		menu.handleUserChoice();
		menu.pause();
	}
	
	return 0;
}

