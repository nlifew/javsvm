

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
public:
    jref new_string(const char *str) noexcept;

    jref new_string(const wchar_t *str) noexcept;


    int length(jref ref) noexcept;

    /**
     * 获取某个字符串内的数组引用
     * @return 失败返回 nullptr
     */
    jref value_of(jref ref) noexcept;


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


