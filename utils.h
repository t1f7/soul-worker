#pragma once

#include <Windows.h>
#include <string>
#include <iostream>
#include <thread>

#pragma warning(disable : 4244 4996 ) 

void loop();
void interact();
auto jumpTime = std::chrono::steady_clock::now();

// read-write console
class outbuf : public std::streambuf {
public:
	outbuf() {
		setp(0, 0);
	}

	virtual int_type overflow(int_type c = traits_type::eof()) {
		return fputc(c, stdout) == EOF ? traits_type::eof() : c;
	}
};

bool GetConsole() {
	if (!AllocConsole()) {
		if (FreeConsole()) {
			if (!AllocConsole()) {
				return false;
			}
		}
	}
	return true;
}

HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;
char conin[256];


void AttachConsole()
{
	_old_out = GetStdHandle(STD_OUTPUT_HANDLE);
	_old_err = GetStdHandle(STD_ERROR_HANDLE);
	_old_in = GetStdHandle(STD_INPUT_HANDLE);

	::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

	_out = GetStdHandle(STD_OUTPUT_HANDLE);
	_err = GetStdHandle(STD_ERROR_HANDLE);
	_in = GetStdHandle(STD_INPUT_HANDLE);

	SetConsoleMode(_out,
		ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

	// create the console
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	SetConsoleTitleA("Try me");
	SetConsoleTextAttribute(_out, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);

	// set std::cout to use my custom streambuf
	outbuf ob;
	std::streambuf *sb = std::cout.rdbuf(&ob);
	// make sure to restore the original so we don't get a crash on close!
	std::cout.rdbuf(sb);
}

void DetachConsole()
{
	if (_out && _err && _in) {
		FreeConsole();

		if (_old_out)
			SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
		if (_old_err)
			SetStdHandle(STD_ERROR_HANDLE, _old_err);
		if (_old_in)
			SetStdHandle(STD_INPUT_HANDLE, _old_in);
	}
}

bool ConsolePrint(const char* fmt, ...)
{
	if (!_out)
		return false;

	char buf[1024];
	va_list va;

	va_start(va, fmt);
	_vsnprintf_s(buf, 1024, fmt, va);
	va_end(va);

	return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
}

#include "lua.h"

HANDLE hMain;
HINSTANCE g_hModule;
DWORD g_threadID;
std::thread mainThread;
std::thread bindThread;

bool crashed = false;


void FreezeLoop() {
	while (true) {
		Sleep(INT_MAX);
	}
}

void AllowDebugging() {
	// Allow debugging
	DWORD dwAddr = (DWORD)GetProcAddress(GetModuleHandleA("KERNELBASE.dll"), "IsDebuggerPresent");
	DWORD dwVirtualProtectBackup;
	bool result = VirtualProtect((BYTE*)dwAddr, 0x20, PAGE_READWRITE, &dwVirtualProtectBackup);
	if (result == NULL)
	{
		ConsolePrint("Failed to patch debugging.\n");
	}
	else
	{
		*(BYTE*)(dwAddr) = 0xB8;
		memset((BYTE*)(dwAddr + 0x1), 0x0, 0x4);
		*(BYTE*)(dwAddr + 0x5) = 0xC3;
		VirtualProtect((BYTE*)dwAddr, 0x20, dwVirtualProtectBackup, &dwVirtualProtectBackup);
	}
}

LONG CALLBACK CrashHandler(PEXCEPTION_POINTERS pExceptionInfo) {
	crashed = true;
	ConsolePrint(("VEH 0x%p\n"), pExceptionInfo->ExceptionRecord->ExceptionCode);

	if (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION || pExceptionInfo->ExceptionRecord->ExceptionCode == 0xE06D7363) {

		if (pExceptionInfo->ExceptionRecord->ExceptionCode == 0xE06D7363) ConsolePrint("0xE06D7363 veh\n");

		pExceptionInfo->ContextRecord->Eip = (DWORD)&FreezeLoop;

		Sleep(100);

		crashed = false;

		bindThread.detach();
		bindThread.~thread();
		bindThread = std::thread(interact);

		mainThread.detach();
		mainThread.~thread();
		mainThread = std::thread(loop);

		return EXCEPTION_CONTINUE_EXECUTION;
	}

	/*if (pExceptionInfo->ExceptionRecord->ExceptionCode == 0xE06D7363) {
		ConsolePrint("0xE06D7363 again?!");
		return EXCEPTION_CONTINUE_EXECUTION;
	}*/

	return EXCEPTION_CONTINUE_SEARCH;
}