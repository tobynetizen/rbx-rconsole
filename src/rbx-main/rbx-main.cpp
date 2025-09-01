#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <string>

constexpr auto MODE_PRINT = 0;
constexpr auto MODE_INFO = 1;
constexpr auto MODE_WARN = 2;
constexpr auto MODE_ERROR = 3;

int ASLR(int offset)
{
	return (offset - 0x00400000 + reinterpret_cast<uintptr_t>(GetModuleHandle(NULL)));
}

void ConsoleBypass()
{
	DWORD OldProtect;
	VirtualProtect((PVOID)&FreeConsole, 1, PAGE_EXECUTE_READWRITE, &OldProtect);
	*(BYTE*)&FreeConsole = 0xC3;
}

typedef int(__cdecl* SINGLETON)(int a1, int a2, int a3);
SINGLETON SingletonPrint = (SINGLETON)ASLR(0x642A10);

void Main()
{
	ConsoleBypass();
	AllocConsole();
	if (!freopen("CONOUT$", "w", stdout)) {
		std::cerr << "Failed to redirect stdout!" << std::endl;
	}
	if (!freopen("CONIN$", "r", stdin)) {
		std::cerr << "Failed to redirect stdin!" << std::endl;
	}
	SetConsoleTitleA("rconsole");
	HWND hConsoleW = GetConsoleWindow();
	SetWindowPos(hConsoleW, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	ShowWindow(hConsoleW, SW_NORMAL);

	std::string userInput;
	int outputMode = MODE_PRINT;

	for (;;)
	{
		std::cout << "> ";
		getline(std::cin, userInput);
		if (userInput == "exit") { ExitProcess(EXIT_SUCCESS); }
		if (userInput == "print") { outputMode = MODE_PRINT; }
		if (userInput == "info") { outputMode = MODE_INFO; }
		if (userInput == "warn") { outputMode = MODE_WARN; }
		if (userInput == "error") { outputMode = MODE_ERROR; }
		if (userInput != "print" && userInput != "info" && userInput != "warn" && userInput != "error")
		{
			SingletonPrint(outputMode, (int)userInput.c_str(), 6);
		}
	}
}

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&Main, 0, 0, 0);
		break;
	}

	return TRUE;
}