//
// Created by nlifew on 2020/10/15.
//

#include "jclass_file.h"

using namespace javsvm;


int jclass_file::read_from(input_stream& in)
{
    if (header.read_from(in) < 0) {
        LOGE("failed to read jclass_header\n");
        return -1;
    }

    auto constant_pool_size = in.read<u2>();
    if (constant_pool.read_from(in, constant_pool_size - 1) < 0) {
        LOGE("failed to read jclass_constant_pool");
        return -1;
    }

//    LOGI("where: %#lx\n", in.where());

    in >> access_flag;
    in >> this_class;
    in >> super_class;

    in >> interface_count;
    interfaces = new jclass_interface[interface_count];
    for (int i = 0; i < interface_count; ++i) {
        interfaces[i].read_from(in);
    }

    in >> field_count;
    fields = new jclass_field[field_count];
    for (int i = 0; i < field_count; ++i)  {
        fields[i].read_from(in, constant_pool);
    }

    in >> method_count;
    methods = new jclass_method[method_count];

    for (int i = 0; i < method_count; ++i) {
//        LOGI("test method [%d]\n", i);
        methods[i].read_from(in, constant_pool);
    }

    in >> attribute_count;
    attributes = new jclass_attribute*[attribute_count];
    for (int i = 0; i < attribute_count; ++i) {
        attributes[i] = jclass_attribute::read_from(in, constant_pool);
    }
//     LOGI("%#lx\n", in.where());

    return 0;
}



jclass_file::~jclass_file() noexcept
{
    delete[] interfaces;
    interfaces = nullptr;
    interface_count = 0;

    delete[] fields;
    fields = nullptr;
    field_count = 0;

    delete[] methods;
    methods = nullptr;
    method_count = 0;

    for (int i = 0; i < attribute_count; ++i) {
        delete attributes[i];
    }
    delete[] attributes;
    attributes = nullptr;
    attribute_count = 0;
}
