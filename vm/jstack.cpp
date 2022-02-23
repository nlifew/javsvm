

#include <new>
#include "jstack.h"
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
        PLOGE("stack over flow [%d]/[%d] bytes\n", m_offset + bytes, m_capacity);
        exit(1);
    }

    void *mem = m_buff + m_offset;
    m_offset += bytes;

    return mem; // memset(mem, 0, bytes);
}

void jstack::recycle_bytes(int bytes)
{
    m_offset -= bytes;
}


jstack::jstack(int capacity) noexcept
{
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

    jstack_frame *frame = m_top;
    m_top = frame->next;
    frame->next = nullptr;

    recycle_bytes(frame->bytes);
    return m_top;
}

jstack_frame& jstack::push(jmethod *m) noexcept
{
    int pos = m_offset;
    auto frame = alloc<jstack_frame>();


    if ((m->access_flag & jclass_method::ACC_NATIVE) == 0) {
        jclass_attr_code *code = m->entrance.code_func;

        frame->operand_stack = alloc<slot_t>(code->max_stack);
        frame->variable_table = alloc<slot_t>(code->max_locals);

        frame->operand_ref_stack = alloc<u1>(code->max_stack);
        frame->variable_ref_table = alloc<u1>(code->max_locals);

        frame->operand_stack_orig = frame->operand_stack;
        frame->operand_ref_stack_orig = frame->operand_ref_stack;
    }

    frame->method = m;
    frame->bytes = m_offset - pos;

    frame->next = m_top;
    m_top = frame;
    return *frame;
}
