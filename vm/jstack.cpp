

#include <new>
#include <cassert>

#include "jstack.h"
#include "jheap.h"
#include "../object/jobject.h"
#include "../object/jmethod.h"
#include "../utils/arrays.h"

using namespace javsvm;

void* jstack::malloc_bytes(int bytes)
{
//    if (bytes == 0) {
//        return nullptr;
//    }
    bytes = align<16>(bytes);
    if (m_offset + bytes > m_capacity) {
        // 别问，问就是 stack over flow
        PLOGE("stack over flow [%lu]/[%zu] bytes\n", m_offset + bytes, m_capacity);
        exit(1);
    }

    void *mem = m_buff + m_offset;
    m_offset += bytes;

    return mem; // memset(mem, 0, bytes);
}

//void jstack::recycle_bytes(int bytes)
//{
//    m_offset -= bytes;
//}


jstack::jstack(size_t capacity) noexcept
{
    if (capacity <= 0) {
        capacity = DEFAULT_STACK_SIZE;
    }
    m_capacity = capacity;
    m_buff = new char[capacity];
}


jstack::~jstack() noexcept
{
    delete[] m_buff;
    m_buff = nullptr;
}


jstack_frame *jstack::pop() noexcept
{
    assert(m_top != nullptr);

    m_offset = m_top->backup;
    m_top = m_top->next;

    return m_top;
}

jstack_frame& jstack::push(jmethod *m) noexcept
{
    int pos = m_offset;

    jstack_frame *result;

    if ((m->access_flag & jclass_method::ACC_NATIVE) == 0) {
        auto frame = alloc<java_stack_frame>();
        result = frame;

        jclass_attr_code *code = m->entrance.code_func;

        frame->operand_stack = alloc<slot_t>(code->max_stack);
        frame->variable_table = alloc<slot_t>(code->max_locals);

        frame->operand_ref_stack = alloc<u1>(code->max_stack);
        frame->variable_ref_table = alloc<u1>(code->max_locals);

        frame->operand_stack_orig = frame->operand_stack;
        frame->operand_ref_stack_orig = frame->operand_ref_stack;
    }
    else {
        auto frame = alloc<jni_stack_frame>();
        result = frame;

        frame->stack = this;
        frame->local_ref_table = alloc<jref>(frame->local_ref_table_capacity);
    }

    result->backup = pos;
    result->method = m;
    result->next = m_top;
    m_top = result;
    return *result;
}

void jstack_frame::lock_if(jref lck) noexcept
{
    if ((method->access_flag & jclass_method::ACC_SYNCHRONIZED) == 0) {
        return;
    }

    lock = lck;
    auto *ptr = jheap::cast(lck);
    assert(ptr != nullptr);
    auto ok = ptr->lock();
    assert(ok == 0);
}

void jstack_frame::unlock() noexcept
{
    if (lock == nullptr) {
        return;
    }
    auto ok = jheap::cast(lock)->unlock();
    assert(ok == 0);
    lock = nullptr;
}


int jni_stack_frame::reserve(int capacity) noexcept
{
    if (local_ref_table_capacity >= capacity) {
        return 0;
    }
    assert(stack->top() == this);
    int need = (capacity - local_ref_table_capacity) * (int) sizeof(jref);
    if (stack->m_offset + need > stack->m_capacity) {
        return -1;
    }
    local_ref_table_capacity = capacity;
    stack->m_offset += need;
    return 0;
}



std::vector<stack_trace> jstack::dump() const noexcept
{
    std::vector<stack_trace> v;
    for (auto f = m_top; f; f = f->next) {
        stack_trace st = {
                .source = "",
                .klass = f->method->clazz->name,
                .method = f->method->name,
                .line_number = 0,
                .pc = (int) f->pc,
        };
        {
            auto class_file = f->method->clazz->class_file;
            for (int i = 0, z = class_file->attribute_count; i < z; ++i) {
                auto attr = class_file->attributes[i]->cast<jclass_attr_source_file>();
                if (attr != nullptr) {
                    auto index = attr->source_file_index;
                    auto &constant_pool = class_file->constant_pool;
                    st.source = (char *) constant_pool.cast<jclass_const_utf8>(index)->bytes;
                    break;
                }
            }
        }

        if ((f->method->access_flag & jclass_method::ACC_NATIVE) == 0) {
            jclass_attr_line_number *ln = nullptr;
            auto code = f->method->entrance.code_func;
            for (int i = 0, z = code->attribute_count; i < z; ++i) {
                ln = code->attributes[i]->cast<jclass_attr_line_number>();
                if (ln != nullptr) {
                    break;
                }
            }
            if (ln != nullptr) {
                // 使用二分查找加速效率
                // NOTE: javsvm 中的 pc 是 u4 类型，但 lineNumber 和 Exception 中的 pc
                // 都是 u2 类型. 对于 javac，当 pc 超过 65536 时会拒绝编译
                // 我们这里直接强转
                u2 key = st.pc & 0xFFFF;
                int index = arrays::bsearch(&key,
                                            ln->line_number_table,
                                            ln->line_number_table_length,
                                            sizeof(jclass_attr_line_number::line_number_info),
                                            [](const void *p, const void *q) -> int {
                    auto pc = *(u2 *) p;
                    auto &lni = *(jclass_attr_line_number::line_number_info *) q;
                    return pc - lni.start_pc;
                });
                if (index < 0) index = - index - 2;
                assert(index >= 0 && index < ln->line_number_table_length);
                st.line_number = ln->line_number_table[index].line_number;
            }
        }
        v.push_back(st);
    }
    return v;
}