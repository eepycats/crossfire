#pragma once

namespace ssdk {
    namespace tier0 {
        struct ICommandLine {
            virtual void CreateCmdLine(int argc, char **argv) = 0;
            virtual void CreateCmdLine(const char *commandline) = 0;
            virtual const char* GetCmdLine(void) = 0;
            virtual const char* CheckParm(const char *psz, const char **ppszValue) = 0;
            virtual void RemoveParam(const char *pszParm) = 0;
            virtual void AppendParm(const char *pszParm, const char *pszValues) = 0;
            virtual float ParmValue(const char *psz, float flDefaultVal) = 0;
            virtual int ParmValue(const char *psz, int nDefaultVal) = 0;
            virtual const char* ParmValue(const char *psz, const char *pDefaultVal) = 0;
            virtual int ParmCount(void) = 0;
            virtual int FindParm(const char *psz) = 0;
            virtual const char * GetParm(int nIndex) = 0;
            virtual void SetParm(int nIndex, const char *pParm) = 0;
        /*
            virtual ~ICommandLine();
            virtual ~ICommandLine();
        */
        };
        #pragma pack(push, 1)
        struct Color // sizeof=0x4
        {                                       // XREF: .bss:UNSPECIFIED_LOGGING_COLOR/r
            // .bss:UNSPECIFIED_LOGGING_COLOR/r ...
            unsigned char r,g,b,a;
                // XREF: `global constructor keyed to'unitlib.cpp+E/w
                                                // `global constructor keyed to'unitlib.cpp+15/w ...
        };
        #pragma pack(pop)

    }
}