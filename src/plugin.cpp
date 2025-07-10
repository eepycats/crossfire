#include <server_plugin.hpp>
#include <dt.hpp>
#include <mem.hpp>
#include <init.hpp>
#include <string>
#include <string_view>
#include <srcsdk/cvar.hpp>
#include <srcsdk/tier0.hpp>
#include <global.hpp>

using mem::byte_patch;
using mem::signature_scanner;

// 8B 44 24 ? 83 EC ? 53 8B D9 8D 48 -> 31 C0 40 C2 20 00
// 6A 20 89 4C 24 ? E8 ? ? ? ? 8B 54 24 -> 6A 10
// 6A 20 89 47 ? E8 -> 6A 10

namespace global{
	fnMsg Msg = reinterpret_cast<fnMsg>(mem::modexport<"tier0.dll", "Msg">());
	fnConColorMsg ConColorMsg = reinterpret_cast<fnConColorMsg>(mem::modexport<"tier0", "?ConColorMsg@@YAXABVColor@@PBDZZ">());	
}

bool client = false;
class crossfireplugin : public server_plugin {
	
	virtual const char* GetPluginDescription(void) override {
		return "crossfire :3";
	}

	virtual bool Load(pfnCreateInterface interfaceFactory, pfnCreateInterface gameServerFactory) override{
		using namespace global;

		using namespace ssdk::cvar;
		client = GetModuleHandle(L"client.dll") ? true : false;
		if (!client){
			using namespace ssdk::tier0;
			init_dedicated();
		}

		auto engine_module = mem::module("engine.dll");
		std::wstring cmdline = GetCommandLineW();

		static byte_patch exclusive(mem::rva<"engine">(0x147040), {0x31,0xc0,0xc3,0x90,0x90});
		static byte_patch lobby(mem::rva<"engine">(0x14752C), {0xEB});

		auto* cvar = interfaceFactory("VEngineCvar007", nullptr);
		if (cvar) {
			auto* iter = mem::vcall<41, ICVarIteratorInternal*>(cvar);
			for(iter->SetFirst(); iter->IsValid(); iter->Next())
			{
				ConCommandBase* cmd = iter->Get();
				if(cmd->m_nFlags & FCVAR_HIDDEN || cmd->m_nFlags & FCVAR_DEVELOPMENTONLY)
					cmd->m_nFlags &= ~(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
			}
		}
		auto* current_class = dt::GetAllServerClasses();
		while (current_class) {
			std::string_view name(current_class->m_pNetworkName);
			if (name == "CCSPlayerResource") {
				dt::destroy_member(current_class->m_pTable, "m_iAssists");
				dt::destroy_member(current_class->m_pTable, "m_szClan");
				dt::destroy_member(current_class->m_pTable, "m_iTotalCashSpent");
			}
			if (name == "CPlayerResource") {
				dt::destroy_member(current_class->m_pTable, "m_iAssists");
			}
			current_class = current_class->m_pNext;
		}
		init_net();
		wchar_t _title[512]{};
		dt::dump_sendtables("sendtables.txt");

		ConColorMsg({255,128,255,255},"hi hello\n");

		if (client) {			

			HWND main_window{};
			EnumWindows([](HWND hWnd, LPARAM param) {
				wchar_t classname[1024];
				static const std::wstring valve001(L"Valve001");
				static DWORD mypid = GetCurrentProcessId();
				DWORD pid;
				GetWindowThreadProcessId(hWnd, &pid);
				RealGetWindowClass(hWnd, classname, ARRAYSIZE(classname));
				if (pid == mypid && valve001 == classname) {
					auto mainwnd = reinterpret_cast<HWND*>(param);
					*mainwnd = hWnd;

					return FALSE;
				}
				return TRUE;
				}, reinterpret_cast<LPARAM>(&main_window));
			GetWindowText(main_window, _title, ARRAYSIZE(_title));
			std::wstring title(_title);
			title = L"\xD83D\xDC08" + title ;
			SetWindowText(main_window, title.c_str());
		}
		return true;
	};
};
PLUGIN_ENTRY(crossfireplugin)