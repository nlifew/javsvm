
/**
 * native 函数执行逻辑
 */

#include "engine.h"
#include "jnilib.h"
#include "../vm/jvm.h"
#include "../jni/jni.h"
#include "../jni/jni_utils.h"

#include <array>
#include <cstdlib>

using namespace javsvm;


static void *find_entrance(jmethod *method)
{
    auto &loader = jvm::get().dll_loader;

    auto short_name = jni_short_name(method);
    LOGI("find_entrance: try to use short name '%s'\n", short_name.c_str());

    auto ptr = loader.find_symbol(short_name.c_str());
    if (ptr != nullptr) {
        return ptr;
    }
    auto long_name = jni_long_name(method);
    LOGI("find_entrance: try to use short name '%s'\n", long_name.c_str());

    return loader.find_symbol(long_name.c_str());
}


static_assert(sizeof(javsvm::jvalue) == sizeof(int64_t));

extern "C" javsvm::jvalue calljni64(
        const void *addr,			    /* 函数地址，由 load_library() 获取得到，不能是 nullptr */
        int return_type,			    /* 返回类型，0 表示返回 int64，非 0 返回浮点数.  */
        const int64_t* integers,	    /* 固定长度为 8 的数组，数组内的值将被写入到 x0-x7 通用寄存器 */
        const javsvm::jdouble* floats,  /* 固定长度为 16 的数组，数组内的值将被写入到 d0-d15 浮点数寄存器 */
        int stack_len,				    /* 表示要压进栈的参数大小，单位是字节，为非负的整数。需要调用者自己对齐到 16 字节 */
        const void *stack			    /* 要压进栈的参数的起始地址。高地址会被先压进栈。 */
);

/**
 * @tparam align16 使用栈传递参数时是否强制 8 字节对齐
 */
template <bool align8>
class args_stack
{
private:
    std::array<int64_t, 8> m_registers{};
    int m_register_size = 0;

    std::array<javsvm::jdouble, 16> m_float_registers{};
    int m_float_register_size = 0;

    char *m_stack = nullptr;
    int m_stack_size = 0;
    int m_stack_capacity = 0; // 16 的整数倍

//    static inline int align(int in, int to) noexcept
//    {
//        return ((in - 1) | (to - 1)) + 1;
//    }
public:
    explicit args_stack() noexcept = default;

    ~args_stack() noexcept
    {
        free(m_stack);
        m_stack = nullptr;
    }

    args_stack(const args_stack &) = delete;
    args_stack& operator=(const args_stack&) = delete;

    /**
     * 将参数压进通用寄存器，或者压进栈
     */
    template<typename T>
    void push_integer(T val) noexcept
    {
        // 如果通用寄存器没有装满，使用通用寄存器
        if (m_register_size < m_registers.size()) {
            m_registers[m_register_size ++] = (int64_t) val;
            return;
        }
        // 使用栈传递
        // 先计算需要多少个填充字节
        int padding = 0;
        if (! align8) {
            padding = align<sizeof(T)>(m_stack_size) - m_stack_size; // align(m_stack_size, sizeof(T)) - m_stack_size;
            LOGI("push_integer: m_stack_index = %d, sizeof(T) = %lu, padding = %d\n",
                 m_stack_size, sizeof(T), padding);
        }

        if (m_stack_size + padding + sizeof(T) >= m_stack_capacity) {
            // 2 倍扩容
            int new_cap = std::max(m_stack_capacity * 2, 16);
            char *new_buff = (char *) realloc(m_stack, new_cap);

            if (new_buff == nullptr) {
                PLOGE("push_integer: 分配内存失败! 当前大小 %d, 扩容大小 %d\n", m_stack_capacity, new_cap);
                exit(1);
            }
            m_stack_capacity = new_cap;
            m_stack = new_buff;
        }
        *(uint64_t *) (m_stack + m_stack_size) = 0;
        *(T *) (m_stack + m_stack_size + padding) = val;
        m_stack_size += padding + sizeof(T);
    }

    /**
     * 将参数压进浮点寄存器
     */
    template<typename T>
    void push_float(jargs &args) noexcept
    {
        javsvm::jdouble val;
        *(T *) &val = args.next<T>();

        // 如果通用寄存器没有装满，使用通用寄存器
        if (m_float_register_size < m_float_registers.size()) {
            m_float_registers[m_float_register_size ++] = val;
            return;
        }
        PLOGE("push_float: 浮点数寄存器超出范围 !!\n");
        exit(1);
    }

    template<typename T>
    void inline push_integer(jargs &args) noexcept { push_integer(args.next<T>()); }

    int add_all(const char *sig, jargs &args) noexcept
    {
        for (int i = 1; sig[i] != ')'; i ++) {
            switch (sig[i]) {
                case 'Z':       /* boolean */
                    push_integer<javsvm::jboolean>(args);
                    break;
                case 'B':       /* byte */
                    push_integer<javsvm::jbyte>(args);
                    break;
                case 'C':       /* char */
                    push_integer<javsvm::jchar>(args);
                    break;
                case 'S':       /* short */
                    push_integer<javsvm::jshort>(args);
                    break;
                case 'I':       /* int */
                    push_integer<javsvm::jint>(args);
                    break;
                case 'J':       /* long */
                    push_integer<javsvm::jlong>(args);
                    break;
                case '[':       /* array */
                case 'L':       /* object */
                    push_integer<javsvm::jref>(args);
                    break;
                case 'F':       /* float */
                    push_float<javsvm::jfloat>(args);
                    break;
                case 'D':       /* double */
                    push_float<javsvm::jdouble>(args);
                    break;
                default:
                    LOGE("add_all: unknown jmethod sig: '%s'\n", sig);
                    return -1;
            }
        }
        return 0;
    }

