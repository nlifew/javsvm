

#include "../object/jclass.h"
#include "../object/jobject.h"
#include "../object/jfield.h"
#include "../object/jmethod.h"
#include "../engine/engine.h"
#include "../class/jclass_file.h"

#include <memory>


template <typename T>
static T take_from(const jvalue &v) { }

template <>
inline void take_from(const jvalue &v) {  }

template<>
inline jboolean take_from(const jvalue &v) { return v.z; }

template<>
inline jbyte take_from(const jvalue &v) { return v.b; }

template<>
inline jchar take_from(const jvalue &v) { return v.c; }

template<>
inline jshort take_from(const jvalue &v) { return v.s; }

template<>
inline jint take_from(const jvalue &v) { return v.i; }

template<>
inline jlong take_from(const jvalue &v) { return v.j; }

template<>
inline jfloat take_from(const jvalue &v) { return v.f; }

template<>
inline jdouble take_from(const jvalue &v) { return v.d; }

template<>
inline jobject take_from(const jvalue &v) { return v.l; }


template <typename T>
static void pack_to(jvalue &value, const T &t) {  }


//template <>
//inline void pack_to(jvalue &v) {  }

template<>
inline void pack_to(jvalue &v, const jboolean &z) { v.z = z; }

template<>
inline void pack_to(jvalue &v, const jbyte &b) { v.b = b; }

template<>
inline void pack_to(jvalue &v, const jchar &c) { v.c = c; }

template<>
inline void pack_to(jvalue &v, const jshort &s) { v.s = s; }

template<>
inline void pack_to(jvalue &v, const jint &i) { v.i = i; }

template<>
inline void pack_to(jvalue &v, const jlong &j) { v.j = j; }

template<>
inline void pack_to(jvalue &v, const jfloat &f) { v.f = f; }

template<>
inline void pack_to(jvalue &v, const jdouble &d) { v.d = d; }

template<>
inline void pack_to(jvalue &v, const jobject &l) { v.l = l; }


#define HAS_FLAG(x, f) (((x) & (f)) == (f))



static inline javsvm::jref to_object(jobject obj) noexcept
{
    // todo 此处直接强转罢，反正也没有 gc ==
    return (javsvm::jref) obj;
}


/**
 * 将 jni 使用的 jclass 转为 javsvm 使用的 jclass*
 * @return 失败返回 nullptr
 */
static inline javsvm::jclass* to_class(jclass clazz) noexcept
{
    return javsvm::jclass::of(to_object(clazz));
}

/**
 * jni 层使用的 JNIEnv* 转为 javsvm 使用的 jenv*
 */
static inline javsvm::jenv *to_env(JNIEnv *env)
{
    // todo
    return nullptr;
}

/**
 * va_list 转 slot 数组
 * 调用者需要 delete[] 释放内存
 */
static javsvm::slot_t *to_args(jmethodID method, va_list ap)
{
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return nullptr;
    }

    auto args = new javsvm::slot_t[_method->args_slot];
    javsvm::jargs _args(args);

    const auto sig = _method->sig;
    for (int i = 1; sig[i] != ')'; i ++) {
        switch (sig[i]) {
            case 'Z':       /* boolean */
                _args.next<jboolean>() = va_arg(ap, jint) != 0;
                break;
            case 'B':       /* byte */
                _args.next<jbyte>() = va_arg(ap, jint) & 0xFF;
                break;
            case 'C':       /* char */
                _args.next<jchar>() = va_arg(ap, jint) & 0xFFFF;
                break;
            case 'S':       /* short */
                _args.next<jshort>() = va_arg(ap, jint) & 0xFFFF;
                break;
            case 'I':       /* int */
                _args.next<jint>() = va_arg(ap, jint);
                break;
            case 'J':       /* long */
                _args.next<jlong>() = va_arg(ap, jlong);
                break;
            case 'F':       /* float */
                _args.next<jfloat>() = va_arg(ap, jdouble);
                break;
            case 'D':       /* double */
                _args.next<jdouble>() = va_arg(ap, jdouble);
                break;
            case 'L':       /* object */
                _args.next<javsvm::jref>() = to_object(va_arg(ap, jobject));
                i = (int) (strchr(sig + i + 1, ';') - sig);
                break;
            case '[':       /* array */
                _args.next<javsvm::jref>() = to_object(va_arg(ap, jarray));
                while (sig[i] == '[') i ++;
                if (sig[i] == 'L') i = (int) (strchr(sig + i + 1, ';') - sig);
                break;
            default:
                LOGE("unknown jmethod sig: '%s'\n", sig);
                break;
        }
    }
    return args;
}

/**
 * jvalue 数组转 slot 数组
 * 调用者需要 delete[] 释放内存
 */
static javsvm::slot_t *to_args(jmethodID method, const jvalue *ap)
{
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return nullptr;
    }

    int ap_index = 0;

    auto args = new javsvm::slot_t[_method->args_slot];
    javsvm::jargs _args(args);

    const auto sig = _method->sig;
    for (int i = 1; sig[i] != ')'; i ++) {
        switch (sig[i]) {
            case 'Z':       /* boolean */
                _args.next<jboolean>() = ap[ap_index++].z;
                break;
            case 'B':       /* byte */
                _args.next<jbyte>() = ap[ap_index++].b;
                break;
            case 'C':       /* char */
                _args.next<jchar>() = ap[ap_index++].c;
                break;
            case 'S':       /* short */
                _args.next<jshort>() = ap[ap_index++].s;
                break;
            case 'I':       /* int */
                _args.next<jint>() =  ap[ap_index++].i;
                break;
            case 'J':       /* long */
                _args.next<jlong>() =  ap[ap_index++].j;
                break;
            case 'F':       /* float */
                _args.next<jfloat>() =  ap[ap_index++].f;
                break;
            case 'D':       /* double */
                _args.next<jdouble>() =  ap[ap_index++].d;
                break;
            case 'L':       /* object */
                _args.next<javsvm::jref>() = to_object(ap[ap_index++].l);
                i = (int) (strchr(sig + i + 1, ';') - sig);
                break;
            case '[':       /* array */
                _args.next<javsvm::jref>() = to_object(ap[ap_index++].l);
                while (sig[i] == '[') i ++;
                if (sig[i] == 'L') i = (int) (strchr(sig + i + 1, ';') - sig);
                break;
            default:
                LOGE("unknown jmethod sig: '%s'\n", sig);
                break;
        }
    }
    return args;
}
