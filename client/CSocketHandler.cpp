/**
 * MessageU Client
 * @file CSocketHandler.cpp
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 */

#include "CSocketHandler.h"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>


// Receive (blocking) PACKET_SIZE bytes from socket and copy to buffer.
bool CSocketHandler::receive(boost::asio::ip::tcp::socket& sock, uint8_t(&buffer)[PACKET_SIZE])
{
	try
	{
		memset(buffer, 0, PACKET_SIZE);  // reset array before copying.
		sock.non_blocking(false);             // make sure socket is blocking.
		(void) boost::asio::read(sock, boost::asio::buffer(buffer, PACKET_SIZE));
		return true;
	}
	catch(boost::system::system_error&)
	{
		return false;
	}
}


// Send (blocking) PACKET_SIZE bytes to socket. Data sent copied from buffer.
bool CSocketHandler::send(boost::asio::ip::tcp::socket& sock, const uint8_t(&buffer)[PACKET_SIZE])
{
	try
	{
		sock.non_blocking(false);  // make sure socket is blocking.
		(void) boost::asio::write(sock, boost::asio::buffer(buffer, PACKET_SIZE));
		return true;
	}
	catch (boost::system::system_error&)
	{
		return false;
	}
}