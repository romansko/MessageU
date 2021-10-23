/**
 * MessageU Client
 * @file CClientMenu.cpp
 * @brief Interface class for user input. Handle user's requests.
 * can be replaced by GUI class and invoke CClientLogic correspondingly.
 * @author Roman Koifman
 * https://github.com/Romansko/MessageU/blob/main/client/src/CClientMenu.cpp
 */
#include "CClientMenu.h"
#include <iostream>
#include <boost/algorithm/string/trim.hpp>

/**
 * Print error and exit client.
 */
void CClientMenu::clientStop(const std::string& error) const
{
	std::cout << "Fatal Error: " << error << std::endl << "Client will stop." << std::endl;
	pause();
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
	clear();
	if (_registered && !_clientLogic.getSelfUsername().empty())
		std::cout << "Hello " << _clientLogic.getSelfUsername() << ", ";
	std::cout << "MessageU client at your service." << std::endl << std::endl;
	for (const auto& opt : _menuOptions)
		std::cout << opt << std::endl;
}


/**
 * Read input from console.
 * Do not allow empty lines.
 */
std::string CClientMenu::readUserInput(const std::string& description) const
{
	std::string input;
	std::cout << description << std::endl;
	do
	{
		std::getline(std::cin, input);
		boost::algorithm::trim(input);
		if (std::cin.eof())   // ignore ctrl + z.
			std::cin.clear();
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

	clear();
	std::cout << std::endl;
	if (!_registered && menuOption.requireRegistration())
	{
		std::cout << "You must register first!" << std::endl;
		return;
	}

	// Main selection switch
	switch (menuOption.getValue())
	{
	case CMenuOption::EOption::MENU_EXIT:
	{
		std::cout << "Client will now exit." << std::endl;
		pause();
		exit(0);
	}
	case CMenuOption::EOption::MENU_REGISTER:
	{
		if (_registered)
		{
			std::cout << _clientLogic.getSelfUsername() << ", you have already registered!" << std::endl;
			return;
		}
		const std::string username = readUserInput("Please type your username..");
		success = _clientLogic.registerClient(username);
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
		const std::string username = readUserInput("Please type a username..");
		success = _clientLogic.requestClientPublicKey(username);
		break;
	}
	case CMenuOption::EOption::MENU_REQ_PENDING_MSG:
	{
		std::vector<CClientLogic::SMessage> messages;
		success = _clientLogic.requestPendingMessages(messages);
		if (success)
		{
			std::cout << std::endl;
			for (const auto& msg : messages)
			{
				std::cout << "From: " << msg.username << std::endl << "Content:" << std::endl << msg.content << std::endl << std::endl;
			}
			const std::string lastErr = _clientLogic.getLastError();  // contains a string of errors occurred during messages parsing.
			if (!lastErr.empty())
			{
				std::cout << std::endl << "MESSAGES ERROR LOG: " << std::endl << lastErr;
			}
		}
		break;
	}
	case CMenuOption::EOption::MENU_SEND_MSG:
	{
		const std::string username = readUserInput("Please type a username to send message to..");
		const std::string message  = readUserInput("Enter message: ");
		success = _clientLogic.sendMessage(username, MSG_TEXT, message);
		break;
	}
	case CMenuOption::EOption::MENU_REQ_SYM_KEY:
	{
		const std::string username = readUserInput("Please type a username to request symmetric key from..");
		success = _clientLogic.sendMessage(username, MSG_SYMMETRIC_KEY_REQUEST);
		break;
	}
	case CMenuOption::EOption::MENU_SEND_SYM_KEY:
	{
		const std::string username = readUserInput("Please type a username to send symmetric key to..");
		success = _clientLogic.sendMessage(username, MSG_SYMMETRIC_KEY_SEND);
		break;
	}
	case CMenuOption::EOption::MENU_SEND_FILE:
	{
		const std::string username = readUserInput("Please type a username to send file to..");
		const std::string message  = readUserInput("Enter filepath: ");
		success = _clientLogic.sendMessage(username, MSG_FILE, message);
		break;
	}
	}

	std::cout << (success ? menuOption.getSuccessString() : _clientLogic.getLastError()) << std::endl;
}
