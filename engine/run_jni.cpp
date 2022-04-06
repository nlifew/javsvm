
/**
 * native 函数执行逻辑
 */

#include "engine.h"
#include "jnilib.h"
#include "../vm/jvm.h"
#include "../jni/jni.h"
#include "../gc/safety_point.h"
#include "../jni/jni_utils.h"

#include <array>
#include <cstdlib>

using namespace javsvm;


static void *find_entrance(jmethod *method)
{
    auto &loader = jvm::get().dll_loader;

    auto short_name = jni_short_name(method->clazz->name, method->name);
    LOGI("find_entrance: try to use short name '%s'\n", short_name.c_str());

    auto ptr = loader.find_symbol(short_name.c_str());
    if (ptr != nullptr) {
        return ptr;
    }
    auto long_name = jni_long_name(method->clazz->name, method->name, method->sig);
    LOGI("find_entrance: try to use short name '%s'\n", long_name.c_str());

    return loader.find_symbol(long_name.c_str());
}


static_assert(sizeof(javsvm::jvalue) == sizeof(int64_t));

extern "C" int64_t calljni64(
        const void *addr,			/* 函数地址，由 load_library() 获取得到，不能是 nullptr */
        int return_type,			/* 返回类型，0 表示返回 int64，非 0 返回 double64.  */
        const int64_t* integers,	/* 固定长度为 8 的数组，数组内的值将被写入到 x0-x7 通用寄存器 */
        const double* floats,		/* 固定长度为 16 的数组，数组内的值将被写入到 d0-d15 浮点数寄存器 */
        int stack_len,				/* 表示要压进栈的参数大小，单位是字节，为非负的整数。需要调用者自己对齐到 16 字节 */
        const void *stack			/* 要压进栈的参数的起始地址。高地址会被先压进栈。 */
);

/**
 * @tparam align8 使用栈传递参数时是否强制 8 字节对齐
 * @tparam intRegNum 通用寄存器的数量
 * @tparam floatRegNum 浮点数寄存器数量
 */
template <bool align8, int intRegNum, int floatRegNum>
class args_stack
{
    std::array<int64_t, intRegNum> m_registers{};
    int m_register_size = 0;

    std::array<javsvm::jdouble, floatRegNum> m_float_registers{};
    int m_float_register_size = 0;

    char *m_stack = nullptr;
    int m_stack_size = 0;
    int m_stack_capacity = 0; // 16 的整数倍

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
            padding = align<sizeof(T)>(m_stack_size) - m_stack_size;
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
    void push_float(T t) noexcept
    {
        javsvm::jdouble val;
        *(T *) &val = t;

        // 如果通用寄存器没有装满，使用通用寄存器
        if (m_float_register_size < m_float_registers.size()) {
            m_float_registers[m_float_register_size ++] = val;
            return;
        }
        PLOGE("push_float: 浮点数寄存器超出范围 !!\n");
        exit(1);
    }


