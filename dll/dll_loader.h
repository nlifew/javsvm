

#ifndef JAVSVM_DLL_LOADER_H
#define JAVSVM_DLL_LOADER_H


#include <string>
#include <unordered_map>
#include "../concurrent/recursive_lock.h"

namespace javsvm
{

class jvm;

class dll_loader
{
private:
    jvm &m_vm;
    std::unordered_map<std::string, void*> m_cache;
    recursive_lock m_lock;

    int call_JNI_OnLoad(void *symbol) noexcept;

public:
    explicit dll_loader(jvm *vm) noexcept:
        m_vm(*vm)
    {
    }


    dll_loader(const dll_loader &) = delete;
    dll_loader &operator=(const dll_loader &) = delete;
    ~dll_loader();

    void* load_library(const char *name);

    void free_library(const char *name);

    void* find_symbol(const char *symbol, void *native_ptr = nullptr);
};

};

#endif 