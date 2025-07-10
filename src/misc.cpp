
#include <safetyhook.hpp>
#include <mem.hpp>
#include <init.hpp>

safetyhook::InlineHook g_antileak;

const char* __fastcall antileak(void* ecx, void* edx){
	return ":3";
}

void init_antileak(){
    static const char* anti_leak_lol = "   Public IP is *paws at you*\n";
    auto patch_loc = mem::rva<"engine">(0x140C22);
    #pragma pack(push, 1)
    struct fuckpatch_t{
        uint8_t op = 0x68;
        const char* ptr = anti_leak_lol;
    } fuckpatch;
    #pragma pack(pop)
    mem::patch_bytes(patch_loc,{reinterpret_cast<uint8_t*>(&fuckpatch), sizeof(fuckpatch)});
    g_antileak = safetyhook::create_inline(mem::rva<"engine">(0x1FE890), antileak);
}