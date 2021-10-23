/**
 * MessageU Client
 * @file CSocketHandler.cpp
 * @brief Handle sending and receiving from a socket.
 * @author Roman Koifman
 * https://github.com/Romansko/MessageU/blob/main/client/src/CSocketHandler.cpp
 */

#include "CSocketHandler.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::io_context;

CSocketHandler::CSocketHandler() : _ioContext(nullptr), _resolver(nullptr), _socket(nullptr), _connected(false)
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


/**
 * Try parse IP Address. Return false if failed.
 * Handle special cases of "localhost", "LOCALHOST"
 */
bool CSocketHandler::isValidAddress(const std::string& address)
{
	if ((address == "localhost") || (address == "LOCALHOST"))
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

/**
 * Try to parse a port number from a string.
 * Return false if failed.
 */
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
		close();  // close & clear current socket before new allocations.
		_ioContext = new io_context;
		_resolver  = new tcp::resolver(*_ioContext);
		_socket    = new tcp::socket(*_ioContext);
		boost::asio::connect(*_socket, _resolver->resolve(_address, _port, tcp::resolver::query::canonical_name));
		_socket->non_blocking(false);  // blocking socket..
		_connected = true;
	}
	catch(...)
	{
		_connected = false;
	}
	return _connected;
}

/**
 * Close & clear current socket.
 */
void CSocketHandler::close()
{
	try
	{
		if (_socket != nullptr)
			_socket->close();
	}
	catch (...) {} // Do Nothing
	delete _ioContext;
	delete _resolver;
	delete _socket;
	_ioContext = nullptr;
	_resolver  = nullptr;
	_socket    = nullptr;
	_connected = false;
}


/**
 * Receive size bytes from _socket to buffer.
 * Return false if unable to receive expected size bytes.
 */
bool CSocketHandler::receive(uint8_t* const buffer, const size_t size) const
{
	if (_socket == nullptr || !_connected || buffer == nullptr || size == 0)
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

		if (_bigEndian)  // It's required to convert from little endian to big endian.
		{
			swapBytes(tempBuffer, bytesRead);   
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
	if (_socket == nullptr || !_connected || buffer == nullptr || size == 0)
		return false;
	
	size_t bytesLeft   = size;
	const uint8_t* ptr = buffer;
	while (bytesLeft > 0)
	{
		boost::system::error_code errorCode; // write() will not throw exception when error_code is passed as argument.
		uint8_t tempBuffer[PACKET_SIZE] = { 0 };
		const size_t bytesToSend = (bytesLeft > PACKET_SIZE) ? PACKET_SIZE : bytesLeft;
		
		memcpy(tempBuffer, ptr, bytesToSend);

		if (_bigEndian)  // It's required to convert from big endian to little endian.
		{
			swapBytes(tempBuffer, bytesToSend);
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
 * Handle Endianness.
 */
void CSocketHandler::swapBytes(uint8_t* const buffer, size_t size) const
{
	if (buffer == nullptr || size < sizeof(uint32_t))
		return; 

	size -= (size % sizeof(uint32_t));
	uint32_t* const ptr = reinterpret_cast<uint32_t* const>(buffer);
	for (size_t i = 0; i < size; ++i)
	{
		const uint32_t tmp = ((buffer[i] << 8) & 0xFF00FF00) | ((buffer[i] >> 8) & 0xFF00FF);
		buffer[i] = (tmp << 16) | (tmp >> 16);
	}
		
}



