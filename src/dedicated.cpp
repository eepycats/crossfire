#include <srcsdk/tier0.hpp>
#include <global.hpp>
#include <cstdarg>
#include <cstdio>
#include <safetyhook.hpp>
#include <windows.h>
#include <mem.hpp>
#include <init.hpp>

using namespace global;
using namespace ssdk::tier0;

namespace w32 {
    static DWORD g_oldmode = 0;
    static CONSOLE_SCREEN_BUFFER_INFO g_bufinfo;

    void enable_truecolor() {
        DWORD dwMode;
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hOut, &g_bufinfo);
        GetConsoleMode(hOut, &dwMode);
        g_oldmode = dwMode;
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }

    void disable_truecolor() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleMode(hOut, g_oldmode);
        SetConsoleTextAttribute(hOut, g_bufinfo.wAttributes);
    }

	WORD get_terminal_state(){
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;

		if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
			return 0;
		}

		return csbi.wAttributes;
	}

	void restore_terminal_state(WORD attrib){
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    	SetConsoleTextAttribute(hConsole, attrib);
	}
}

safetyhook::InlineHook g_ConColorMsg;

void ConColorMsg_hook(const Color& clr, const char *pMsgFormat, ...){
    va_list __varargs;
    va_start(__varargs, pMsgFormat);
    char formattedMessage[2072];
    vsnprintf(formattedMessage, 0x800u, pMsgFormat, __varargs);
    va_end(__varargs);

    auto state = w32::get_terminal_state();

    Msg("\033[38;2;%i;%i;%im", +clr.r, +clr.g, +clr.b);
    Msg(formattedMessage);
    w32::restore_terminal_state(state);
}

void init_dedicated(){
    g_ConColorMsg = safetyhook::create_inline(mem::modexport<"tier0", "?ConColorMsg@@YAXABVColor@@PBDZZ">(), &ConColorMsg_hook);
    w32::enable_truecolor();
}