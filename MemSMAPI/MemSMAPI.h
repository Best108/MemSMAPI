#pragma once
#include <string>
#include <vector>
#include "MemSM.h"
#include "WinAPI.h"

/*
	The MemAPI has to be completely 'functional', it can't be
	object orientated at all, because it's impossible to
	import a class implemented in C++ into C#. (Unless using COM or C++/CLI)

	DummyFunction can be called to see if the dll exists and is valid.
	If the dll doesn't exist or doesn't contain the DummyFunction the C# call will throw.
*/

namespace MemSMAPI
{
	struct ScanResult
	{
		unsigned int size;
		LPVOID* result;
	};

	extern MemSM* memSM;
	extern std::string lastExceptionMessage;

	extern __declspec(dllexport) void __cdecl DummyFunction();

	extern __declspec(dllexport) void WriteProcessMemory(LPVOID address, LPCVOID data, SIZE_T dataSize);

	extern __declspec(dllexport) void ReadProcessMemory(LPVOID address, LPVOID buffer, SIZE_T bufferSize);

	extern void SetLastExceptionMessage(const char* message);

	extern __declspec(dllexport) const char* __cdecl GetLastExceptionMessage();

	extern __declspec(dllexport) void __cdecl InitializeMemSMAPI(HANDLE MemSMHandle);
	
	extern __declspec(dllexport) bool __cdecl IsMemSMAPIInitialized();

	extern __declspec(dllexport) void __cdecl UninitializeMemSMAPI();

	extern __declspec(dllexport) ScanResult __cdecl ScanForBytes(LPCVOID startAddress, LPCVOID endAddress, LPCBYTE bytes, SIZE_T bytesSize);

	extern __declspec(dllexport) ScanResult __cdecl ScanForBytes(LPCVOID* addresses, SIZE_T sizeAddresses, LPCBYTE bytes, SIZE_T bytesSize);

	extern "C" __declspec(dllexport) ScanResult __cdecl ScanForPattern(LPCVOID startAddress, LPCVOID endAddress, LPCBYTE bytes, SIZE_T bytesSize, LPDWORD ignoreIndices, SIZE_T ignoreIndicesSize);
	
	extern __declspec(dllexport) void __cdecl DeleteScanResult(ScanResult scanResult);

	extern __declspec(dllexport) void __cdecl DeleteMemoryMap(MemoryMap memoryMap);

	extern __declspec(dllexport) MemoryMap CreateMemoryMap(LPCVOID startAddress, LPCVOID endAddress);
}