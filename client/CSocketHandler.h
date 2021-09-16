/**
 * MessageU Client
 * @file CSocketHandler.h
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 */

#pragma once
#include <boost/asio/ip/tcp.hpp>

constexpr size_t PACKET_SIZE = 1024;
constexpr INT VALID_ADDRESS  = 1;
constexpr auto LOCALHOST     = "localhost";

class CSocketHandler
{
public:
	CSocketHandler();
	bool isValidIp(std::string& ip);
	bool isValidPort(std::string& port);
	bool receive(boost::asio::ip::tcp::socket& sock, uint8_t (&buffer)[PACKET_SIZE]);
	bool send(boost::asio::ip::tcp::socket& sock, const uint8_t(&buffer)[PACKET_SIZE]);

	
	// todo remove
	static std::string testSocket(std::string& address, std::string& port, const char* msg);

private:
	const bool _bigEndian;
	bool isBigEndian() const;
	void convertEndian(const uint8_t* const src, uint8_t* const dst, const size_t size) const;
	
};
