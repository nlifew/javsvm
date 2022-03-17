

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