/**
 * MessageU Client
 * @file CClientMenu.h
 * @brief Interface class for user input. Handle user's requests.
 * can be replaced by GUI class and invoke CClientLogic correspondingly.
 * @author Roman Koifman
 * https://github.com/Romansko/MessageU/blob/main/client/header/CClientMenu.h
 */
#pragma once
#include "CClientLogic.h"
#include <string>       // std::to_string
#include <iomanip>      // std::setw

class CClientMenu
{
public:
	CClientMenu() : _registered(false) {}
	void initialize();
	void display() const;
	void handleUserChoice();

	void clear() const { system("cls"); }     // clear menu
	void pause() const { system("pause"); }   // pause menu

private:
	
	class CMenuOption
	{
	public:
		enum class EOption
		{
			MENU_REGISTER        = 10,
			MENU_REQ_CLIENT_LIST = 20,
			MENU_REQ_PUBLIC_KEY  = 30,
			MENU_REQ_PENDING_MSG = 40,
			MENU_SEND_MSG        = 50,
			MENU_REQ_SYM_KEY     = 51,
			MENU_SEND_SYM_KEY    = 52,
			MENU_SEND_FILE       = 53,
			MENU_EXIT            = 0
		};

	private:
		EOption     _value;
		bool        _registration;  // indicates whether registration is required before option usage.
		std::string _description;
		std::string _success;       // success description

	public:
		CMenuOption() : _value(EOption::MENU_EXIT), _registration(false) {}
		CMenuOption(const EOption val, const bool reg, std::string desc, std::string success) : _value(val),
			_registration(reg), _description(std::move(desc)), _success(std::move(success)) {}

		friend std::ostream& operator<<(std::ostream& os, const CMenuOption& opt) {
			os << std::setw(2) << static_cast<uint32_t>(opt._value) << ") " << opt._description;
			return os;
		}
		EOption getValue()             const { return _value; }
		bool requireRegistration()     const { return _registration; }
		std::string getDescription()   const { return _description; }
		std::string getSuccessString() const { return _success; }
	};

private:
	
	void clientStop(const std::string& error) const;
	std::string readUserInput(const std::string& description = "") const;
	bool getMenuOption(CMenuOption& menuOption) const;


	CClientLogic                   _clientLogic;
	bool                           _registered;
	const std::vector<CMenuOption> _menuOptions {
		{ CMenuOption::EOption::MENU_REGISTER,        false, "Register",                         "Successfully registered on server."},
		{ CMenuOption::EOption::MENU_REQ_CLIENT_LIST, true,  "Request for client list",          ""},
		{ CMenuOption::EOption::MENU_REQ_PUBLIC_KEY,  true,  "Request for public key",           "Public key was retrieved successfully."},
		{ CMenuOption::EOption::MENU_REQ_PENDING_MSG, true,  "Request for waiting messages",     ""},
		{ CMenuOption::EOption::MENU_SEND_MSG,        true,  "Send a text message",              "Message was sent successfully."},
		{ CMenuOption::EOption::MENU_REQ_SYM_KEY,     true,  "Send a request for symmetric key", "Symmetric key request was sent successfully."},
		{ CMenuOption::EOption::MENU_SEND_SYM_KEY,    true,  "Send your symmetric key",          "Symmetric key was sent successfully."},
		{ CMenuOption::EOption::MENU_SEND_FILE,       true,  "Send a file",                      "File was sent successfully."},
		{ CMenuOption::EOption::MENU_EXIT,            false, "Exit client",                      ""}
	};
};

