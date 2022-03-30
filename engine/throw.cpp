

#include "engine.h"
#include "../object/jmethod.h"
#include "../object/jclass.h"
#include "../class/jclass_file.h"
#include "../vm/jvm.h"

using namespace javsvm;


#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

void javsvm::throw_exp(const char *class_name, const char *msg)
{
    // 异常类一定使用初始类加载器
    auto klass = jvm::get().bootstrap_loader.load_class(class_name);
    if (klass == nullptr) {
        PLOGE("throw_exp: failed to load class '%s', abort\n", class_name);
        throw_err(class_name, msg);
        return;
    }
    auto _init_ = klass->get_method("<init>", "(Ljava/lang/String;)V");
    auto exp = klass->new_instance(_init_, jvm::get().string.find_or_new(msg));
    auto exp_ptr = jheap::cast(exp);
    if (exp_ptr == nullptr) {
        LOGE("failed to create exception instance with class '%s' and msg '%s'\n", class_name, msg);
        throw_err(class_name, msg);
        return;
    }
    throw_throwable(exp);
}


void javsvm::throw_err(const char *class_name, const void *msg)
{
    // todo
    exit(1);
}

/**
 * 查询异常表。如果成功，返回对应的异常表结构体，否则返回 nullptr
 */
static jclass_attr_code::exception_table *
lookup_exp_table(u4 pc, jref ref, jmethod *method)
{
    const auto code = method->entrance.code_func;
    const auto &constant_pool = method->clazz->class_file->constant_pool;

    for (int i = 0; i < code->exception_table_count; ++i) {
        auto &etb = code->exception_tables[i];
        if (pc < etb.start_pc || pc > etb.end_pc) continue;

        // 如果 catch_type 为 0，表示捕获任何形式的异常
        if (etb.catch_type == 0) {
            return &etb;
        }

        // 尝试加载 catch_type 指向的类
        auto catch_type = constant_pool.cast<jclass_const_class>(etb.catch_type);
        assert(catch_type);

        if (catch_type->extra == nullptr) {
            auto *class_name = constant_pool.cast<jclass_const_utf8>(catch_type->index);
            catch_type->extra = jclass::load_class((char *) class_name->bytes);
            if (catch_type->extra == nullptr) {
                LOGE("can't find class [%s]\n", class_name->bytes);
                exit(1);
            }
        }
        if (((jclass*) catch_type->extra)->is_instance(ref)) {
            return &etb;
        }
    }
    return nullptr;
}

void javsvm::throw_throwable(jref ref) noexcept
{
    auto ptr = jheap::cast(ref);
    if (ptr == nullptr) {
        throw_exp("java/lang/NullPointerException", "throw_throwable");
        return;
    }
    // 栈回溯
    const auto &stack = jvm::get().env().stack;

    for (jstack_frame *frame = stack.top(); frame; frame = frame->next) {
        const auto &method = frame->method;

        // 先判断一下是不是 native 函数
        auto access_flag = method->access_flag;
        if ((access_flag & jclass_method::ACC_NATIVE) != 0) {
            // 当前正在执行 native 函数，只需要把异常放进栈帧中暂存一下即可 (jni随时会清除异常)
            // native 函数执行完，栈帧弹出后，引擎会重新抛出异常
            frame->exp = ref;
            return;
        }
        // 不是 native 函数，需要查询异常表
        auto etb = lookup_exp_table(frame->pc, ref, method);
        if (etb != nullptr) {
            // 查询到了对应的结构体，调整 pc 指针，引用压栈
            frame->exp_handler_pc = etb->handler_pc;
            frame->pc = method->entrance.code_func->code_length;
            frame->exp = ref;
            return;
        }
        // 没有查询到对应结构体，调整 pc，函数终止执行
        frame->pc = method->entrance.code_func->code_length;
    }
    // 即使经过栈回溯，也没有找到能捕获异常的函数
    // 此时需要调用 java.lang.Thread 的默认异常处理函数
    // todo:
}

#pragma clang diagnostic pop

jref javsvm::check_exception() noexcept
{
    // 栈回溯
    const auto &stack = jvm::get().env().stack;

    for (jstack_frame *frame = stack.top(); frame; frame = frame->next) {

        // 如果是 native 栈，可以快速返回
        if ((frame->method->access_flag & jclass_method::ACC_NATIVE) != 0) {
            return frame->exp;
        }

        if (frame->exp != nullptr) {
            return frame->exp;
        }
    }
    return nullptr;
}

void javsvm::clear_exception() noexcept
{
    auto frame = jvm::get().env().stack.top();
    if (frame == nullptr) {
        return;
    }
    auto method = frame->method;
    if ((method->access_flag & jclass_method::ACC_NATIVE) != 0) {
        frame->exp = nullptr;
        return;
    }
    LOGE("clear_exception: 禁止在 java 层中清除异常");
    exit(1);
}