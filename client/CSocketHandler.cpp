/**
 * MessageU Client
 * @file CSocketHandler.cpp
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 */

#include "CSocketHandler.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>


bool CSocketHandler::isValidIp(std::string& ip)
{
	in_addr dst;  // dummy
	const auto cip = ip.c_str();
	if (strcmp(cip, LOCALHOST) == 0)  // special case
		return true;
	return (VALID_ADDRESS == inet_pton(AF_INET, cip, &dst));
}

bool CSocketHandler::isValidPort(std::string& port)
{
	try
	{
		const int p = std::stoi(port);
		return (p != 0);  // port 0 is invalid..
	}
	catch(...)
	{
		return false;
	}
}

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


std::string CSocketHandler::testSocket(std::string& address, std::string& port, const char* msg)  // todo remove
{
	std::stringstream ss;
	try
	{
		boost::asio::io_context ioContext;
		boost::asio::ip::tcp::socket s(ioContext);
		boost::asio::ip::tcp::resolver resolver(ioContext);
		boost::asio::connect(s, resolver.resolve(address, port, boost::asio::ip::tcp::resolver::query::canonical_name));
		char req[PACKET_SIZE];
		strcpy_s(req, msg);
		boost::asio::write(s, boost::asio::buffer(req, PACKET_SIZE));
		char reply[PACKET_SIZE];
		boost::asio::read(s, boost::asio::buffer(reply, PACKET_SIZE));
		ss << "Server Response: " << reply;
		s.close();
	}
	catch (...)
	{
		ss << "EXCEPTION: Server up ?";
	}
	return ss.str();
}

bool CSocketHandler::isBigEndian()
{
	union
	{
		uint32_t i;
		uint8_t c[sizeof(uint32_t)];
	}tester{1};
	return (tester.c[0] == 0);
}
