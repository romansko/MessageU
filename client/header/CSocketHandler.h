/**
 * MessageU Client
 * @file CSocketHandler.h
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 * https://github.com/Romansko/MessageU/blob/main/client/header/CSocketHandler.h
 */
#pragma once
#include <string>
#include <cstdint>
#include <ostream>
#include <boost/asio/ip/tcp.hpp>

constexpr size_t PACKET_SIZE = 1024;
constexpr auto LOCALHOST     = "localhost";
constexpr auto CLOCALHOST    = "LOCALHOST";

class CSocketHandler
{
public:
	CSocketHandler();
	~CSocketHandler();

	// do not allow
	CSocketHandler(const CSocketHandler& other) = delete;
	CSocketHandler(CSocketHandler&& other) noexcept = delete;
	CSocketHandler& operator=(const CSocketHandler& other) = delete;
	CSocketHandler& operator=(CSocketHandler&& other) noexcept = delete;

	friend std::ostream& operator<<(std::ostream& os, const CSocketHandler* socket) {
		if (socket != nullptr)
			os << socket->_address << ':' << socket->_port;
		return os;
	}
	friend std::ostream& operator<<(std::ostream& os, const CSocketHandler& socket) {
		return operator<<(os, &socket);
	}

	// validations
	static bool isValidAddress(const std::string& address);
	static bool isValidPort(const std::string& port);

	// logic
	bool setSocketInfo(const std::string& address, const std::string& port);
	bool connect();
	void close();
	bool receive(uint8_t* const buffer, const size_t size) const;
	bool send(const uint8_t* const buffer, const size_t size) const;
	bool sendReceive(const uint8_t* const toSend, const size_t size, uint8_t* const response, const size_t resSize);


private:
	std::string                     _address;
	std::string                     _port;
	boost::asio::io_context*        _ioContext;
	boost::asio::ip::tcp::resolver* _resolver;
	boost::asio::ip::tcp::socket*   _socket;
	bool _bigEndian;

	void clear();
	void swapBytes(uint8_t* const buffer, size_t size) const;

	
};
