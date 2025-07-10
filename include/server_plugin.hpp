#pragma once
#ifndef SERVERPLUGIN_HPP
#define SERVERPLUGIN_HPP
#include <cstdint>
#include <cstring>

class server_plugin_newcvar {
public:
	using pfnCreateInterface = void* (*)(const char*, int*);
	virtual bool Load(pfnCreateInterface interfaceFactory, pfnCreateInterface gameServerFactory) {
		return true;
	};

	virtual void			Unload(void) {
	};
	virtual void			Pause(void) {}
	virtual void			UnPause(void) {}

	virtual const char* GetPluginDescription(void) {
		return "";
	}

	virtual void			LevelInit(char const* pMapName) {}
	virtual void			ServerActivate(uintptr_t pEdictList, int edictCount, int clientMax) {}
	virtual void			GameFrame(bool simulating) {}
	virtual void			LevelShutdown(void) {}

	virtual void			ClientActive(uintptr_t pEntity) {}
	virtual void			ClientDisconnect(uintptr_t pEntity) {}

	virtual void			unk(uintptr_t) {} // abysmal

	virtual void			ClientPutInServer(uintptr_t pEntity, char const *playername) {}
	virtual void			SetCommandClient(int index) {}
	virtual void			ClientSettingsChanged(uintptr_t pEdict) {}
	virtual int	ClientConnect(bool* bAllowConnect, uintptr_t pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) {
		return 0;
	}

	virtual int ClientCommand(void* pEntity, void* args) {
		return 0;
	}

	virtual int	NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) {
		return 0;
	}
	virtual void			OnQueryCvarValueFinished(int iCookie, uintptr_t pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) {}
	virtual void			OnEdictAllocated(uintptr_t edict) {}
	virtual void			OnEdictFreed(const uintptr_t edict) {}
};

class server_plugin_oldcvar {
public:
	using pfnCreateInterface = void* (*)(const char*, int*);
	virtual bool Load(pfnCreateInterface interfaceFactory, pfnCreateInterface gameServerFactory) {
		return true;
	};

	virtual void			Unload(void) {
	};
	virtual void			Pause(void) {}
	virtual void			UnPause(void) {}

	virtual const char* GetPluginDescription(void) {
		return "";
	}

	virtual void			LevelInit(char const* pMapName) {}
	virtual void			ServerActivate(uintptr_t pEdictList, int edictCount, int clientMax) {}
	virtual void			GameFrame(bool simulating) {}
	virtual void			LevelShutdown(void) {}
	virtual void			ClientActive(uintptr_t pEntity) {}
	virtual void			ClientDisconnect(uintptr_t pEntity) {}
	virtual void			ClientPutInServer(uintptr_t pEntity, char const* playername) {}
	virtual void			SetCommandClient(int index) {}
	virtual void			ClientSettingsChanged(uintptr_t pEdict) {}
	virtual int	ClientConnect(bool* bAllowConnect, uintptr_t pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen) {
		return 0;
	}

	virtual int ClientCommand(void* pEntity) {
		return 0;
	}

	virtual int	NetworkIDValidated(const char* pszUserName, const char* pszNetworkID) {
		return 0;
	}
	virtual void			OnQueryCvarValueFinished(int iCookie, uintptr_t pPlayerEntity, int eStatus, const char* pCvarName, const char* pCvarValue) {}
	virtual void			OnEdictAllocated(uintptr_t edict) {}
	virtual void			OnEdictFreed(const uintptr_t edict) {}
};

using server_plugin = server_plugin_newcvar;

#define PLUGIN_ENTRY(T) extern "C" __declspec(dllexport) void* CreateInterface(const char* name, int* _) {	\
	static T _plugin;																						\
	if (strstr(name, "ISERVERPLUGINCALLBACKS")) {															\
		return &_plugin;																					\
	}																										\
	else {																									\
		return 0;																							\
	}																										\
}
#endif