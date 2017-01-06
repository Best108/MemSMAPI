#include <memory>
#include "MemSM.h"

MemSM::MemSM(HANDLE processHandle)
{
	WinAPI::GetNativeSystemInfo(&systemInfo);

	DWORD exitCode;
	WinAPI::GetExitCodeProcess(processHandle, &exitCode);

	//Manually import some NTAPI functions. Pretty sick functions if you ask me
	NtSuspendProcess = reinterpret_cast<decltype(NtSuspendProcess)>(WinAPI::GetProcAddress(WinAPI::GetModuleHandle("ntdll.dll"), "NtSuspendProcess"));
	NtResumeProcess  = reinterpret_cast<decltype(NtResumeProcess)> (WinAPI::GetProcAddress(WinAPI::GetModuleHandle("ntdll.dll"), "NtResumeProcess"));

	//GetExitCodeProcess will return an exit code of STILL_ACTIVE if the Process
	//didn't exit yet
	if (exitCode != STILL_ACTIVE) {
		throw std::exception { "The Process is not running" };
	}

	this->processHandle = processHandle;
}

void MemSM::VirtualProtectEx(LPVOID address, SIZE_T size, DWORD newProtection, LPDWORD oldProtection) const
{
	WinAPI::VirtualProtectEx(processHandle, address, size, newProtection, oldProtection);
}

void MemSM::ReadProcessMemory(LPCVOID address, LPVOID buffer, SIZE_T bufferSize) const
{
	WinAPI::ReadProcessMemory(processHandle, address, buffer, bufferSize, nullptr);
}

void MemSM::WriteProcessMemory(LPVOID address, LPCVOID data, SIZE_T dataSize) const
{
	WinAPI::WriteProcessMemory(processHandle, address, data, dataSize, nullptr);
}

MemoryMap MemSM::CreateMemoryMap(LPCVOID startAddress, LPCVOID endAddress) const
{
	MemoryMap result { 0, systemInfo.dwPageSize, nullptr };
	Region* region { nullptr };

	LPCVOID currentAddress{ startAddress };

	MEMORY_BASIC_INFORMATION memoryBasicInformation;

	NtSuspendProcess(processHandle);

	while (currentAddress < endAddress) {
		try {
			WinAPI::VirtualQueryEx(processHandle, currentAddress, &memoryBasicInformation, sizeof(memoryBasicInformation));
		}
		catch (const WinAPIException& ex) {
			//This means there is no more memory to query
			if (ex.lastError == ERROR_INVALID_PARAMETER) {
				break;
			}

			//If something else happened, we will pass the exception to the caller
			throw ex;
		}

		if (!result.regions) {
			result.regions = region = new Region();
		}
		else {
			region = region->next = new Region();
		}

		++result.size;

		region->baseAddress = memoryBasicInformation.BaseAddress;

		region->regionSize = memoryBasicInformation.RegionSize;
		//regionSize % dwPageSize is always 0, there are no regions with for example half pages so integer division won't have unexpected results
		region->numberOfPages = region->regionSize / systemInfo.dwPageSize;

		region->state = memoryBasicInformation.State;

		//Here I check for the protection flags of the region. Doing & ( ... | ... | ...) will check if any of the flags are set
		region->readable = memoryBasicInformation.Protect != 0 && !(memoryBasicInformation.Protect & (PAGE_NOACCESS | PAGE_GUARD));
		region->writable = (memoryBasicInformation.Protect & (PAGE_READWRITE | PAGE_WRITECOMBINE | PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY)) != 0;
		region->executable = (memoryBasicInformation.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) != 0;

		region->type = memoryBasicInformation.Type;

		currentAddress = (reinterpret_cast<LPBYTE>(region->baseAddress) + region->regionSize);
	}

	NtResumeProcess(processHandle);

	return result;
}

void MemSM::DeleteMemoryMap(MemoryMap memoryMap)
{
	Region* region = memoryMap.regions;

	//loop throug LL and delete all nodes
	while (region) {
		Region* next { region->next };

		delete region;

		region = next;
	}
}

