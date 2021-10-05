/**
 * MessageU Client
 * @file CSocketHandler.cpp
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 */

#include "CSocketHandler.h"

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_context;
using boost::asio::detail::u_long_type;
using boost::asio::detail::socket_ops::host_to_network_long;

CSocketHandler::CSocketHandler() : _ioContext(nullptr), _resolver(nullptr), _socket(nullptr)
{
	union   // Test for endianess
	{
		uint32_t i;
		uint8_t c[sizeof(uint32_t)];
	}tester{ 1 };
	_bigEndian = (tester.c[0] == 0);
}

CSocketHandler::~CSocketHandler()
{
	close();
}

void CSocketHandler::clear()
{
	delete _ioContext;
	delete _resolver;
	delete _socket;
	_ioContext = nullptr;
	_resolver  = nullptr;
	_socket    = nullptr;
}

bool CSocketHandler::isValidIp(std::string& ip) const
{
	auto cip = ip.c_str();
	if ((strcmp(cip, LOCALHOST) == 0) || (strcmp(cip, CLOCALHOST) == 0))  // special case
		return true;
	try
	{
		(void) boost::asio::ip::address_v4::from_string(ip);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool CSocketHandler::isValidPort(std::string& port) const
{
	try
	{
		const int p = std::stoi(port);
		return (p != 0);  // port 0 is invalid..
	}
	catch (...)
	{
		return false;
	}
}

bool CSocketHandler::connect(std::string& address, std::string& port)
{
	if (!isValidIp(address) || !isValidPort(port))
		return false;
	try
	{
		clear();  // clear before new allocations
		_ioContext = new io_context;
		_resolver  = new tcp::resolver(*_ioContext);
		_socket    = new tcp::socket(*_ioContext);
		boost::asio::connect(*_socket, _resolver->resolve(address, port, tcp::resolver::query::canonical_name));
		_socket->non_blocking(false);  // blocking socket..
		return true;
	}
	catch (...)
	{
		return false;
	}
}

void CSocketHandler::close()
{
	try
	{
		if (_socket != nullptr)
			_socket->close();
	}
	catch (...)
	{
		/* Do Nothing */
	}
	clear();
}


// Receive (blocking) PACKET_SIZE bytes from socket and copy to buffer.
bool CSocketHandler::receive(uint8_t(&buffer)[PACKET_SIZE]) const
{
	memset(buffer, 0, PACKET_SIZE);
	if (_socket == nullptr)
		return false;
	try
	{
		(void) read(*_socket, boost::asio::buffer(buffer, PACKET_SIZE));
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
bool CSocketHandler::send(uint8_t(&buffer)[PACKET_SIZE]) const
{
	if (_socket == nullptr)
		return false;
	try
	{
		if (_bigEndian)
		{
			convertEndian(buffer, PACKET_SIZE);  // convert to little endian.
		}
		(void) write(*_socket, boost::asio::buffer(buffer, PACKET_SIZE));
		return true;
	}
	catch (boost::system::system_error& e)
	{
		std::cout << e.what() << std::endl;
		return false;
	}
}

bool CSocketHandler::send(const uint8_t* const buffer, const size_t size) const
{
	uint8_t buffToSend[PACKET_SIZE];
	const uint8_t* ptr = buffer;
	size_t bytesLeft = size;
	while (bytesLeft != 0)
	{
		const size_t bytesToSend = (bytesLeft > PACKET_SIZE) ? PACKET_SIZE : bytesLeft;
		if (bytesToSend < PACKET_SIZE)
			memset(buffToSend, 0, PACKET_SIZE);

		memcpy(buffToSend, ptr, bytesToSend);
		if (!send(buffToSend))
			return false;
		bytesLeft -= bytesToSend;
	}
	
	return (size != 0);  // if reached here return true unless size = 0.
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


/************ TODO REMOVE ***************/
std::string CSocketHandler::testSocket(const char* msg) const

{
	std::stringstream ss;
	try
	{
		char req[PACKET_SIZE];
		strcpy_s(req, msg);
		write(*_socket, boost::asio::buffer(req, PACKET_SIZE));
		char reply[PACKET_SIZE];
		read(*_socket, boost::asio::buffer(reply, PACKET_SIZE));
		ss << "Server Response: " << reply;
	}
	catch (...)
	{
		ss << "EXCEPTION: Server up ?";
	}
	return ss.str();
}