/**
 * MessageU Client
 * @file CSocketHandler.h
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 */
#pragma once
#include <string>
#include <cstdint>
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

	// validations
	bool isValidIp(std::string& ip) const;
	bool isValidPort(std::string& port) const;

	// logic
	bool connect(std::string& address, std::string& port);
	void close();
	bool receive(uint8_t(&buffer)[PACKET_SIZE]) const;
	bool send(uint8_t(&buffer)[PACKET_SIZE]) const;
	bool send(const uint8_t* const buffer, const size_t size) const;
	
	// todo remove
	std::string testSocket(const char* msg) const;

private:
	boost::asio::io_context*        _ioContext;
	boost::asio::ip::tcp::resolver* _resolver;
	boost::asio::ip::tcp::socket*   _socket;
	bool _bigEndian;

	void clear();
	void convertEndian(uint8_t* const buffer, const size_t size) const;

	
};
