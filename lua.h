#pragma once
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <unordered_set>

// minhook
#include "MinHook.h"
#pragma comment(lib, "MinHook.x86.lib")

bool ignore_lua_newthread = false;
bool ignore_lua_close = false;
bool ignore_lua_open = false;


typedef lua_State *(*game_lua_open)(lua_Alloc f, void *ud);
typedef lua_State *(*game_lua_newthread)(lua_State *L);
typedef void(*game_lua_close)(lua_State *L);
typedef int(*game_luaopen_io)(lua_State *L);
typedef int(*game_lua_dostring)(lua_State *L, const char *str);
typedef int(*game_lua_dofile)(lua_State *L, const char *filename);
typedef int(*game_lua_gettop)(lua_State *L);
typedef const char * (*game_lua_tostring)(lua_State *L, int idx);
typedef const char * (*game_lua_tolstring)(lua_State *L, int idx, size_t *len);
typedef void(*game_lua_settop)(lua_State *L, int idx);
typedef int(*game_lua_next)(lua_State *L, int idx);
typedef lua_Number(*game_lua_tonumber)(lua_State *L, int idx);
typedef const void * (*game_lua_topointer)(lua_State *L, int idx);
typedef void(*game_lua_pushstring)(lua_State *L, const char *s);
typedef void(*game_lua_gettable)(lua_State *L, int idx);
typedef void(*game_lua_pushnil)(lua_State *L);
typedef int(*game_lua_isstring)(lua_State *L, int idx);
typedef int(*game_lua_isnumber)(lua_State *L, int idx);
typedef int(*game_lua_type)(lua_State *L, int idx);
typedef int(*game_lua_loadbuffer)(lua_State *L, const char *buff, size_t sz,
	const char *name);
typedef int(*game_lua_pcall)(lua_State *L, int nargs, int nresults, int errfunc);

typedef void(*game_lua_getfield)(lua_State *L, int idx, const char *k);
typedef void(*game_luaL_register)(lua_State *L, const char *libname,
	const luaL_Reg *l);

game_lua_open orig_lua_open = NULL;
game_lua_close orig_lua_close = NULL;
game_lua_gettop orig_gettop = NULL;
game_lua_newthread orig_lua_newthread = NULL;
lua_State * game_L = NULL;

std::unordered_set<lua_State *> lua_States;

namespace Lua {
	game_lua_open lua_open = NULL;
	game_lua_close lua_close = NULL;
	game_luaopen_io luaopen_io = NULL;
	game_lua_dostring lua_dostring = NULL;
	game_lua_dofile lua_dofile = NULL;
	game_lua_gettop lua_gettop = NULL;
	game_lua_tostring lua_tostring = NULL;
	game_lua_tolstring lua_tolstring = NULL;
	game_lua_settop lua_settop = NULL;
	game_lua_next lua_next = NULL;
	game_lua_tonumber lua_tonumber = NULL;
	game_lua_topointer lua_topointer = NULL;

	game_lua_pushstring lua_pushstring = NULL;
	game_lua_gettable lua_gettable = NULL;
	game_lua_pushnil lua_pushnil = NULL;

	game_lua_isstring lua_isstring = NULL;
	game_lua_isnumber lua_isnumber = NULL;
	game_lua_type lua_type = NULL;
	game_lua_loadbuffer lua_loadbuffer = NULL;
	game_lua_newthread lua_newthread = NULL;
	game_lua_pcall lua_pcall = NULL;
	game_lua_getfield lua_getfield = NULL;
	game_luaL_register luaL_register = NULL;
};

// lua ??
static int l_my_print(lua_State* L) {
	int nargs = Lua::lua_gettop(L);
	std::cout << "> ";
	for (int i = 1; i <= nargs; ++i) {
		if (Lua::lua_isstring(L, i)) {
			std::cout << Lua::lua_tolstring(L, i, NULL);
			//Lua::lua_settop(L, -(1) - 1);
		}
		else {
			std::cout << "nil";
		}
	}
	std::cout << std::endl;
	Lua::lua_settop(L, -(1) - 1);

	return 0;
}

static const struct luaL_Reg printlib[] = {
	{ "print", l_my_print },
{ NULL, NULL } /* end of array */
};