    [[nodiscard]]
    const int64_t *registers() const noexcept { return m_registers.data(); }


    [[nodiscard]]
    const javsvm::jdouble *float_registers() const noexcept { return m_float_registers.data(); }


    [[nodiscard]]
    int float_size() const noexcept { return m_float_register_size; }

    [[nodiscard]]
    const void *stack() const noexcept { return m_stack; }

    [[nodiscard]]
    int stack_size() const noexcept { return align<16>(m_stack_size); }
};

template <bool B>
static void dump_stack_trace(jmethod *method, JNIEnv *jni_env, jref jni_obj,
                             int return_type, args_stack<B> &as)
{
    LOGD("run_jni:-----------------------dump stack-----------------------\n");
    LOGD("run_jni: %s->%s%s\n", method->clazz->name, method->name, method->sig);
    LOGD("run_jni: JNIEnv: %p, jenv: %p\n", jni_env, &jvm::get().env());
    LOGD("run_jni: obj: %s, %p\n", (method->access_flag & jclass_method::ACC_STATIC) ?
                                   "static" : "direct", jheap::cast(jni_obj));
    LOGD("run_jni: entrance: %p\n", method->entrance.jni_func);
    LOGD("run_jni: return_type: %d\n", return_type);
    LOGD("run_jni: \n");
    LOGD("run_jni: registers:\n");
    for (int i = 0; i < 8; i ++) {
        LOGD("run_jni: \t\t[%d/%d]: %lld\n", i, 8, as.registers()[i]);
    }
    LOGD("run_jni: \n");
    LOGD("run_jni: float_registers: %d\n", as.float_size());
    for (int i = 0, z = as.float_size(); i < z; i ++) {
        const void *buff = as.float_registers() + i;
        LOGD("run_jni: \t\t[%d/%d]: %f, %f\n", i, z,
             *(float *) buff, *(double *) buff
        );
    }
    LOGD("run_jni: \n");
    LOGD("run_jni: stack: %d\n", as.stack_size());
    for (int i = 0, z = as.stack_size(); i < z; i ++) {
        LOGD("run_jni: \t\t[%d/%d]: %llu\n", i, z, ((uint64_t *) as.stack())[i]);
    }
    LOGD("run_jni:---------------------dump stack end---------------------\n");
}

javsvm::jvalue javsvm::run_jni(jmethod *method, jref, jargs &args)
{
    LOGD("run_jni: start with %s->%s%s\n", method->clazz->name,
         method->name, method->sig);

    if (method->entrance.jni_func == nullptr) {
        // 如果当前函数还没有绑定，尝试去动态库中找
        LOGD("run_jni: entrance == nullptr, lookup ...\n");

        auto ptr = find_entrance(method);
        if (ptr == nullptr) {
            // todo 抛出一个链接异常
            LOGE("run_jni: lookup failed, abort\n");
            exit(1);
        }
        method->entrance.jni_func = ptr;
    }

    // 创建一个新的栈帧
    LOGD("run_jni: push a new frame\n");
    auto &env = jvm::get().env();
    auto &frame = env.stack.push(method);

    // 准备 jni 运行时参数
    args.reset();

    // 1. JNIEnv *, 直接使用 jenv 的 jni() api 即可
    auto jni_env = (JNIEnv *) env.jni();

    // 2. 根据是否是静态函数，确定传进 jobject 还是 jclass
    auto jni_obj = (method->access_flag & jclass_method::ACC_STATIC) ?
                   method->clazz->object :
                   args.next<jref>();


    // 3. 判断返回值类型
    int return_type = 0;
    switch (strchr(method->sig + 2, ')')[1]) {
        case 'F': return_type = 1; break;
        case 'D': return_type = 2; break;
        default: LOGE("run_jni: unknown return type: %s->%s\n", method->name, method->sig);
    }


    // 4. 参数
    // macOS 使用栈传递参数时不强制扩充到 8 字节
    args_stack<false> as;
    as.push_integer<::JNIEnv*>(jni_env);
    as.push_integer<::jobject>(to_object(jni_obj));
    as.add_all(method->sig, args);

    // 打印下参数
    dump_stack_trace(method, jni_env, jni_obj, return_type, as);

    // let's go, ka ku go, go go go ghost !
    javsvm::jvalue ret = calljni64(
            method->entrance.jni_func, return_type, as.registers(),
            as.float_registers(), as.stack_size(), as.stack()
            );

    // 弹出栈之前检查有没有异常。如果有，向上抛出
    auto exp = frame.exp;
    env.stack.pop();

    if (exp != nullptr) {
        throw_throwable(exp);
    }
    return ret;
}