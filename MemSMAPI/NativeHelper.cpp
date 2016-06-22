#include "NativeHelper.h"
#include <Psapi.h>

const char* NativeHelper::GetProcessMainModulePath(HANDLE MemSMHandle)
{
	static char mainModulePath[MAX_PATH];

	GetModuleFileNameExA(MemSMHandle, nullptr, mainModulePath, MAX_PATH);

	return mainModulePath;
}