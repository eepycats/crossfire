#include "dt.hpp"
#include <cstring>
#include <Windows.h>
#include "mem.hpp"
#include <fstream>
#include <string_view>
#include <format>
namespace dt {
	using namespace ssdk::dt;
	void destroy_member(SendTable* table, std::string_view prop_name){
		for (int i = 0; i < table->m_nProps; i++){
			auto* ptr_Prop = &table->m_pProps[i];
			std::string_view prop_name_(ptr_Prop->m_pVarName);
			if (prop_name_ == prop_name){
				uintptr_t to_move = (table->m_nProps - i) * sizeof(SendProp);
				if (to_move > 0) {
					std::memmove(ptr_Prop, &table->m_pProps[i+1], to_move);
				}
				table->m_nProps--;
				return;
			}
		}
	}

	ServerClass* GetAllServerClasses() {
		using fnCreateInterface = void* (*)(const char*, int*);
		auto gamedll = reinterpret_cast<fnCreateInterface>(GetProcAddress(reinterpret_cast<HMODULE>(GetModuleHandleW(L"server.dll")), "CreateInterface"))("ServerGameDLL005", nullptr);
		return mem::vcall<10, ServerClass*>(gamedll);
	}

	void print_table(std::fstream& f, SendTable* table, int indent) {
		std::string indent_string(indent, '\t');
		for (int i = 0; i < table->m_nProps; i++) {
			auto prop = table->m_pProps[i];
			auto dt = prop.m_pDataTable;
			if (dt) {
				f << std::vformat("{}table: {} type: {}\n", std::make_format_args(indent_string, prop.m_pVarName, dt->m_pNetTableName));
				print_table(f, dt, indent + 1);
			}
			else {
				f << std::vformat("{}member: {}\n", std::make_format_args(indent_string, prop.m_pVarName));
			}
		}
	}

	void dump_sendtables(std::string_view filename) {
		std::fstream f(filename.data(), std::ios_base::out);
		auto current = GetAllServerClasses();
		while (current) {
			auto str = std::vformat("class {} (type {})\n", std::make_format_args(current->m_pNetworkName, current->m_pTable->m_pNetTableName));
			f << str;
			print_table(f, current->m_pTable, 0);
			current = current->m_pNext;
		}
	}
}
