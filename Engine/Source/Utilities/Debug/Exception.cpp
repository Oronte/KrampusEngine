#include "Exception.h"
#include "Logger.h"

Krampus::Exception::Exception(const String& _error, const String& _debugInfo)
{
	message = _error + " " + _debugInfo;
	if (_debugInfo == "") message += DEBUG_INFO;

	// Write what was logged before the throw (the program may end on it), but keep the Logger running:
	// most exceptions are caught (a corrupted save, a bad scene...) and the game goes on.
	Krampus::Logger::Flush();
}

const char* Krampus::Exception::what() const noexcept
{
	return message;
}

const char* Krampus::Exception::What() const
{
	return what();
	
}