    int add_all(const char *sig, jargs &args) noexcept
    {
        for (int i = 1; sig[i] != ')'; i ++) {
            switch (sig[i]) {
                case 'Z':       /* boolean */
                    push_integer(args.next<javsvm::jboolean>());
                    break;
                case 'B':       /* byte */
                    push_integer(args.next<javsvm::jbyte>());
                    break;
                case 'C':       /* char */
                    push_integer(args.next<javsvm::jchar>());
                    break;
                case 'S':       /* short */
                    push_integer(args.next<javsvm::jshort>());
                    break;
                case 'I':       /* int */
                    push_integer(args.next<javsvm::jint>());
                    break;
                case 'J':       /* long */
                    push_integer(args.next<javsvm::jlong>());
                    break;
                case '[':       /* array */
                case 'L':       /* object */
                    push_integer(args.next<javsvm::jref>());
                    break;
                case 'F':       /* float */
                    push_float(args.next<javsvm::jfloat>());
                    break;
                case 'D':       /* double */
                    push_float(args.next<javsvm::jdouble>());
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
    int register_size() const noexcept { return m_register_size; }

    [[nodiscard]]
    const javsvm::jdouble *float_registers() const noexcept { return m_float_registers.data(); }


    [[nodiscard]]
    int float_size() const noexcept { return m_float_register_size; }

    [[nodiscard]]
    const void *stack() const noexcept { return m_stack; }

    [[nodiscard]]
    int stack_size() const noexcept { return align<16>(m_stack_size); }
};


template <bool align8, int regNum, int floatRegNum>
static void dump_stack_trace(jmethod *method, JNIEnv *jenv, jref jni_obj,
                             int return_type, args_stack<align8, regNum, floatRegNum> &as)
{
    LOGD("run_jni:-----------------------dump stack-----------------------\n");
    LOGD("run_jni: %s->%s%s\n", method->clazz->name, method->name, method->sig);
    LOGD("run_jni: JNIEnv: %p\n", jenv);
    LOGD("run_jni: obj: %s, %p\n", (method->access_flag & jclass_method::ACC_STATIC) ?
                                   "static" : "direct", jheap::cast(jni_obj));
    LOGD("run_jni: entrance: %p\n", method->entrance.jni_func);
    LOGD("run_jni: return_type: %d\n", return_type);
    LOGD("run_jni: \n");
    LOGD("run_jni: registers:\n");
    for (int i = 0, z = as.register_size(); i < z; i ++) {
        LOGD("run_jni: \t\t[%d/%d]: %lld\n", i, z, as.registers()[i]);
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
        LOGD("run_jni: \t\t[%d/%d]: %#x\n", i, z, 0xff & ((char *) as.stack())[i]);
    }
    LOGD("run_jni:---------------------dump stack end---------------------\n");
}


using macos_arm64_args_t = args_stack<false, 8, 16>;


javsvm::jvalue javsvm::run_jni(jmethod *method, jref lock_object, jargs &args)
{
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

    frame.lock_if(lock_object);

    // 准备 jni 运行时参数
    args.reset();

    // 1. JNIEnv *, 直接使用全局变量即可
    auto jenv = jni::jni_env;

    // 2. 根据是否是静态函数，确定传进 jobject 还是 jclass
    auto jni_obj = (method->access_flag & jclass_method::ACC_STATIC) ?
                   method->clazz->object.get() :
                   args.next<jref>();


    // 3. 判断返回值类型
    int return_type = 0;
    switch (strchr(method->sig + 2, ')')[1]) {
        case 'F': return_type = 1; break;
        case 'D': return_type = 2; break;
        default: break;
    }

    // 4. 参数
    macos_arm64_args_t as;
    as.push_integer<::JNIEnv*>(jenv);
    as.push_integer<::jobject>(to_object(jni_obj));
    as.add_all(method->sig, args);

#ifndef NDEBUG
    // 打印下参数
    dump_stack_trace(method, jenv, jni_obj, return_type, as);
#endif

    // 进入安全区
    enter_safety_area();

    // let's go, ka ku go, go go go ghost !
    int64_t ret_val = calljni64(
            method->entrance.jni_func, return_type, as.registers(),
            as.float_registers(), as.stack_size(), as.stack());

    // 离开安全区
    leave_safety_area();
    
    // 处理返回值
    javsvm::jvalue v;

    switch (return_type) {
        case 0: v.j = ret_val; break;
        case 1: v.f = *(javsvm::jfloat *) &ret_val; break;
        case 2: v.d = *(javsvm::jdouble *) &ret_val; break;
        default: break;
    }

    // 弹出栈之前检查有没有异常。如果有，向上抛出
    auto exp = frame.exp;

    frame.unlock();
    env.stack.pop();

    if (exp != nullptr) {
        throw_throwable(exp);
    }

    return v;
}