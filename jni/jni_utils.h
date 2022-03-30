
#ifndef JAVSVM_JNI_UTILS_H
#define JAVSVM_JNI_UTILS_H

#include "jni.h"

#include "../object/jclass.h"
#include "../object/jobject.h"
#include "../object/jmethod.h"
#include "../engine/engine.h"
#include "../class/jclass_file.h"
#include "../vm/jvm.h"
#include "../gc/safety_point.h"

#include <memory>

/**
 * 这个类实现了 jni 和 javsvm 之间某些关键数据结构的双向映射，
 * 可作为 jni 层的拓展
 */

template <typename T>
static T take_from(const javsvm::jvalue &v) { }

//template <>
//inline void take_from(const javsvm::jvalue &v) {  }

template<>
inline jboolean take_from(const javsvm::jvalue &v) { return v.z; }

template<>
inline jbyte take_from(const javsvm::jvalue &v) { return v.b; }

template<>
inline jchar take_from(const javsvm::jvalue &v) { return v.c; }

template<>
inline jshort take_from(const javsvm::jvalue &v) { return v.s; }

template<>
inline jint take_from(const javsvm::jvalue &v) { return v.i; }

template<>
inline jlong take_from(const javsvm::jvalue &v) { return v.j; }

template<>
inline jfloat take_from(const javsvm::jvalue &v) { return v.f; }

template<>
inline jdouble take_from(const javsvm::jvalue &v) { return v.d; }

template <typename T = jobject>
static T to_object(javsvm::jref obj) noexcept
{
    auto &env = javsvm::jvm::get().env();
    auto frame = (javsvm::jni_stack_frame *) env.stack.top();
    assert(frame != nullptr);

    return (T) frame->append(obj);
}

template<>
inline jobject take_from(const javsvm::jvalue &v) { return to_object(v.l); }


template <typename T>
static javsvm::jvalue pack_to(T) {  }


//template <>
//inline void pack_to(jvalue &v) {  }

template<>
inline javsvm::jvalue pack_to(jboolean val) { return {.z = val }; }

template<>
inline javsvm::jvalue pack_to(jbyte val) { return { .b = val }; }

template<>
inline javsvm::jvalue pack_to(jchar val) { return { .c = val }; }

template<>
inline javsvm::jvalue pack_to(jshort val) { return { .s = val }; }

template<>
inline javsvm::jvalue pack_to(jint val) { return { .i = val }; }

template<>
inline javsvm::jvalue pack_to(jlong val) { return { .j = val }; }

template<>
inline javsvm::jvalue pack_to(jfloat val) { return { .f = val }; }

template<>
inline javsvm::jvalue pack_to(jdouble val) { return { .d = val }; }

static inline javsvm::jref to_object(jobject obj) noexcept
{
    if (obj == nullptr) {
        return nullptr;
    }
    // jobject 也可能是指向 gc_root/gc_weak 的指针，
    // 但 gc_root 的第一个成员变量就是 jref，gc_root* 和 jref* 是等价的
    return *(javsvm::jref *) obj;
}

template<>
inline javsvm::jvalue pack_to(jobject val) { return { .l = to_object(val) }; }


#define HAS_FLAG(x, f) (((x) & (f)) == (f))



/**
 * 将 jni 使用的 jclass 转为 javsvm 使用的 jclass*
 * @return 失败返回 nullptr
 */
static inline javsvm::jclass* to_class(jclass clazz) noexcept
{
    return javsvm::jclass::of(to_object(clazz));
}


using jargs_ptr = std::unique_ptr<javsvm::slot_t, void(*)(const javsvm::slot_t *)>;

static inline jargs_ptr make_jargs(javsvm::slot_t *p) noexcept
{
    return { p, [](const javsvm::slot_t*){  } };
}

/**
 * va_list 转 slot 数组
 */
static jargs_ptr to_args(jmethodID method, jobject obj, va_list ap)
{
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return make_jargs(nullptr);
    }

    auto args = new javsvm::slot_t[_method->args_slot];
    memset(args, 0, sizeof(javsvm::slot_t) * _method->args_slot);
    javsvm::jargs _args(args);

    // 如果是实例函数，需要留一个位置存放 this 对象
    if (!HAS_FLAG(_method->access_flag, javsvm::jclass_method::ACC_STATIC)) {
        _args.next<javsvm::jref>() = to_object(obj);
    }

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
                LOGE("to_args: unknown jmethod sig: '%s'\n", sig);
                break;
        }
    }
    return make_jargs(args);
}

/**
 * jvalue 数组转 slot 数组
 */
static jargs_ptr to_args(jmethodID method, jobject obj, const jvalue *ap)
{
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return make_jargs(nullptr);
    }


    auto args = new javsvm::slot_t[_method->args_slot];
    javsvm::jargs _args(args);

    // 如果是实例函数，需要留一个位置存放 this 对象
    if (!HAS_FLAG(_method->access_flag, javsvm::jclass_method::ACC_STATIC)) {
        _args.next<javsvm::jref>() = to_object(obj);
    }

    const auto sig = _method->sig;
    int ap_index = 0;

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
                _args.next<jint>() = ap[ap_index++].i;
                break;
            case 'J':       /* long */
                _args.next<jlong>() = ap[ap_index++].j;
                break;
            case 'F':       /* float */
                _args.next<jfloat>() = ap[ap_index++].f;
                break;
            case 'D':       /* double */
                _args.next<jdouble>() = ap[ap_index++].d;
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
                LOGE("to_args: unknown jmethod sig: '%s'\n", sig);
                break;
        }
    }
    return make_jargs(args);
}


extern JNIEnv *jni_env;

extern JavaVM *java_vm;



struct safety_area_guard
{
    safety_area_guard() noexcept
    {
        javsvm::leave_safety_area();
    }
    ~safety_area_guard() noexcept
    {
        javsvm::enter_safety_area();
    }

    safety_area_guard(const safety_area_guard&) = delete;
    safety_area_guard& operator=(const safety_area_guard&) = delete;
    safety_area_guard(safety_area_guard &&) = delete;
};

#endif // JAVSVM_JNI_UTILS_H