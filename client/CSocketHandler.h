/**
 * MessageU Client
 * @file CSocketHandler.h
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 */

#pragma once
#include <boost/asio/ip/tcp.hpp>

constexpr size_t PACKET_SIZE = 1024;

class CSocketHandler
{
public:
	bool receive(boost::asio::ip::tcp::socket& sock, uint8_t (&buffer)[PACKET_SIZE]);
	bool send(boost::asio::ip::tcp::socket& sock, const uint8_t(&buffer)[PACKET_SIZE]);
};

