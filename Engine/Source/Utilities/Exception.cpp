#include "Exception.h"
#include "Logger.h"

Krampus::Exception::Exception(const Error& _error, const Error& _debugInfo)
{
	message = _error + " " + _debugInfo;
	if (_debugInfo == "") message += DEBUG_INFO;

	Krampus::Logger::Shutdown();
}

const char* Krampus::Exception::what() const noexcept
{
	return message.c_str();
}

const char* Krampus::Exception::What() const
{
	return what();
	
}
