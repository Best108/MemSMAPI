#include "Exception.h"
#include "MemSMAPI.h"

Exception::Exception(const char* message) :
	std::exception { message }
{
	//When an exception is thrown, it's message will get set as the last exception message
	MemSMAPI::SetLastExceptionMessage(message);
}