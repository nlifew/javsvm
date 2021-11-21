

#include <new>
#include "jstack.h"
#include "../object/jmethod.h"

using namespace javsvm;

void* jstack::malloc_bytes(int bytes)
{
    if (bytes == 0) {
        return nullptr;
    }
    if (m_offset + bytes > m_capacity) {
        // 别问，问就是 stack over flow
        PLOGE("stack over flow [%d]/[%d] bytes\n", m_offset + bytes, m_capacity);
        exit(1);
    }

    void *mem = m_buff + m_offset;
    m_offset += bytes;
    return mem;
}

void jstack::recycle_bytes(int bytes)
 { 
    m_offset -= bytes;
}


jstack::jstack(int capacity)
{
    m_capacity = capacity;
    m_buff = new char[capacity];
}


jstack::~jstack()
{
    delete[] m_buff;
    m_buff = nullptr;
}


void jstack::pop()
{
    if (m_top == nullptr) {
        return;
    }
    jstack_frame *frame = m_top;
    m_top = frame->next;
    frame->next = nullptr;
    // frame->in_use = false;

    recycle_bytes(frame->bytes);
}

jstack_frame& jstack::push(jmethod *m)
{
    int pos = m_offset;
    auto frame = calloc_type<jstack_frame>();

    jclass_attr_code *code = m->entrance.code_func;

    frame->method = m;
    frame->operand_stack = calloc_type<slot_t>(code->max_stack);
    frame->variable_table = calloc_type<slot_t>(code->max_locals);
    frame->bytes = m_offset - pos;

    frame->next = m_top;
    m_top = frame;
    return *frame;
}
