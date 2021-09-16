/**
 * MessageU Client
 * @file CSocketHandler.cpp
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 */

#include "CSocketHandler.h"

#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

using boost::asio::detail::u_long_type;
using boost::asio::detail::socket_ops::host_to_network_long;

CSocketHandler::CSocketHandler()
{
	union   // Test for endianess
	{
		uint32_t i;
		uint8_t c[sizeof(uint32_t)];
	}tester{ 1 };
	_bigEndian = (tester.c[0] == 0);
}

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
bool CSocketHandler::receive(tcp::socket& sock, uint8_t(&buffer)[PACKET_SIZE])
{
	try
	{
		memset(buffer, 0, PACKET_SIZE);  // reset array before copying.
		sock.non_blocking(false);             // make sure socket is blocking.
		(void) read(sock, boost::asio::buffer(buffer, PACKET_SIZE));
		if (_bigEndian)
		{
			convertEndian(buffer, PACKET_SIZE);  // convert to big endian
		}
		
		return true;
	}
	catch(boost::system::system_error&)
	{
		return false;
	}
}


// Send (blocking) PACKET_SIZE bytes to socket. Data sent copied from buffer.
bool CSocketHandler::send(tcp::socket& sock, const uint8_t(&buffer)[PACKET_SIZE])
{
	uint8_t buffToSend[PACKET_SIZE];
	memcpy(buffToSend, buffer, PACKET_SIZE);  // If conversion is needed, work on the copy.
	try
	{
		sock.non_blocking(false);  // blocking socket..
		if (_bigEndian)
		{
			convertEndian(buffToSend, PACKET_SIZE);  // convert to little endian.
		}
		(void)write(sock, boost::asio::buffer(buffToSend, PACKET_SIZE));
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
		tcp::socket s(ioContext);
		tcp::resolver resolver(ioContext);
		boost::asio::connect(s, resolver.resolve(address, port, tcp::resolver::query::canonical_name));
		char req[PACKET_SIZE];
		strcpy_s(req, msg);
		write(s, boost::asio::buffer(req, PACKET_SIZE));
		char reply[PACKET_SIZE];
		read(s, boost::asio::buffer(reply, PACKET_SIZE));
		ss << "Server Response: " << reply;
		s.close();
	}
	catch (...)
	{
		ss << "EXCEPTION: Server up ?";
	}
	return ss.str();
}


void CSocketHandler::convertEndian(uint8_t* const buffer, const size_t size) const
{
	if (size % sizeof(u_long_type) != 0)
		return;  // invalid size.

	for (size_t i = 0; i < size; ++i)
	{
		const auto val = reinterpret_cast<u_long_type*>(&buffer[i * sizeof(u_long_type)]);
		*val = host_to_network_long(*val);
	}
}