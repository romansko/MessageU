
#include "CClientMenu.h"
#include <iostream>

int main(int argc, char* argv[])
{
	CClientMenu menu;
	menu.initialize();
	
	for (;;)
	{
		menu.display();
		menu.handleUserChoice();
		system("pause");
	}
	
	return 0;
}

