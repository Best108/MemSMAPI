#include "MemSMAPI.h"
#include <Psapi.h>
#include <sstream>

MemSM* MemSMAPI::memSM { nullptr };
//This string will hold the last exception message
std::string MemSMAPI::lastExceptionMessage;

void MemSMAPI::DummyFunction()
{

}

void MemSMAPI::WriteProcessMemory(LPVOID address, LPCVOID data, SIZE_T dataSize)
{
	if (MemSMAPI::IsMemSMAPIInitialized()) {
		//Unprotect memory so that it can be written to even if it's not writable
		DWORD oldProtections[2];
		memSM->VirtualProtectEx(address, dataSize, PAGE_EXECUTE_READWRITE, &oldProtections[0]);
		memSM->WriteProcessMemory(address, data, dataSize);
		memSM->VirtualProtectEx(address, dataSize, oldProtections[0], &oldProtections[1]);
	}
}

void MemSMAPI::ReadProcessMemory(LPVOID address, LPVOID buffer, SIZE_T bufferSize)
{
	if (MemSMAPI::IsMemSMAPIInitialized()) {
		memSM->ReadProcessMemory(address, buffer, bufferSize);
	}
}

void MemSMAPI::SetLastExceptionMessage(const char* message)
{
	lastExceptionMessage = std::string(message);
}

const char* MemSMAPI::GetLastExceptionMessage()
{
	return lastExceptionMessage.c_str();
}

void MemSMAPI::InitializeMemSMAPI(HANDLE MemSMHandle)
{
	UninitializeMemSMAPI();

	memSM = new MemSM { MemSMHandle };
}

bool MemSMAPI::IsMemSMAPIInitialized()
{
	return memSM != nullptr;
}

void MemSMAPI::UninitializeMemSMAPI()
{
	if (MemSMAPI::IsMemSMAPIInitialized()) {
		delete memSM;
		memSM = nullptr;
	}
}

MemSMAPI::ScanResult MemSMAPI::ScanForBytes(LPCVOID startAddress, LPCVOID endAddress, LPCBYTE bytes, SIZE_T numberOfBytes)
{
	ScanResult scanResult { 0, nullptr };

	if (MemSMAPI::IsMemSMAPIInitialized()) {
		std::vector<LPVOID> result;

		//Scan for the bytes
		result = memSM->ScanForBytes(startAddress, endAddress, bytes, numberOfBytes);

	
		//store the results in a simple array
		if (result.size() > 0) {
			scanResult.size = result.size();
			scanResult.result = new LPVOID[scanResult.size];

			memcpy(scanResult.result, result.data(), scanResult.size * sizeof(LPVOID));
		}
	}

	return scanResult;
}

MemSMAPI::ScanResult MemSMAPI::ScanForBytes(LPCVOID* addresses, SIZE_T sizeAddresses, LPCBYTE bytes, SIZE_T bytesSize)
{
	ScanResult scanResult { 0, nullptr };

	if (MemSMAPI::IsMemSMAPIInitialized()) {
		std::vector<LPVOID> result;

		//Convert the addresses to an std::vector and scan
		result = memSM->ScanForBytes(std::vector<LPCVOID>(addresses, addresses + sizeAddresses), bytes, bytesSize);

		//Store the results in a simple array
		if (result.size() > 0) {
			scanResult.size = result.size();
			scanResult.result = new LPVOID[scanResult.size];

			memcpy(scanResult.result, result.data(), scanResult.size * sizeof(LPVOID));
		}
	}

	return scanResult;
}

MemSMAPI::ScanResult MemSMAPI::ScanForPattern(LPCVOID startAddress, LPCVOID endAddress, LPCBYTE bytes, SIZE_T bytesSize, LPDWORD ignoreIndices, SIZE_T ignoreIndicesSize)
{
	ScanResult scanResult { 0, nullptr };

	if (MemSMAPI::IsMemSMAPIInitialized()) {
		std::vector<LPVOID> result;

		//Convert the addresses and indices to an std::vector and scan
		result = memSM->ScanForPattern(startAddress, endAddress, bytes, bytesSize, std::vector<DWORD>(ignoreIndices, ignoreIndices + ignoreIndicesSize));

		//Store the results in a simple array
		if (result.size() > 0) {
			scanResult.size = result.size();
			scanResult.result = new LPVOID[scanResult.size];

			memcpy(scanResult.result, result.data(), scanResult.size * sizeof(LPVOID));
		}
	}

	return scanResult;
}

//Simply delete the array
void MemSMAPI::DeleteScanResult(ScanResult scanResult)
{
	if (scanResult.result) {
		delete[] scanResult.result;
	}
}

void MemSMAPI::DeleteMemoryMap(MemoryMap memoryMap)
{
	if (MemSMAPI::IsMemSMAPIInitialized()) {
		memSM->DeleteMemoryMap(memoryMap);
	}
}

MemoryMap MemSMAPI::CreateMemoryMap(LPCVOID startAddress, LPCVOID endAddress)
{
	MemoryMap memoryMap { 0, 0, nullptr };

	if (MemSMAPI::IsMemSMAPIInitialized()) {
		//this one doesn't have to be converted, it can simply be returned
		memoryMap = memSM->CreateMemoryMap(startAddress, endAddress);
	}

	return memoryMap;
}