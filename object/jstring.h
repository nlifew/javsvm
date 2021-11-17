

#ifndef JAVSVM_JSTRING_H
#define JAVSVM_JSTRING_H

#include "utils/global.h"

#include <string>
#include <concurrent/concurrent_map.h>

namespace javsvm
{

struct jfield;
struct jclass;
struct jmethod;

class jstring
{
private:
    concurrent_map<std::wstring, jref> m_cache;

    // std::shared_mutex m_lock;
    // std::unordered_map<std::wstring, jref> m_cache;

    jfield *java_lang_String_value = nullptr;
    jclass *java_lang_String = nullptr;
    jmethod *java_lang_String_init = nullptr;

    bool bind_java_class();
    jref new_string(const std::wstring &);
    std::wstring get_string(jref);

public:

    jref new_string(const char *s);

    jref find_or_new(const char *s);

    jref intern(jref str);
};

} // namespace javsvm

#endif


