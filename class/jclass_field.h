//
// Created by nlifew on 2020/10/16.
//

#ifndef JAVSVM_JCLASS_FIELD_H
#define JAVSVM_JCLASS_FIELD_H

namespace javsvm
{

struct jclass_field
{
    static constexpr u4 ACC_PUBLIC      = 0x0001;
    static constexpr u4 ACC_PRIVATE     = 0x0002;
    static constexpr u4 ACC_PROTECTED   = 0x0004;
    static constexpr u4 ACC_STATIC      = 0x0008;
    static constexpr u4 ACC_FINAL       = 0x0010;
    static constexpr u4 ACC_VOLATILE    = 0x0040;
    static constexpr u4 ACC_TRANSIENT   = 0x0080;
    static constexpr u4 ACC_SYNTHETIC   = 0x1000;
    static constexpr u4 ACC_ENUM        = 0x4000;

    u2 access_flag = 0;
    u2 name_index = 0;
    u2 descriptor_index = 0;
    u2 attributes_count = 0;
    jclass_attribute **attributes = nullptr;

    int read_from(input_stream& r, jclass_const_pool& pool)
    {
        r >> access_flag;
        r >> name_index;
        r >> descriptor_index;
        r >> attributes_count;
        attributes = new jclass_attribute*[attributes_count];
        for (int i = 0; i < attributes_count; ++i) {
            attributes[i] = jclass_attribute::read_from(r, pool);
        }
        return 0;
    }

    ~jclass_field()
    {
        for (int i = 0; i < attributes_count; ++i) {
            delete attributes[i];
        }
        delete[] attributes;
        attributes = nullptr;
        attributes_count = 0;
    }
};

};

#endif //JAVSVM_JCLASS_FIELD_H
