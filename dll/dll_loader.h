

#ifndef JAVSVM_DLLLOADER_H
#define JAVSVM_DLLLOADER_H


#include <mutex>
#include <string>
#include <unordered_map>

namespace javsvm
{

class dll_wrapper;

class dll_loader
{
private:
    std::mutex m_lock;
    std::unordered_map<std::string, dll_wrapper *> m_cache;
    

public:
    dll_loader() = default;
    dll_loader(const dll_loader &) = delete;
    dll_loader &operator=(const dll_loader &) = delete;
    ~dll_loader();

    void load_library(const char *name);
};

};

#endif 