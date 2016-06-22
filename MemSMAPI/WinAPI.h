#pragma once
#include <Windows.h>
#include <string>
#include "Exception.h"

//Another type of exception that formats a WinAPI error code as a neat string
class WinAPIException final : public Exception
{
public:
	WinAPIException(DWORD lastError = GetLastError());

	const DWORD lastError;
private:
	static std::string GetLastErrorAsString(DWORD lastError);
};

//This is simply a wrapper for the WinAPI functions I use.
//If they fail, they throw exceptions, which makes error handling way
//easier.
namespace WinAPI
{
	void WINAPI GetNativeSystemInfo(LPSYSTEM_INFO lpSystemInfo);
	BOOL WINAPI GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode);
	SIZE_T WINAPI VirtualQueryEx(HANDLE hProcess, LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength);
	BOOL WINAPI ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead);
	BOOL WINAPI WriteProcessMemory(HANDLE  hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);
	BOOL WINAPI VirtualProtectEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
	FARPROC WINAPI GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
	HMODULE WINAPI GetModuleHandle(LPCTSTR lpModuleName);
}