
#include "dll_wrapper.h"

using namespace javsvm;

#ifdef _WIN32
#include <wtypes.h>
#include <winbase.h>
#else
#include <dlfcn.h>
#endif


dll_wrapper::dll_wrapper(const char *dll)
{
#ifdef _WIN32
    m_native_ptr = LoadLibraryA(dll);
#else
    m_native_ptr = dlopen(dll, RTLD_NOW | RTLD_GLOBAL);
#endif
}

dll_wrapper::~dll_wrapper()
{
#ifdef _WIN32
    FreeLibrary((HMODULE) m_native_ptr);
#else
    dlclose(m_native_ptr);
#endif
    m_native_ptr = nullptr;
}


void* dll_wrapper::find(const char *func, int /* unused */)
{
#ifdef _WIN32
    return GetProcAddress((HMODULE) m_native_ptr, func);
#else
    return dlsym(m_native_ptr, func);
#endif
}

dll_wrapper::dll_wrapper(const dll_wrapper& o)
{
    m_native_ptr = o.m_native_ptr;
    o.~dll_wrapper();
}

dll_wrapper& dll_wrapper::operator=(const dll_wrapper& o)
{
    this->~dll_wrapper();
    m_native_ptr = o.m_native_ptr;
    o.~dll_wrapper();
}
