#include "WinAPI.h"

WinAPIException::WinAPIException(DWORD lastError) :
	lastError { lastError },
	Exception { GetLastErrorAsString(lastError).c_str() }
{

}

//This function returns a string description of the lastError. It will be the same as in the WinAPI documentation
std::string WinAPIException::GetLastErrorAsString(DWORD lastError)
{
	LPSTR messageBuffer { nullptr };
	DWORD size { FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr) };

	std::string message { size == 0 ? "Default WinAPIException message" : std::string(messageBuffer, size) };

	message += " [" + std::to_string(lastError) + "]";

	HeapFree(GetProcessHeap(), 0, messageBuffer);

	return message;
}

void WINAPI WinAPI::GetNativeSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
	::GetNativeSystemInfo(lpSystemInfo);
}

BOOL WINAPI WinAPI::GetExitCodeProcess(HANDLE hMemSM, LPDWORD lpExitCode)
{
	BOOL result { ::GetExitCodeProcess(hMemSM, lpExitCode) };

	if (!result) {
		throw WinAPIException();
	}

	return result;
}

SIZE_T WINAPI WinAPI::VirtualQueryEx(HANDLE hMemSM, LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength)
{
	SIZE_T result { ::VirtualQueryEx(hMemSM, lpAddress, lpBuffer, dwLength) };

	if (!result) {
		throw WinAPIException();
	}

	return result;
}

BOOL WINAPI WinAPI::ReadProcessMemory(HANDLE hMemSM, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead)
{
	BOOL result { ::ReadProcessMemory(hMemSM, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead) };

	if (!result) {
		throw WinAPIException();
	}

	return result;
}

BOOL WINAPI WinAPI::WriteProcessMemory(HANDLE hMemSM, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten)
{
	BOOL result { ::WriteProcessMemory(hMemSM, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten) };

	if (!result) {
		throw WinAPIException();
	}

	return result;
}

BOOL WINAPI WinAPI::VirtualProtectEx(HANDLE hMemSM, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
{
	BOOL result { ::VirtualProtectEx(hMemSM, lpAddress, dwSize, flNewProtect, lpflOldProtect) };

	if (!result) {
		throw WinAPIException();
	}

	return result;
}

FARPROC WINAPI WinAPI::GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC result { ::GetProcAddress(hModule, lpProcName) };

	if (!result) {
		throw WinAPIException();
	}

	return result;
}

HMODULE WINAPI WinAPI::GetModuleHandle(LPCTSTR lpModuleName)
{
	HMODULE result { ::GetModuleHandle(lpModuleName) };

	if (!result) {
		throw WinAPIException();
	}

	return result;
}
