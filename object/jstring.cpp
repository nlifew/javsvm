

#include "jstring.h"
#include "jobject.h"
#include "jarray.h"
#include "jclass.h"
#include "jmethod.h"
#include "jfield.h"
#include "vm/jvm.h"
#include "utils/log.h"
#include "utils/string_utils.h"

using namespace javsvm;


static std::wstring str2wstr(const char *str)
{
    std::wstring wstr;
    wchar_t *buff = string_utils::towstring(str);
    if (buff != nullptr) {
        wstr.append(buff);
        delete[] buff;
    }

    return wstr;
}


bool jstring::bind_java_class()
{
    if (java_lang_String != nullptr) {
        return true;
    }

    java_lang_String = jclass::find_class("java/lang/String");
    if (java_lang_String == nullptr) {
        return false;
    }
    
    java_lang_String_init = java_lang_String->get_method("<init>", "([C)V");
    if (java_lang_String_init == nullptr) {
        return false;
    }

    java_lang_String_value = java_lang_String->get_field("value", "[C");
    if (java_lang_String_value == nullptr) {
        return false;
    }

    return true;
}


jref jstring::new_string(const std::wstring& wstr)
{
    if (! bind_java_class()) {
        LOGE("java_lang_String hasn't been init\n");
        return nullptr;
    }

    int len = wstr.length();

    jref char_array = jarray::new_char_array(len);
    jarray::set_char_array_region(char_array, 0, len, (jchar*) wstr.c_str());

    jref obj = java_lang_String->new_instance(java_lang_String_init, char_array);

    return obj;
}


jref jstring::new_string(const char *str)
{
    std::wstring wstr = str2wstr(str);
    return new_string(wstr);
}


jref jstring::find_or_new(const char *str)
{
    std::wstring wstr = str2wstr(str);

    return m_cache.put_if_absent(wstr, [this, wstr]() -> jref {
        return new_string(wstr);
    });
}



std::wstring jstring::get_string(jref ref)
{
    std::wstring wstr;

    if (! bind_java_class()) {
        LOGE("java_lang_String hasn't be init yet\n");
        return wstr;
    }

    jvalue v = java_lang_String_value->get(ref);
    int len = jarray::get_array_length(v.l);

    jchar *buff = new jchar[len];
    std::unique_ptr<jchar, void (*)(jchar *)> buff_guard(buff, [](jchar *p) { delete[] p; });

    jarray::get_char_array_region(v.l, 0, len, buff);
    wstr.append((wchar_t *)buff);

    return wstr;
}


jref jstring::intern(jref str)
{
    // if (jobject::is_null(str)) {
        // return nullptr;
    // }

    std::wstring wstr = get_string(str);

    return m_cache.put_if_absent(wstr, [str]() -> jref {
        return str;
    });
}
