

#ifndef JAVSVM_DLLLOADER_H
#define JAVSVM_DLLLOADER_H


#include <string>
#include <unordered_map>
#include "../concurrent/recursive_lock.h"

namespace javsvm
{


class dll_loader
{
private:
    std::unordered_map<std::string, void*> m_cache;
    recursive_lock m_lock;

    void *find_symbol0(const char *symbol, void *native_ptr = nullptr);
public:
    dll_loader() = default;
    dll_loader(const dll_loader &) = delete;
    dll_loader &operator=(const dll_loader &) = delete;
    ~dll_loader();

    void* load_library(const char *name);

    void free_library(const char *name);

    template <typename T>
    T find_symbol(const char *symbol, void *native_ptr = nullptr)
    {
        return (T) find_symbol0(symbol, native_ptr);
    }
};

};

#endif 