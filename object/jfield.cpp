

#include "jfield.h"
#include "../object/jclass.h"
#include "../engine/engine.h"
#include "../vm/jvm.h"

using namespace javsvm;


static inline void calculate_field_size(jfield *field) noexcept
{
    switch (field->sig[0]) {
        case 'Z':       /* boolean */
            field->type = javsvm::jfield::BOOLEAN;
            field->mem_size = 1;
            break;
        case 'B':       /* byte */
            field->type = javsvm::jfield::BYTE;
            field->mem_size = 1;
            break;
        case 'C':       /* char */
            field->type = javsvm::jfield::CHAR;
            field->mem_size = 2;
            break;
        case 'S':       /* short */
            field->type = javsvm::jfield::SHORT;
            field->mem_size = 2;
            break;
        case 'I':       /* int */
            field->type = javsvm::jfield::INT;
            field->mem_size = 4;
            break;
        case 'J':       /* long */
            field->type = javsvm::jfield::LONG;
            field->mem_size = 8;
            break;
        case 'F':       /* float */
            field->type = javsvm::jfield::FLOAT;
            field->mem_size = 4;
            break;
        case 'D':       /* double */
            field->type = javsvm::jfield::DOUBLE;
            field->mem_size = 8;
            break;
        case 'L':       /* object */
            field->type = javsvm::jfield::OBJECT;
            field->mem_size = sizeof(slot_t);
            break;
        case '[':       /* array */
            field->type = javsvm::jfield::ARRAY;
            field->mem_size = sizeof(slot_t);
            break;
        default:
            LOGE("calculate_field_size: unknown field sig : %s\n", field->sig);
            break;
    }
}


void jfield::bind(jclass *_clazz, jclass_file *cls, int index)
{
    clazz = _clazz;
    orig = cls->fields + index;
    access_flag = orig->access_flag;
    name = (char *)cls->constant_pool.cast<jclass_const_utf8>(orig->name_index)->bytes;
    sig = (char *)cls->constant_pool.cast<jclass_const_utf8>(orig->descriptor_index)->bytes;

    calculate_field_size(this);
}




static inline jvalue get0(const void *src, int size) noexcept
{
    jvalue val;
    switch (size) {
        case 1: val.b = *(jbyte *) src; break;
        case 2: val.s = *(jshort *) src ;break;
        case 4: val.i = *(jint *) src; break;
        case 8: val.j = *(jlong *) src; break;
        default: val.j = 0; break;
    }
    return val;
}

jvalue jfield::get(jref obj) const noexcept
{
    if ((access_flag & jclass_field::ACC_STATIC) != 0) {
        // 静态函数，访问类内存
        // 先检查这个类有没有被初始化
        if (clazz->invoke_clinit() < 0) {
            return { 0 };
        }
        return get0((char*) clazz->data + mem_offset, mem_size);
    }

    // 成员函数
    auto ptr = jvm::get().heap.lock(obj);
    if (ptr == nullptr) {
        throw_exp("java/lang/NullPointerException", name);
        return { 0 };
    }

    return get0(ptr->values + mem_offset, mem_size);
}

static inline void set0(void *dst, jvalue val, int size) noexcept
{
    switch (size) {
        case 1: *(jbyte *) dst = val.b; break;
        case 2: *(jshort *) dst = val.s; break;
        case 4: *(jint *) dst = val.i; break;
        case 8: *(jlong *) dst = val.j; break;
        default:
            break;
    }
}

void jfield::set(jref obj, jvalue val) const noexcept
{
    if ((access_flag & jclass_field::ACC_STATIC) != 0) {
        // 静态函数，访问类内存
        // 先检查这个类有没有被初始化
        if (clazz->invoke_clinit() < 0) {
            return;
        }
        set0((char*) clazz->data + mem_offset, val, mem_size);
        return;
    }

    // 成员函数
    auto ptr = jvm::get().heap.lock(obj);
    if (ptr == nullptr) {
        throw_exp("java/lang/NullPointerException", name);
        return;
    }

    set0(ptr->values + mem_offset, val, mem_size);
}