std::vector<LPVOID> MemSM::ScanForBytes(LPCVOID startAddress, LPCVOID endAddress, LPCBYTE bytes, SIZE_T bytesSize) const
{
	std::vector<LPVOID> result;

	MemoryMap memoryMap { CreateMemoryMap(startAddress, endAddress) };
	Region* region { memoryMap.regions };

	//The buffer will initally be able to hold 32 memory pages
	std::unique_ptr<BYTE> raiiBuffer { new BYTE[systemInfo.dwPageSize * 32] };
	size_t bufferSize { systemInfo.dwPageSize * 32 };

	NtSuspendProcess(processHandle);

	while (region) {
		if (region->state == MEM_COMMIT && region->readable && region->type != MEM_MAPPED) {
			//If there is not enough memory, the buffer has to be reallocated
			if (region->regionSize > bufferSize) {
				raiiBuffer.reset(new BYTE[region->regionSize]);
				bufferSize = region->regionSize;
			}

			ReadProcessMemory(region->baseAddress, raiiBuffer.get(), region->regionSize);


			//Search for the bytes in the region I just read from the MemSM
			for (DWORD i = 0; i < (region->regionSize - bytesSize + 1); ++i) {
				if (!memcmp(&raiiBuffer.get()[i], bytes, bytesSize)) {
					LPVOID address { reinterpret_cast<LPBYTE>(region->baseAddress) + i };

					//This check has to be done because the memory map will include regions out of that range. For more information see
					//the remarks of CreateMemoryMap
					if (address >= startAddress && address <= (reinterpret_cast<LPCBYTE>(endAddress) - bytesSize)) {
						result.push_back(address);
					}
				}
			}
		}

		region = region->next;
	}

	NtResumeProcess(processHandle);

	DeleteMemoryMap(memoryMap);

	return result;
}

std::vector<LPVOID> MemSM::ScanForBytes(const std::vector<LPCVOID>& addresses, LPCBYTE bytes, SIZE_T bytesSize) const
{
	std::vector<LPVOID> result;
	std::unique_ptr<BYTE> buffer { new BYTE[bytesSize] };

	NtSuspendProcess(processHandle);

	for (size_t i = 0; i < addresses.size(); ++i) {
		try {
			//if it throws, just continue to the next address
			ReadProcessMemory(addresses[i], buffer.get(), bytesSize);
		}
		catch (...) {
			continue;
		}

		//check if the bytes equal the bytes I just read
		if (!memcmp(buffer.get(), bytes, bytesSize)) {
			result.push_back(const_cast<LPVOID>(addresses[i]));
		}
	}

	NtResumeProcess(processHandle);

	return result;
}

std::vector<LPVOID> MemSM::ScanForPattern(LPCVOID startAddress, LPCVOID endAddress, LPCBYTE bytes, SIZE_T bytesSize, const std::vector<DWORD>& ignoreIndices) const
{
	std::vector<LPVOID> result;

	MemoryMap memoryMap { CreateMemoryMap(startAddress, endAddress) };
	Region* region { memoryMap.regions };

	//The buffer will initally be able to hold 32 memory pages
	std::unique_ptr<BYTE> raiiBuffer{ new BYTE[systemInfo.dwPageSize * 32] };
	size_t bufferSize{ systemInfo.dwPageSize * 32 };

	NtSuspendProcess(processHandle);

	while (region) {
		if (region->state == MEM_COMMIT && region->readable && region->type != MEM_MAPPED) {
			//If there is not enough memory, the buffer has to be reallocated
			if (region->regionSize > bufferSize) {
				raiiBuffer.reset(new BYTE[region->regionSize]);
				bufferSize = region->regionSize;
			}

			ReadProcessMemory(region->baseAddress, raiiBuffer.get(), region->regionSize);

			//Search for the bytes in the region I just read from the MemSM
			for (DWORD i = 0; i < (region->regionSize - bytesSize + 1); ++i) {
				bool patternMatches = true;

				for (size_t j = 0; j < bytesSize; ++j) {
					if (std::find(ignoreIndices.begin(), ignoreIndices.end(), j) != ignoreIndices.end()) {
						continue;
					}

					if (raiiBuffer.get()[i + j] != bytes[j]) {
						patternMatches = false;
						break;
					}
				}

				if (patternMatches) {
					LPVOID address { reinterpret_cast<LPBYTE>(region->baseAddress) + i };

					//This check has to be done because the memory map will include regions out of that range. For more information see
					//the remarks of CreateMemoryMap
					if (address >= startAddress && address <= (reinterpret_cast<LPCBYTE>(endAddress) - bytesSize)) {
						result.push_back(address);
					}
				}
			}
		}

		region = region->next;
	}

	NtResumeProcess(processHandle);

	DeleteMemoryMap(memoryMap);

	return result;
}