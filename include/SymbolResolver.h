#pragma once
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#endif

struct ResolvedFrame 
{
    std::string functionName;
    std::string fileName;
    uint32_t lineNumber;
};

class SymbolResolver 
{
public:
    SymbolResolver();
    ~SymbolResolver();

    std::vector<ResolvedFrame> Resolve(void** stack, int frames);

private:
    bool m_initialized = false;
};