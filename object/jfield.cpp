

#include "jfield.h"
#include "../object/jclass.h"

using namespace javsvm;


/*
jvalue jfield::get(jref obj)
{
    
}

void jfield::set(jref obj, jvalue val)
{
    
}
*/
static void calculate_field_size(const char *sig, int *p_mem_size, int *p_slot_num)
{
    int slot_num = 0;
    int mem_size = 0;

    switch (sig[0]) {
        case 'Z':       /* boolean */
        case 'B':       /* byte */
            mem_size = 1;
            slot_num = 1;
            break;
        case 'C':       /* char */
        case 'S':       /* short */
            mem_size = 2;
            slot_num = 1;
            break;
        case 'I':       /* int */
        case 'F':       /* float */
            mem_size = 4;
            slot_num = 1;
            break;
        case 'D':       /* double */
        case 'J':       /* long */
            mem_size = 8;
            slot_num = 2;
            break;
        case '[':       /* array */
        case 'L':       /* object */
            mem_size = 8;
            slot_num = 1;
        default:
            LOGE("unknown field sig : %s\n", sig);
    }
    *p_slot_num = slot_num;
    *p_mem_size = mem_size;
}



void jfield::bind(jclass *clazz, jclass_file *cls, int index)
{
    klass = clazz;
    orig = cls->fields + index;
    access_flag = orig->access_flag;
    name = (char *)cls->constant_pool.cast<jclass_const_utf8>(orig->name_index)->bytes;
    sig = (char *)cls->constant_pool.cast<jclass_const_utf8>(orig->descriptor_index)->bytes;

    calculate_field_size(sig, &mem_size, &slot_num);
}