//https://stackoverflow.com/questions/20523044/loop-through-all-lua-global-variables-in-c#20525495
//http://www.lua.org/manual/5.1/manual.html#lua_next
//http://lua-users.org/lists/lua-l/2004-04/msg00201.html
void print_table(lua_State *L, int depth) {

	Lua::lua_pushnil(L); // put a nil key on stack
	while (Lua::lua_next(L, -2) != 0) {
		auto istring = Lua::lua_isstring(L, -1);
		auto luanumber = Lua::lua_isnumber(L, -1);
		auto luatype = Lua::lua_type(L, -1);
		auto luatype2 = Lua::lua_type(L, -2);
		if (istring) {
			auto tostr = Lua::lua_tolstring(L, -2, NULL);
			auto tostr2 = Lua::lua_tolstring(L, -1, NULL);
			ConsolePrint("%i STR %s = %s\n", depth, tostr, tostr2);
		}
		else if (luanumber) {
			auto tostr = Lua::lua_tolstring(L, -2, NULL);
			ConsolePrint("%i NUM %s = 0x%f\n", depth, tostr, luanumber);
		}
		else if (luatype == LUA_TTABLE && luatype2 == LUA_TSTRING) {
			auto table_pointer = Lua::lua_topointer(L, -1);
			auto table_name = Lua::lua_tolstring(L, -2, NULL);
			ConsolePrint("%i TAB %s = 0x%p\n", depth, table_name, table_pointer, "\n");

			if (std::string(table_name).compare("G") == 0 && depth < 3) {
				Lua::lua_settop(L, -(1) - 1);
				Lua::lua_pushstring(L, table_name);
				Lua::lua_gettable(L, LUA_GLOBALSINDEX);
				print_table(L, depth + 1);
			}
			else if (std::string(table_name).compare("Soulworker") == 0 && depth < 3) {
				Lua::lua_settop(L, -(1) - 1);
				Lua::lua_pushstring(L, table_name);
				Lua::lua_gettable(L, LUA_GLOBALSINDEX);
				print_table(L, depth + 1);
			}
			else if (std::string(table_name).compare("Client") == 0 && depth < 3) {
				Lua::lua_settop(L, -(1) - 1);
				Lua::lua_pushstring(L, table_name);
				Lua::lua_gettable(L, LUA_GLOBALSINDEX);
				print_table(L, depth + 1);
			}
			else if (std::string(table_name).compare("Game") == 0 && depth < 3) {
				Lua::lua_settop(L, -(1) - 1);
				Lua::lua_pushstring(L, table_name);
				Lua::lua_gettable(L, LUA_GLOBALSINDEX);
				print_table(L, depth + 1);
			}
			else {
				//Lua::lua_pushstring(L, table_name);
				//Lua::lua_gettable(L, LUA_GLOBALSINDEX);
				//print_table(L, depth+1);
			}
		}
		else if (luatype == LUA_TFUNCTION) {
			auto topointer = Lua::lua_topointer(L, -1);
			auto tostr = Lua::lua_tolstring(L, -2, NULL);
			ConsolePrint("%i FUN %s = 0x%p\n", depth, tostr, topointer);
		}
		Lua::lua_settop(L, -(1) - 1);
	}

}

//lua_State * hooked_lua_open(void) {
lua_State * hooked_lua_open(lua_Alloc f, void *ud)
{
	auto L = orig_lua_open(f, ud);
	if (!ignore_lua_open) {
		if (!game_L) game_L = L;
		lua_States.insert(L);
		ConsolePrint(("In lua_open, lua_State 0x%p\n"), L);
	}

	return L;
}

void hooked_lua_close(lua_State *L) {

	if (!ignore_lua_close) ConsolePrint(("In lua_close, lua_State 0x%p\n"), L);

	//lua_States.erase(L);
	//orig_lua_close(L);
}

lua_State * hooked_lua_newthread(lua_State *L) {
	auto thread = orig_lua_newthread(L);
	if (!ignore_lua_newthread) ConsolePrint(("In lua_newthread, lua_State 0x%p, thread 0x%p\n"), L, thread);
	return thread;
}

int hooked_lua_gettop(lua_State *L) {
	if (game_L == NULL) game_L = L;
	if (!lua_States.count(L) && !ignore_lua_newthread) { // if state is already recorded, don't debug print
		ConsolePrint(("In lua_gettop, lua_State 0x%p\n"), L);
		lua_States.insert(L);

		// debug
		game_L = L;
	}
	return orig_gettop(L);
}

