//
// Created by nlifew on 2020/10/15.
//

#ifndef JAVSVM_JCLASS_FILE_H
#define JAVSVM_JCLASS_FILE_H

#include "../utils/global.h"
#include "../io/input_stream.h"

#include "jclass_header.h"
#include "jclass_const.h"
#include "jclass_attribute.h"
#include "jclass_interface.h"
#include "jclass_field.h"
#include "jclass_method.h"

#include <vector>

namespace javsvm
{

struct jclass_file
{
    static constexpr u4 ACC_PUBLIC      = 0x0001;
    static constexpr u4 ACC_FINAL       = 0x0010;
    static constexpr u4 ACC_SUPER       = 0x0020;
    static constexpr u4 ACC_INTERFACE   = 0x0200;
    static constexpr u4 ACC_ABSTRACT    = 0x0400;
    static constexpr u4 ACC_SYNTHETIC   = 0x1000;
    static constexpr u4 ACC_ANNOTATION  = 0x2000;
    static constexpr u4 ACC_ENUM        = 0x4000;
    static constexpr u4 ACC_MODULE      = 0x8000;

    /**
     * 下面的 access flag 是 .class 文件未使用，但我们在运行时需要的
     */
     static constexpr u4 ACC_FINALIZE = 0x0002;


    jclass_header header;
    jclass_const_pool constant_pool;

    u2 access_flag = 0;
    u2 this_class = 0;
    u2 super_class = 0;

    u2 interface_count = 0;
    jclass_interface *interfaces = nullptr;

    u2 field_count = 0;

    jclass_field *fields = nullptr;

    u2 method_count = 0;
    jclass_method *methods = nullptr;

    u2 attribute_count = 0;
    jclass_attribute **attributes = nullptr;

    jclass_file() = default;

    jclass_file(const jclass_file&) = delete;
    jclass_file& operator=(const jclass_file&) = delete;

    ~jclass_file() noexcept;

    int read_from(input_stream& r);
};


}
#endif //JAVSVM_JCLASS_FILE_H
