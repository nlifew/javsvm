//
// Created by nlifew on 2020/10/16.
//

#ifndef JAVSVM_JCLASS_METHOD_H
#define JAVSVM_JCLASS_METHOD_H

namespace javsvm
{

struct jclass_method
{
    static constexpr u4 ACC_PUBLIC          = 0x0001;
    static constexpr u4 ACC_PRIVATE         = 0x0002;
    static constexpr u4 ACC_PROTECTED       = 0x0004;
    static constexpr u4 ACC_STATIC          = 0x0008;
    static constexpr u4 ACC_FINAL           = 0x0010;
    static constexpr u4 ACC_SYNCHRONIZED    = 0x0020;
    static constexpr u4 ACC_BRIDGE          = 0x0040;
    static constexpr u4 ACC_VARARGS         = 0x0080;
    static constexpr u4 ACC_NATIVE          = 0x0100;
    static constexpr u4 ACC_ABSTRACT        = 0x0400;
    static constexpr u4 ACC_STRICT          = 0x0800;
    static constexpr u4 ACC_SYNTHETIC       = 0x1000;


    u2 access_flag = 0;
    u2 name_index = 0;
    u2 descriptor_index = 0;
    u2 attributes_count = 0;
    jclass_attribute **attributes = nullptr;

    int read_from(input_stream& in, jclass_const_pool& pool)
    {
        // LOGI("where() = %#x\n", in.where());
        in >> access_flag;
        in >> name_index;
        in >> descriptor_index;
        in >> attributes_count;

//        auto utf8 = pool.cast<jclass_const_utf8>(name_index);
//        LOGI("reading method [%s]\n", utf8->bytes);

        attributes = new jclass_attribute*[attributes_count];
        for (int i = 0; i < attributes_count; ++i) {
//            LOGI("[%d], where = [%#lx]\n", i, in.where());

            attributes[i] = jclass_attribute::read_from(in, pool);
            // LOGI("[%d/%d] [%#x] [%d]\n", i + 1, attributes_count,
            //     in.where(), attributes[i] == nullptr ? 
            //     -1 : attributes[i]->attribute_name_index);
        }
        return 0;
    }
    
    ~jclass_method()
    {
        for (int i = 0; i < attributes_count; ++i) {
            delete attributes[i];
        }
        delete[] attributes;
        attributes = nullptr;
        attributes_count = 0;
    }
};
}

#endif //JAVSVM_JCLASS_METHOD_H
