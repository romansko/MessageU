/**
 * MessageU Client
 * @file CClientMenu.cpp
 * @brief Interface class for user input. Handle user's requests.
 * @author Roman Koifman
 */


#include "CClientMenu.h"
#include <iostream>
#include <boost/algorithm/string/trim.hpp>


CClientMenu::CClientMenu() : _registered(false) {}

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
 * Read & Validate user's input according to main menu options.
 * If valid option, assign menuOption.
 */
bool CClientMenu::getMenuOption(CMenuOption& menuOption) const
{
	const std::string input = readUserInput();
	const auto it = std::find_if(_menuOptions.begin(), _menuOptions.end(),
		[&input](auto& opt) { return (input == std::to_string(static_cast<uint32_t>(opt.getValue()))); });
	if (it == _menuOptions.end())
	{
		return false; // menuOption invalid.
	}
	menuOption = *it;
	return true;
}


/**
 * Invoke matching function to user's choice. User's choice is validated.
 */
void CClientMenu::handleUserChoice()
{
	CMenuOption menuOption;
	bool success = getMenuOption(menuOption);
	while (!success)
	{
		std::cout << "Invalid input. Please try again.." << std::endl;
		success = getMenuOption(menuOption);
	}

	clearMenu();
	std::cout << menuOption.getDescription() << std::endl;
	if (!_registered && menuOption.requireRegistration())
	{
		std::cout << "You must register first!" << std::endl;
		return;
	}
	switch (menuOption.getValue())
	{
	case CMenuOption::EOption::MENU_EXIT:
	{
		system("pause");
		exit(0);
	}
	case CMenuOption::EOption::MENU_REGISTER:
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
	case CMenuOption::EOption::MENU_REQ_CLIENT_LIST:
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
	case CMenuOption::EOption::MENU_REQ_PUBLIC_KEY:
	{
		const auto username = readUserInput("Please type a username..");
		if (username == _clientLogic.getSelfUsername())
		{
			std::cout << _clientLogic.getSelfUsername() << ", your key is stored in the system already. " << std::endl;
			return;
		}
		success = _clientLogic.requestClientPublicKey(username);
		if (success)
		{
			std::cout << username << "'s public key was retrieved successfully." << std::endl;
		}
		break;
	}
	case CMenuOption::EOption::MENU_REQ_PENDING_MSG:
	{
		std::vector<CClientLogic::SMessage> messages;
		success = _clientLogic.requestPendingMessages(messages);
		if (success)
		{
			for (const auto& msg : messages)
				std::cout << "From: " << msg.username << std::endl << "Content:" << std::endl << msg.content << std::endl;
		}
		break;
	}
	case CMenuOption::EOption::MENU_SEND_MSG:
	{
		const auto username = readUserInput("Please type a username..");
		const auto message = readUserInput("Enter message: ");
		success = _clientLogic.sendMessage(username, MSG_TEXT, message);
		if (success)
		{
			std::cout << "Message was sent successfully" << std::endl;
		}
		break;
	}
	case CMenuOption::EOption::MENU_REQ_SYM_KEY:
	{
		const auto username = readUserInput("Please type a username..");
		success = _clientLogic.sendMessage(username, MSG_SYMMETRIC_KEY_REQUEST);
		if (success)
		{
			std::cout << "Symmetric key request was sent successfully" << std::endl;
		}
		break;
	}
	case CMenuOption::EOption::MENU_SEND_SYM_KEY:
	{
		const auto username = readUserInput("Please type a username..");
		success = _clientLogic.sendMessage(username, MSG_SYMMETRIC_KEY_SEND);
		if (success)
		{
			std::cout << "Symmetric key was sent successfully" << std::endl;
		}
		break;
	}
	case CMenuOption::EOption::MENU_SEND_FILE:
	{
		const auto username = readUserInput("Please type a username..");
		const auto message = readUserInput("Enter filepath: ");
		success = _clientLogic.sendMessage(username, MSG_FILE, message);
		if (success)
		{
			std::cout << "File was sent successfully" << std::endl;
		}
		break;
	}
	}

	if (!success)
	{
		std::cout << _clientLogic.getLastError() << std::endl;
	}
}
