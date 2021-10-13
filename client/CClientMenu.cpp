/**
 * MessageU Client
 * @file CClientMenu.cpp
 * @brief Interface class for user input. Handle user's requests.
 * @author Roman Koifman
 */


#include "CClientMenu.h"
#include <iostream>
#include <string>
#include <boost/algorithm/string/trim.hpp>


CClientMenu::CClientMenu() : _registered(false)
{
}

/**
 * Print error and exit client.
 */
void CClientMenu::clientStop(const std::string& error)
{
	std::cout << "Fatal Error: " << error << std::endl << "Client will stop." << std::endl;
	exit(1);
}

/**
 * Initialize client's menu & its internals.
 */
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
 * Read input from console.
 */
std::string CClientMenu::readUserInput(const std::string& description) const
{
	std::string input;
	std::cout << description << std::endl;
	do
	{
		std::getline(std::cin, input);
		boost::algorithm::trim(input);
	} while (input.empty());

	return input;
}

/**
 * Read & Validate user's input according to main menu options.
 */
int CClientMenu::readValidateUserChoice() const
{
	const auto input = readUserInput();
	const auto it = std::find_if(_menuOptions.begin(), _menuOptions.end(),
		[&input](const EOptions& opt) {
			return (input == std::to_string(opt) );
		});
	return (it == _menuOptions.end()) ? INVALID_CHOICE : *it;
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
		std::cout << "UNIMPLEMENTED" << std::endl;
		break;
	}
	case MENU_SEND_MSG:
	{
		std::cout << "Send a text message" << std::endl;
		std::cout << "UNIMPLEMENTED" << std::endl;
		break;
	}
	case MENU_REQ_SYM_KEY:
	{
		std::cout << "Send a request for symmetric key" << std::endl;
		const auto username = readUserInput("Please type a username..");
		success = _clientLogic.requestSymmetricKey(username);
		if (success)
		{
			std::cout << "success" << std::endl;
		}
		break;
	}
	case MENU_SEND_SYM_KEY:
	{
		std::cout << "Send your symmetric key" << std::endl;
		std::cout << "UNIMPLEMENTED" << std::endl;
		break;
	}
	case MENU_SEND_FILE:
	{
		std::cout << "File Send" << std::endl;
		std::cout << "UNIMPLEMENTED" << std::endl;
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
