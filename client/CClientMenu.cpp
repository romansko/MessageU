/**
 * MessageU Client
 * @file CClientMenu.cpp
 * @brief Interface class for user input. Handle user's requests.
 * @author Roman Koifman
 */


#include "CClientMenu.h"
#include <iostream>
#include <cstdlib>


int CClientMenu::getUserSelection() const
{
	int opt;
	std::string input;
	std::cin >> input;

	// Clear cin stream. I.e. Only the 1st token will be parsed.
	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	
	try
	{
		opt = std::stoi(input);
	}
	catch (std::exception&)
	{
		std::cout << _invalidChoiceString << std::endl;
		return INVALID_CHOICE;
	}

	switch (opt)
	{
	case MENU_EXIT:
	case MENU_REGISTER:
	case MENU_REQ_CLIENT_LIST:
	case MENU_REQ_PUBLIC_KEY:
	case MENU_REQ_PENDING_MSG:
	case MENU_SEND_MSG:
	case MENU_REQ_SYM_KEY:
	case MENU_SEND_SYM_KEY:
#ifdef BONUS
	case MENU_SEND_FILE:
#endif
	{
		return opt;
	}
	default:
	{
		std::cout << _invalidChoiceString << std::endl;
		return INVALID_CHOICE;
	}
	}
}

void CClientMenu::invokeMenu()
{
	int userChoice;
	system("cls");
	std::cout << _welcomeString << std::endl << std::endl
		<< MENU_REGISTER        << ") Register" << std::endl
		<< MENU_REQ_CLIENT_LIST << ") Request for client list" << std::endl
		<< MENU_REQ_PUBLIC_KEY  << ") Request for public key" << std::endl
		<< MENU_REQ_PENDING_MSG << ") Request for waiting messages" << std::endl
		<< MENU_SEND_MSG        << ") Send a text message" << std::endl
		<< MENU_REQ_SYM_KEY     << ") Send a request for symmetric key" << std::endl
		<< MENU_SEND_SYM_KEY    << ") Send your symmetric key" << std::endl
#ifdef BONUS
		<< MENU_SEND_FILE       << ") Send a file" << std::endl
#endif
		<< " " << MENU_EXIT     << ") Exit client" << std::endl;

	do
	{
		userChoice = getUserSelection();
	} while (userChoice == INVALID_CHOICE);

	switch (userChoice)
	{
	case MENU_EXIT:
	{
		exit(0);
	}
	case MENU_REGISTER:
	{
		std::cout << "Register" << std::endl;
		break;
	}
	case MENU_REQ_CLIENT_LIST:
	{
		std::cout << "Request for client list" << std::endl;
		break;
	}
	case MENU_REQ_PUBLIC_KEY:
	{
		std::cout << "Request for public key" << std::endl;
		break;
	}
	case MENU_REQ_PENDING_MSG:
	{
		std::cout << "Request for waiting messages" << std::endl;
		break;
	}
	case MENU_SEND_MSG:
	{
		std::cout << "Send a text message" << std::endl;
		break;
	}
	case MENU_REQ_SYM_KEY:
	{
		std::cout << "Send a request for symmetric key" << std::endl;
		break;
	}
	case MENU_SEND_SYM_KEY:
	{
		std::cout << "Send your symmetric key" << std::endl;
		break;
	}
#ifdef BONUS
	case MENU_SEND_FILE:
	{
		std::cout << "Register" << std::endl;
		break;
	}
#endif
	default:  /* Can't happen. Was validated in getUserSelection. */
	{
		break;
	}
	}
	
	system("pause"); // todo remove
}



#define EVER (;;)
int main(int argc, char* argv[])
{
	CClientMenu menu;

	for EVER
	{
		menu.invokeMenu();
	}
	
	return 0;
}