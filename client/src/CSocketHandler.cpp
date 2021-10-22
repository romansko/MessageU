/**
 * MessageU Client
 * @file CSocketHandler.cpp
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 * https://github.com/Romansko/MessageU/blob/main/client/src/CSocketHandler.cpp
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

bool CSocketHandler::setSocketInfo(const std::string& address, const std::string& port)
{
	if (!isValidAddress(address) || !isValidPort(port))
	{
		return false;
	}
	_address = address;
	_port    = port;

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

/**
 * Clear socket and connect to new socket.
 */
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
	catch (...) {} // Do Nothing
	clear();
}


/**
 * Receive size bytes from _socket to buffer.
 * Return false if unable to receive expected size bytes.
 */
bool CSocketHandler::receive(uint8_t* const buffer, const size_t size) const
{
	if (_socket == nullptr || buffer == nullptr || size == 0)
		return false;
	
	size_t bytesLeft = size;
	uint8_t* ptr     = buffer;
	while (bytesLeft > 0)
	{
		uint8_t tempBuffer[PACKET_SIZE] = { 0 };

		boost::system::error_code errorCode; // read() will not throw exception when error_code is passed as argument.
		
		size_t bytesRead = read(*_socket, boost::asio::buffer(tempBuffer, PACKET_SIZE), errorCode);
		if (bytesRead == 0)
			return false;     // Error. Failed receiving and shouldn't use buffer.

		if (_bigEndian)
		{
			convertEndianness(tempBuffer, bytesRead);   // todo: handle correctly
		}
		
		const size_t bytesToCopy = (bytesLeft > bytesRead) ? bytesRead : bytesLeft;  // prevent buffer overflow.
		memcpy(ptr, tempBuffer, bytesToCopy);
		ptr       += bytesToCopy;
		bytesLeft = (bytesLeft < bytesToCopy) ? 0 : (bytesLeft - bytesToCopy);  // unsigned protection.
	}
	
	return true;
}

/**
 * Send size bytes from buffer to _socket.
 * Return false if unable to send expected size bytes.
 */
bool CSocketHandler::send(const uint8_t* const buffer, const size_t size) const
{
	if (_socket == nullptr || buffer == nullptr || size == 0)
		return false;
	
	size_t bytesLeft   = size;
	const uint8_t* ptr = buffer;
	while (bytesLeft > 0)
	{
		boost::system::error_code errorCode; // write() will not throw exception when error_code is passed as argument.
		uint8_t tempBuffer[PACKET_SIZE] = { 0 };
		const size_t bytesToSend = (bytesLeft > PACKET_SIZE) ? PACKET_SIZE : bytesLeft;
		
		memcpy(tempBuffer, ptr, bytesToSend);

		if (_bigEndian)
		{
			convertEndianness(tempBuffer, bytesToSend);  // todo: handle correctly
		}

		const size_t bytesWritten = write(*_socket, boost::asio::buffer(tempBuffer, PACKET_SIZE), errorCode);
		if (bytesWritten == 0)
			return false;
	
		ptr += bytesWritten;
		bytesLeft = (bytesLeft < bytesWritten) ? 0 : (bytesLeft - bytesWritten);  // unsigned protection.
	}
	return true;
}

/**
 * Wrap connect, send, receive and close functions.
 * Inner function have validations. Hence, this function does not validate arguments.
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

/**
 * Handle Endianness
 */
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



