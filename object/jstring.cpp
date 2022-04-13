

#include "jstring.h"
#include "jobject.h"
#include "jarray.h"
#include "jclass.h"
#include "jmethod.h"
#include "jfield.h"
#include "../vm/jvm.h"
#include "../utils/strings.h"

using namespace javsvm;



static jfield *java_lang_String_value = nullptr;
static jclass *java_lang_String = nullptr;
//static jmethod *java_lang_String_init = nullptr;
static jmethod *java_lang_String_init2 = nullptr;

static bool bind_java_class()
{
    // 直接使用 bootstrap_loader 加载
    bootstrap_loader &loader = jvm::get().bootstrap_loader;

    if (java_lang_String == nullptr && (java_lang_String = loader.load_class("java/lang/String")) == nullptr) {
        return false;
    }
//    if (java_lang_String_init == nullptr && (java_lang_String_init = java_lang_String->get_method("<init>", "([BII)V")) == nullptr) {
//        return false;
//    }
    if (java_lang_String_init2 == nullptr && (java_lang_String_init2 = java_lang_String->get_method("<init>", "([CZ)V")) == nullptr) {
        return false;
    }
    if (java_lang_String_value == nullptr && (java_lang_String_value = java_lang_String->get_field("value", "[C")) == nullptr) {
        return false;
    }
    return true;
}


jref jstring::new_string(const char* str) noexcept
{
    auto wstr = strings::towstring(str);
    return new_string(wstr.c_str(), wstr.length());
}


jref jstring::new_string(const wchar_t * str, int len) noexcept
{
    if (! bind_java_class()) {
        LOGE("java_lang_String hasn't been init\n");
        exit(1);
    }

    jref chars_array = m_jvm.array.new_char_array(len);
    m_jvm.array.set_char_array_region(chars_array, 0, len, (jchar *) str);

    jref obj = java_lang_String->new_instance(java_lang_String_init2, chars_array, true);
    return obj;
}


jref jstring::value_of(jref ref) noexcept
{
    if (! bind_java_class()) {
        LOGE("java_lang_String hasn't been init\n");
        exit(1);
    }
    return java_lang_String_value->get(ref).l;
}

int jstring::length(jref ref) const noexcept
{
    if (! bind_java_class()) {
        LOGE("java_lang_String hasn't been init\n");
        exit(1);
    }
    jref char_array = java_lang_String_value->get(ref).l;
    int char_array_len = m_jvm.array.get_array_length(char_array);
    return char_array_len;
}



jref jstring::find(const char *str) const
{
    std::string s(str);
    jref *ref = const_cast<jref*>(m_cache.get(s));
    return ref == nullptr ? nullptr : *ref;
}

jref jstring::find_or_new(const char *str)
{
    return m_cache.put_if_absent(str, [this, str]() -> jref {
        return new_string(str);
    });
}




jref jstring::intern(jref str)
{
    if (! bind_java_class()) {
        LOGE("intern: java_lang_String hasn't been init\n");
        exit(1);
    }
    jref char_array = java_lang_String_value->get(str).l;
    int char_array_len = m_jvm.array.get_array_length(char_array);

    auto *buff = new jchar[char_array_len];
    std::unique_ptr<jchar, void(*)(const jchar*)> buff_guard(
            buff, [](const jchar *ptr) { delete[] ptr; });

    m_jvm.array.get_char_array_region(char_array, 0, char_array_len, buff);

    auto utf8 = strings::tostring((wchar_t *) buff);
    return m_cache.put_if_absent(utf8, [str]() -> jref {
        return str;
    });
}
