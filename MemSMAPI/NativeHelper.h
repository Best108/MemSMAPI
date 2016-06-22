#pragma once
#include "WinAPI.h"

namespace NativeHelper
{
	//This can also be done in C#, but for some reason it
	//only works for a few processes in C
	extern __declspec(dllexport) const char* GetProcessMainModulePath(HANDLE MemSMHandle);
}