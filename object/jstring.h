

#ifndef JAVSVM_JSTRING_H
#define JAVSVM_JSTRING_H

#include "../utils/global.h"
#include "../concurrent/concurrent_map.h"

#include <string>

namespace javsvm
{

struct jvm;

class jstring
{
private:
    concurrent_map<std::string, jref> m_cache;

    // std::shared_mutex m_lock;
    // std::unordered_map<std::wstring, jref> m_cache;

    jvm &m_jvm;

    jref new_string(const char *str);

public:
    explicit jstring(jvm &vm) noexcept :
        m_jvm(vm)
    {
    }

    ~jstring() = default;
    jstring(const jstring &) = delete;
    jstring& operator=(jstring const&) = delete;

    jref find(const char *s) const;

    jref find_or_new(const char *s);

    jref intern(jref str);
};

} // namespace javsvm

#endif


