

#include <new>
#include "jstack.h"
#include "jheap.h"
#include "../object/jobject.h"
#include "../object/jmethod.h"

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