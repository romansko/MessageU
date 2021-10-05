/**
 * MessageU Client
 * @file CClientMenu.cpp
 * @brief Interface class for user input. Handle user's requests.
 * @author Roman Koifman
 */


#include "CClientMenu.h"
#include <iostream>

/**
 * Print main menu to the screen.
 */
void CClientMenu::display() const
{
	clearMenu();
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
}

void CClientMenu::clearMenu() const
{
	system("cls");
}


/**
 * Read & Validate user's input according to main menu options.
 */
int CClientMenu::readValidateUserChoice() const
{
	int opt;
	const auto input = _clientLogic.readUserInput();
	
	// do not allow multiple tokens (separated by tab or space). E.g. "20 20 20" is invalid!
	if (input.find_first_of(" \t") != std::string::npos)
		return INVALID_CHOICE;

	if (input == "0")   // special case.
		return MENU_EXIT;
	
	try
	{
		opt = std::stoi(input);
	}
	catch (...)
	{
		return INVALID_CHOICE;
	}

	switch (opt)
	{
	//case MENU_EXIT:       // special case. compared as ascii above.
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
		return INVALID_CHOICE;
	}
	}
}


/**
 * Invoke matching function to user's choice. User's choice is validated.
 */
void CClientMenu::handleUserChoice()
{
	int userChoice = readValidateUserChoice();
	while (userChoice == INVALID_CHOICE)
	{
		std::cout << _invalidInput << std::endl;
		userChoice = readValidateUserChoice();
	}

	clearMenu();
	bool success = true;
	switch (userChoice)
	{
	case MENU_EXIT:
	{
		std::cout << "MessageU Client will now exit." << std::endl;
		system("pause");
		exit(0);
	}
	case MENU_REGISTER:
	{
		std::cout << "MessageU Client Register" << std::endl;
		success = _clientLogic.registerClient();
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
	default:  /* Can't happen. Was validated in readValidateUserChoice. */
	{
		break;
	}
	}

	if (!success)
	{
		std::cout << _clientLogic.getLastError() << std::endl;
	}
}