void ExecuteString(lua_State *L, std::string buff)
{
	ignore_lua_open = true;
	ignore_lua_newthread = true;
	ignore_lua_close = true;

	auto NT = Lua::lua_newthread(L);

	if (!NT)
	{
		ConsolePrint(("Error %s\n"), Lua::lua_tolstring(L, -1, NULL));
		return;
	}

	Lua::lua_pushstring(NT, "_G");
	Lua::lua_pushstring(NT, "G");
	Lua::lua_gettable(NT, LUA_GLOBALSINDEX);

	if (Lua::lua_loadbuffer(NT, buff.c_str(), strlen(buff.c_str()), "exec"))
	{
		//there was an error
		ConsolePrint(("Error %s\n"), Lua::lua_tolstring(NT, -1, NULL));
	}
	//logwrite("PRE lua_pcall");
	if (Lua::lua_pcall(NT, 0, -1, 0))
	{
		//there was an erro
		ConsolePrint(("Error %s\n"), Lua::lua_tolstring(NT, -1, NULL));
	}
	Lua::lua_close(NT);


	Lua::lua_settop(L, -(1) - 1);

	ignore_lua_close = false;
	ignore_lua_newthread = false;
	ignore_lua_open = false;
}

int LuaHook() {
	HMODULE hLua = GetModuleHandleA(("lua100.dll"));
	if (hLua == NULL) {
		ConsolePrint(("GetModuleHandle failed\n"));
		return 0;
	}

	while (hLua == NULL) {
		hLua = GetModuleHandleA(("lua100.dll"));
		ConsolePrint(("GetModuleHandle failed\n"));
		return 0;
	}

	ConsolePrint("Lua module at 0x%p\n", hLua);

	Lua::lua_open = (game_lua_open)GetProcAddress(hLua, "lua_newstate");
	ConsolePrint("lua_open at 0x%p\n", Lua::lua_open);

	Lua::lua_getfield = (game_lua_getfield)GetProcAddress(hLua, "lua_getfield");
	ConsolePrint("lua_pcall at 0x%p\n", Lua::lua_getfield);

	Lua::luaL_register = (game_luaL_register)GetProcAddress(hLua, "luaL_register");
	ConsolePrint("luaL_register at 0x%p\n", Lua::luaL_register);

	Lua::lua_pcall = (game_lua_pcall)GetProcAddress(hLua, "lua_pcall");
	ConsolePrint("lua_pcall at 0x%p\n", Lua::lua_pcall);
	Lua::lua_newthread = (game_lua_newthread)GetProcAddress(hLua, "lua_newthread");
	ConsolePrint("lua_newthread at 0x%p\n", Lua::lua_newthread);
	Lua::lua_close = (game_lua_close)GetProcAddress(hLua, "lua_close");
	ConsolePrint("lua_close at 0x%p\n", Lua::lua_close);
	Lua::luaopen_io = (game_luaopen_io)GetProcAddress(hLua, "luaopen_io");
	ConsolePrint("luaopen_io at 0x%p\n", Lua::luaopen_io);
	Lua::lua_dostring = (game_lua_dostring)GetProcAddress(hLua, "luaL_loadstring");
	ConsolePrint("lua_dostring at 0x%p\n", Lua::lua_dostring);
	Lua::lua_dofile = (game_lua_dofile)GetProcAddress(hLua, "luaL_loadfile");
	ConsolePrint("lua_dofile at 0x%p\n", Lua::lua_dofile);

	Lua::lua_gettop = (game_lua_gettop)GetProcAddress(hLua, "lua_gettop");
	ConsolePrint("lua_gettop at 0x%p\n", Lua::lua_gettop);
	Lua::lua_tostring = (game_lua_tostring)GetProcAddress(hLua, "lua_tolstring");
	ConsolePrint("lua_tostring at 0x%p\n", Lua::lua_tostring);
	Lua::lua_tolstring = (game_lua_tolstring)GetProcAddress(hLua, "lua_tolstring");
	ConsolePrint("lua_tolstring at 0x%p\n", Lua::lua_tolstring);
	Lua::lua_settop = (game_lua_settop)GetProcAddress(hLua, "lua_settop");
	ConsolePrint("lua_pop at 0x%p\n", Lua::lua_settop);
	Lua::lua_next = (game_lua_next)GetProcAddress(hLua, "lua_next");
	ConsolePrint("lua_next at 0x%p\n", Lua::lua_next);
	Lua::lua_tonumber = (game_lua_tonumber)GetProcAddress(hLua, "lua_tonumber");
	ConsolePrint("lua_tonumber at 0x%p\n", Lua::lua_tonumber);
	Lua::lua_topointer = (game_lua_topointer)GetProcAddress(hLua, "lua_topointer");
	ConsolePrint("lua_topointer at 0x%p\n", Lua::lua_topointer);
	Lua::lua_pushstring = (game_lua_pushstring)GetProcAddress(hLua, "lua_pushstring");
	ConsolePrint("lua_pushstring at 0x%p\n", Lua::lua_pushstring);
	Lua::lua_gettable = (game_lua_gettable)GetProcAddress(hLua, "lua_gettable");
	ConsolePrint("lua_gettable at 0x%p\n", Lua::lua_gettable);
	Lua::lua_pushnil = (game_lua_pushnil)GetProcAddress(hLua, "lua_pushnil");
	ConsolePrint("lua_pushnil at 0x%p\n", Lua::lua_pushnil);
	Lua::lua_loadbuffer = (game_lua_loadbuffer)GetProcAddress(hLua, "luaL_loadbuffer");

	Lua::lua_isstring = (game_lua_isstring)GetProcAddress(hLua, "lua_isstring");
	ConsolePrint("lua_isstring at 0x%p\n", Lua::lua_isstring);
	Lua::lua_isnumber = (game_lua_isnumber)GetProcAddress(hLua, "lua_isnumber");
	ConsolePrint("lua_isnumber at 0x%p\n", Lua::lua_isnumber);
	Lua::lua_type = (game_lua_type)GetProcAddress(hLua, "lua_type");
	ConsolePrint("lua_type at 0x%p\n", Lua::lua_type);

	// Initialize MinHook.
	if (MH_Initialize() != MH_OK)
	{
		ConsolePrint("Initialize failed\n");
		return 0;
	}

	// Create a hook 
	if (MH_CreateHook((PVOID *)Lua::lua_open, &hooked_lua_open, reinterpret_cast<LPVOID*>(&orig_lua_open)) != MH_OK)
	{
		ConsolePrint(("Create hook 1 failed"));
		return 0;
	}

	// Enable the hook 
	if (MH_EnableHook((PVOID *)Lua::lua_open) != MH_OK)
	{
		ConsolePrint(("Enable hook 1 failed"));
		return 0;
	}

	// Create a hook 
	if (MH_CreateHook((PVOID *)Lua::lua_close, &hooked_lua_close, reinterpret_cast<LPVOID*>(&orig_lua_close)) != MH_OK)
	{
		ConsolePrint(("Create hook 2 failed"));
		return 0;
	}

	// Enable the hook 
	if (MH_EnableHook((PVOID *)Lua::lua_close) != MH_OK)
	{
		ConsolePrint(("Enable hook 2 failed"));
		return 0;
	}

	// Create a hook 
	if (MH_CreateHook((PVOID *)Lua::lua_gettop, &hooked_lua_gettop, reinterpret_cast<LPVOID*>(&orig_gettop)) != MH_OK)
	{
		ConsolePrint(("Create hook 3 failed"));
		return 0;
	}

	// Enable the hook 
	if (MH_EnableHook((PVOID *)Lua::lua_gettop) != MH_OK)
	{
		ConsolePrint(("Enable hook 3 failed"));
		return 0;
	}

	// Create a hook 
	if (MH_CreateHook((PVOID *)Lua::lua_newthread, &hooked_lua_newthread, reinterpret_cast<LPVOID*>(&orig_lua_newthread)) != MH_OK)
	{
		ConsolePrint(("Create hook 4 failed"));
		return 0;
	}

	// Enable the hook 
	if (MH_EnableHook((PVOID *)Lua::lua_newthread) != MH_OK)
	{
		ConsolePrint(("Enable hook 4 failed"));
		return 0;
	}

	return 1;
}