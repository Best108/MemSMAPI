#pragma once
#include <vector>
#include "WinAPI.h"

/*
	Contains information about a Region
*/
struct Region
{
	LPVOID baseAddress;

	DWORD regionSize;
	DWORD numberOfPages;

	DWORD state;

	BOOL readable;
	BOOL writable;
	BOOL executable;

	DWORD type;

	Region* next;
};

struct MemoryMap
{
	DWORD size;
	DWORD pageSize;
	Region* regions;
};

class MemSM
{
public:
	MemSM(HANDLE processHandle);

	void VirtualProtectEx(LPVOID address, SIZE_T size, DWORD newProtection, LPDWORD oldProtection) const;

	void ReadProcessMemory(LPCVOID address, LPVOID buffer, SIZE_T bufferSize) const;

	void WriteProcessMemory(LPVOID address, LPCVOID data, SIZE_T dataSize) const;

	/*
		Queries the memory starting at startAddress and ending at endAddress.
		If startAddress and endAddress don't specify addresses of respectively a beginning
		or ending of a memory region the returned vector will contain information
		starting at the beginning of the region that startAddress includes and ending until the last
		region that overlapps endAddress.
	*/
	MemoryMap CreateMemoryMap(LPCVOID startAddress, LPCVOID endAddress) const;
	
	/*
		Deletes a memoryMap by looping through all nodes.
	*/
	static void DeleteMemoryMap(MemoryMap memoryMap);

	/*
		Scans for bytesSize bytes pointed to by the pointer bytes in the range startAddress and endAddress.
	*/
	std::vector<LPVOID> ScanForBytes(LPCVOID startAddress, LPCVOID endAddress, LPCBYTE bytes, SIZE_T bytesSize) const;

	/*
		Goes through all the address and checks if the bytes at each address equals the bytes parameter.
		The ones that equal the parameter will be returned as a new vector.
	*/
	std::vector<LPVOID> ScanForBytes(const std::vector<LPCVOID>& addresses, LPCBYTE bytes, SIZE_T bytesSize) const;

	/*
		Scans for bytesSize bytes pointed to by the pointer bytes in the range startAddress and endAddress and ignores ignoreIndices.
	*/
	std::vector<LPVOID> ScanForPattern(LPCVOID startAddress, LPCVOID endAddress, LPCBYTE bytes, SIZE_T bytesSize, const std::vector<DWORD>& ignoreIndices) const;

private:
	HANDLE processHandle;
	SYSTEM_INFO systemInfo;

	NTSTATUS (NTAPI* NtSuspendProcess)(HANDLE hProcess);
	NTSTATUS (NTAPI* NtResumeProcess)(HANDLE hProcess);

};