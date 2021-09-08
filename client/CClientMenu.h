/**
 * MessageU Client
 * @file CClientMenu.h
 * @brief Interface class for user input. Handle user's requests.
 * @author Roman Koifman
 */

#pragma once
#include <string>

class CClientMenu
{
private:
	const int INVALID_CHOICE = -1;	
	enum EOptions
	{
		MENU_EXIT             = 0,
		MENU_REGISTER         = 10,
		MENU_REQ_CLIENT_LIST  = 20,
		MENU_REQ_PUBLIC_KEY   = 30,
		MENU_REQ_PENDING_MSG  = 40,
		MENU_SEND_MSG         = 50,
		MENU_REQ_SYM_KEY      = 51,
		MENU_SEND_SYM_KEY     = 52
#ifdef BONUS
	   ,MENU_SEND_FILE        = 53
#endif
	};
	
	const std::string _welcomeString       = "MessageU client at your service.";
	const std::string _serverErrorString   = "Server responded with an error";
	const std::string _invalidChoiceString = "Invalid selection. Please try again..";

public:
	void invokeMenu();

private:
	int getUserSelection() const;
};

