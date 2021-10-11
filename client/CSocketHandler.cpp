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
	union   // Test for endianness
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

std::ostream& operator<<(std::ostream& os, const CSocketHandler& socket)
{
	os << socket._address << ':' << socket._port;
	return os;
}

bool CSocketHandler::setSocketInfo(const std::string& address, const std::string& port)
{
	if (!isValidAddress(address) || !isValidPort(port))
	{
		return false;
	}
	_address = address;
	_port = port;

	return true;
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

bool CSocketHandler::isValidAddress(const std::string& address)
{
	const auto cip = address.c_str();
	if ((strcmp(cip, LOCALHOST) == 0) || (strcmp(cip, CLOCALHOST) == 0))  // special case
		return true;
	try
	{
		(void) boost::asio::ip::address_v4::from_string(address);
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool CSocketHandler::isValidPort(const std::string& port)
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

bool CSocketHandler::connect()
{
	if (!isValidAddress(_address) || !isValidPort(_port))
		return false;
	try
	{
		clear();  // clear before new allocations
		_ioContext = new io_context;
		_resolver  = new tcp::resolver(*_ioContext);
		_socket    = new tcp::socket(*_ioContext);
		boost::asio::connect(*_socket, _resolver->resolve(_address, _port, tcp::resolver::query::canonical_name));
		_socket->non_blocking(false);  // blocking socket..
		return true;
	}
	catch(...)
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
	catch(...)
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
			convertEndianness(buffer, PACKET_SIZE);   // todo: handle correctly
		}
		
		return true;
	}
	catch(...)
	{
		return false;
	}
}

bool CSocketHandler::receive(uint8_t* const buffer, const size_t size) const
{
	uint8_t receivedBuffer[PACKET_SIZE] = { 0 };
	uint8_t* ptr = buffer;
	size_t bytesLeft = size;
	while (bytesLeft != 0)
	{
		if (!receive(receivedBuffer))
			return false;
		const size_t bytesToCopy = (bytesLeft > PACKET_SIZE) ? PACKET_SIZE : bytesLeft;
		memcpy(ptr, receivedBuffer, bytesToCopy);
		ptr += PACKET_SIZE;
		bytesLeft -= bytesToCopy;
	}
	return (size != 0);  // if reached here return true unless size = 0.
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
			convertEndianness(buffer, PACKET_SIZE);  // todo: handle correctly
		}
		(void) write(*_socket, boost::asio::buffer(buffer, PACKET_SIZE));
		return true;
	}
	catch(...)
	{
		return false;
	}
}

bool CSocketHandler::send(const uint8_t* const buffer, const size_t size) const
{
	uint8_t buffToSend[PACKET_SIZE] = { 0 };
	const uint8_t* ptr = buffer;
	size_t bytesLeft = size;
	while (bytesLeft != 0)
	{
		const size_t bytesToSend = (bytesLeft > PACKET_SIZE) ? PACKET_SIZE : bytesLeft;
		memcpy(buffToSend, ptr, bytesToSend);
		if (!send(buffToSend))
			return false;
		ptr += bytesToSend;
		bytesLeft -= bytesToSend;
	}
	return (size != 0);  // if reached here return true unless size = 0.
}

/**
 * Wrap connect, send, receive and close functions.
 */
bool CSocketHandler::sendReceive(const uint8_t* const toSend, const size_t size, uint8_t* const response, const size_t resSize)
{
	if (!connect())
	{
		return false;
	}
	if (!send(toSend, size))
	{
		close();
		return false;
	}
	if (!receive(response, resSize))
	{
		close();
		return false;
	}
	close();
	return true;
}

void CSocketHandler::convertEndianness(uint8_t* const buffer, const size_t size) const
{
	if (size % sizeof(u_long_type) != 0)
		return;  // invalid size.

	for (size_t i = 0; i < size; ++i)
	{
		const auto val = reinterpret_cast<u_long_type*>(&buffer[i * sizeof(u_long_type)]);
		*val = host_to_network_long(*val);
	}
}



