#include "utils.h"


void loop()
{
	if (game_L != NULL) {
		AttachConsole();
	}

	while (!crashed) {
		if (game_L == NULL) {
			continue;
		}

		// Sleep(2200);

		std::cin.getline(conin, sizeof(conin));
		if (conin[0] == '!') {
			if (conin[1] == '1') {
				Lua::lua_getfield(game_L, LUA_GLOBALSINDEX, ("_G"));
				Lua::luaL_register(game_L, NULL, printlib); // for Lua versions < 5.2
															//luaL_setfuncs(L, printlib, 0);  // for Lua versions 5.2 or greater
				Lua::lua_settop(game_L, -(1) - 1);
			}
			if (conin[1] == '2') {
				for (auto it : lua_States) {
					Lua::lua_pushstring(it, "_G");
					Lua::lua_gettable(it, LUA_GLOBALSINDEX);
					print_table(it, 0);
				}
			}
			if (conin[1] == '3') {
				auto current_state = lua_States.find(game_L);
				auto end_state = lua_States.end();
				std::advance(current_state, 1);
				if (current_state == end_state) {
					ConsolePrint(("Use first state\n"));
					game_L = *lua_States.begin();
				}
				else {
					ConsolePrint(("Increment state\n"));
					game_L = *current_state;
				}
			}

		}
		else {
			ExecuteString(game_L, conin);
		}
	}

	// FreeLibraryAndExitThread(g_hModule, 0);
}

void interact()
{
	while (!crashed)
	{
		if (game_L != NULL) {
			auto now = std::chrono::steady_clock::now();
			auto isTime = (std::chrono::duration_cast<std::chrono::milliseconds>(now - jumpTime).count() >= 100);
			if (GetAsyncKeyState(VK_MBUTTON) && isTime) {  // force jump
				jumpTime = std::chrono::steady_clock::now();
				ExecuteString(game_L, "G.Client.OwnerPlayer.Obj:JumpStart(true)");
			}
		}
		Sleep(5);
	}
}


DWORD WINAPI MyThread(LPVOID) {

	// We need this because game can crash with some lua commands.
	// And also this game likes to crash a lot without hacks..
	AddVectoredExceptionHandler(true, CrashHandler);

	// Threads to restart hack in case lua command crashed the game but VEH stopped it
	mainThread = std::thread(loop);
	bindThread = std::thread(interact);

	// Attemp to remove antidebugging
	AllowDebugging();

	// Allow us to use spawned console as lua console.
	AttachConsole();

	if (LuaHook() != 1) {
		ConsolePrint("LuaHook failed.");
		std::cin.getline(conin, sizeof(conin));
		FreeLibraryAndExitThread(g_hModule, 0);
		return 0;
	}

	while (1)
	{
		Sleep(10);
	}

	ConsolePrint("Somehow we've exited our internal thread.");
	FreeLibraryAndExitThread(g_hModule, 0);
	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:  // Use manual map
		g_hModule = hDll;
		DisableThreadLibraryCalls(hDll);
		hMain = CreateThread(NULL, NULL, &MyThread, NULL, NULL, &g_threadID);
		break;

	case DLL_PROCESS_DETACH:  // Manual mapping detach ???
		break;
	}
	return TRUE;
}
