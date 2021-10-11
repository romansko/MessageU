/**
 * MessageU Client
 * @file CClientMenu.cpp
 * @brief Interface class for user input. Handle user's requests.
 * @author Roman Koifman
 */


#include "CClientMenu.h"
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

CClientMenu::CClientMenu() : _registered(false)
{
}


void CClientMenu::clientStop(const std::string& error)
{
	std::cout << "Fatal Error: " << error << std::endl << "Client will stop." << std::endl;
	exit(1);
}


void CClientMenu::initialize()
{
	if (!_clientLogic.parseServeInfo())
	{
		clientStop(_clientLogic.getLastError());
	}
	_registered = _clientLogic.parseClientInfo();

}

/**
 * Print main menu to the screen.
 */
void CClientMenu::display() const
{
	clearMenu();
	if (_registered && !_clientLogic.getSelfUsername().empty())
		std::cout << "Hello " << _clientLogic.getSelfUsername() << ", ";
	std::cout << _welcomeString << std::endl << std::endl
		<< MENU_REGISTER        << ") Register" << std::endl
		<< MENU_REQ_CLIENT_LIST << ") Request for client list" << std::endl
		<< MENU_REQ_PUBLIC_KEY  << ") Request for public key" << std::endl
		<< MENU_REQ_PENDING_MSG << ") Request for waiting messages" << std::endl
		<< MENU_SEND_MSG        << ") Send a text message" << std::endl
		<< MENU_REQ_SYM_KEY     << ") Send a request for symmetric key" << std::endl
		<< MENU_SEND_SYM_KEY    << ") Send your symmetric key" << std::endl
		<< MENU_SEND_FILE       << ") Send a file" << std::endl
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
	const auto input = readUserInput();
	
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
	case MENU_SEND_FILE:
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
 * Read input from console.
 */
std::string CClientMenu::readUserInput(const std::string& description) const
{
	std::string input;
	if (!description.empty())
		std::cout << description << std::endl;
	do
	{
		std::getline(std::cin, input);
		boost::algorithm::trim(input);
	} while (input.empty());

	return input;
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
		std::cout << "MessageU Client Registration" << std::endl;
		if (_registered)
		{
			std::cout << "You have already registered!" << std::endl;
			return;
		}
		const auto username = readUserInput("Please type your username..");
		success = _clientLogic.registerClient(username);
		if (success)
		{
			std::cout << "Successfully registered on server." << std::endl;
		}
		_registered = success;
		break;
	}
	case MENU_REQ_CLIENT_LIST:
	{
		std::cout << "Request for client list" << std::endl;
		if (!_registered)
		{
			std::cout << "You must register before requesting clients list!" << std::endl;
			return;
		}
		success = _clientLogic.requestClientsList();
		if (success)
		{
			// Copy usernames into vector & sort them alphabetically.
			std::vector<std::string> usernames = _clientLogic.getUsernames();
			if (usernames.empty())
			{
				std::cout << "Server has no users registered." << std::endl;
				return;
			}
			
			std::cout << "Registered users:" << std::endl;
			for (const auto& username : usernames )
			{
				std::cout << username << std::endl;
			}
		}
		break;
	}
	case MENU_REQ_PUBLIC_KEY:
	{
		std::cout << "Request for public key" << std::endl;
		if (!_registered)
		{
			std::cout << "You must register before requesting clients list!" << std::endl;
			return;
		}
		const auto username = readUserInput("Please type a username..");
		std::string hexifiedKey;
		success = _clientLogic.requestClientPublicKey(username, hexifiedKey);
		if (success)
		{
			if (username == _clientLogic.getSelfUsername())
			{
				std::cout << _clientLogic.getSelfUsername() << ", your key is: " << std::endl;
			}
			else
			{
				std::cout << username << "'s public key is " << std::endl;
			}
			std::cout << hexifiedKey << std::endl;
		}
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
	case MENU_SEND_FILE:
	{
		std::cout << "File Send" << std::endl;
		break;
	}
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
