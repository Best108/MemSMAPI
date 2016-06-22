#pragma once
#include <exception>

/*
	Extends std::exception and sets the message it gets constructed
	with as lastExceptionMessage, so that it can be received in C#
*/
class Exception : public std::exception
{
public:
	Exception(const char* message);
};