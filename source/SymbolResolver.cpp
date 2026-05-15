#include "../include/SymbolResolver.h"
#include <cstdio>

#ifdef _WIN32
#pragma comment(lib, "dbghelp.lib")

SymbolResolver::SymbolResolver() 
{
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    HANDLE process = GetCurrentProcess();
    if (SymInitialize(process, NULL, TRUE)) 
    {
        m_initialized = true;
    }
}

SymbolResolver::~SymbolResolver() 
{
    if (m_initialized) 
    {
        SymCleanup(GetCurrentProcess());
    }
}

std::vector<ResolvedFrame> SymbolResolver::Resolve(void** stack, int frames)
{
    std::vector<ResolvedFrame> result;
    if (!m_initialized) return result;

    HANDLE process = GetCurrentProcess();
    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    for (int i = 0; i < frames; i++) 
    {
        if (stack[i] == nullptr) break;

        DWORD64 address = (DWORD64)stack[i];
        ResolvedFrame frame = { "UnknownFunction", "", 0 };

        if (SymFromAddr(process, address, 0, pSymbol)) 
        {
            frame.functionName = pSymbol->Name;
        }

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement;
        if (SymGetLineFromAddr64(process, address, &displacement, &line)) 
        {
            frame.fileName = line.FileName;
            frame.lineNumber = line.LineNumber;
        }

        result.push_back(frame);
    }
    return result;
}
#endif