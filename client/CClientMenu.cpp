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
	std::cout << "MessageU client at your service." << std::endl << std::endl;
	for (const auto& opt : _menuOptions)
		std::cout << opt << std::endl;
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
 * Get menu option. valid EOption will succeed.
 */
CClientMenu::CMenuOption CClientMenu::getMenuOption(CMenuOption::EOption val) const
{
	const auto it = std::find_if(_menuOptions.begin(), _menuOptions.end(),
		[&val](auto& opt) { return val == opt.getValue(); });
	return *it;
}

/**
 * Read & Validate user's input according to main menu options.
 * Return as int because of INVALID_CHOICE usage.
 */
int CClientMenu::readValidateUserChoice() const
{
	const std::string input = readUserInput();
	
	const auto it = std::find_if(_menuOptions.begin(), _menuOptions.end(),
		[&input](auto& opt) { return (input == std::to_string(opt.getValue())); });
	return (it == _menuOptions.end()) ? CMenuOption::INVALID_CHOICE : it->getValue();
}


/**
 * Invoke matching function to user's choice. User's choice is validated.
 */
void CClientMenu::handleUserChoice()
{
	bool success = true;
	int userChoice = readValidateUserChoice();
	while (userChoice == CMenuOption::INVALID_CHOICE)
	{
		std::cout << "Invalid input. Please try again.." << std::endl;
		userChoice = readValidateUserChoice();
	}

	clearMenu();
	const auto menuOption = getMenuOption(static_cast<CMenuOption::EOption>(userChoice));
	std::cout << menuOption.getDescription() << std::endl;
	if (!_registered && menuOption.requireRegistration())
	{
		std::cout << "You must register first!" << std::endl;
		return;
	}
	switch (userChoice)
	{
	case CMenuOption::MENU_EXIT:
	{
		system("pause");
		exit(0);
	}
	case CMenuOption::MENU_REGISTER:
	{
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
	case CMenuOption::MENU_REQ_CLIENT_LIST:
	{
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
	case CMenuOption::MENU_REQ_PUBLIC_KEY:
	{
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
	case CMenuOption::MENU_REQ_PENDING_MSG:
	{
		std::cout << "UNIMPLEMENTED" << std::endl;
		break;
	}
	case CMenuOption::MENU_SEND_MSG:
	{
		std::cout << "UNIMPLEMENTED" << std::endl;
		break;
	}
	case CMenuOption::MENU_REQ_SYM_KEY:
	{
		const auto username = readUserInput("Please type a username..");
		success = _clientLogic.requestSymmetricKey(username);
		if (success)
		{
			std::cout << "success" << std::endl;
		}
		break;
	}
	case CMenuOption::MENU_SEND_SYM_KEY:
	{
		std::cout << "UNIMPLEMENTED" << std::endl;
		break;
	}
	case CMenuOption::MENU_SEND_FILE:
	{
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
