#include <srcsdk/tier0.hpp>

// global function externs from other modules

namespace global{
    using namespace ssdk::tier0;
	using fnMsg = void(*)(const char* fmt, ...);
	using fnConColorMsg = void(*)(const Color& clr, const char *pMsgFormat, ...);
    extern fnMsg Msg;
	extern fnConColorMsg ConColorMsg;    
}