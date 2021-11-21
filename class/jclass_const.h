//
// Created by nlifew on 2020/10/15.
//

#ifndef JAVSVM_JCLASS_CONST_H
#define JAVSVM_JCLASS_CONST_H

namespace javsvm
{
struct jclass_const
{
    u1 tag = 0;
    void *extra = nullptr;

    jclass_const() = default;
    virtual ~jclass_const() = default;
};


class jclass_const_pool
{
private:
    static jclass_const* read_from(input_stream& r);

    jclass_const **m_pools = nullptr;
    int m_size = 0;
public:
    int read_from(input_stream& r, int length);

    [[nodiscard]]
    int size() const noexcept { return m_size; }

    [[nodiscard]]
    jclass_const* child_at(int index) const noexcept { return m_pools[index]; }

    jclass_const_pool() = default;

    ~jclass_const_pool();

    jclass_const_pool(const jclass_const_pool&) = delete;
    jclass_const_pool& operator=(const jclass_const_pool&) = delete;

    
    template <typename T>
    T *cast(int index)
    {
        jclass_const *p = child_at(index - 1);
        return T::TAG == p->tag ? (T *)p : nullptr;
    }
};


#define class_name utf8 
#define tag 1
#define class_field \
    set_short(length) \
    set_bytes(bytes, length)
#include "jclass_const_gen.h"



#define class_name int 
#define tag 3
#define class_field \
    set_int(bytes)
#include "jclass_const_gen.h"



#define class_name float
#define tag 4
#define class_field \
    set_int(bytes)
#include "jclass_const_gen.h"



#define class_name long
#define tag 5
#define class_field \
    set_long(bytes)
#include "jclass_const_gen.h"



#define class_name double
#define tag 6
#define class_field \
    set_long(bytes)
#include "jclass_const_gen.h"



#define class_name class
#define tag 7
#define class_field \
    set_short(index)
#include "jclass_const_gen.h"


#define class_name string
#define tag 8
#define class_field \
    set_short(index)
#include "jclass_const_gen.h"




#define class_name field_ref
#define tag 9
#define class_field \
    set_short(class_index) \
    set_short(name_and_type_index)
#include "jclass_const_gen.h"



#define class_name method_ref
#define tag 10
#define class_field \
    set_short(class_index) \
    set_short(name_and_type_index)
#include "jclass_const_gen.h"



#define class_name interface_ref
#define tag 11
#define class_field \
    set_short(class_index) \
    set_short(name_and_type_index)
#include "jclass_const_gen.h"




#define class_name name_and_type
#define tag 12
#define class_field \
    set_short(name_index) \
    set_short(type_index)
#include "jclass_const_gen.h"



#define class_name method_handle
#define tag 15
#define class_field \
    set_char(reference_kind) \
    set_short(reference_index)
#include "jclass_const_gen.h"



#define class_name method_type
#define tag 16
#define class_field \
    set_short(descriptor_index)
#include "jclass_const_gen.h"



#define class_name dynamic
#define tag 17
#define class_field \
    set_short(bootstrap_method_attr_index) \
    set_short(name_and_type_index)
#include "jclass_const_gen.h"



#define class_name invoke_dynamic
#define tag 18
#define class_field \
    set_short(bootstrap_method_attr_index) \
    set_short(name_and_type_index)
#include "jclass_const_gen.h"



#define class_name module
#define tag 19
#define class_field \
    set_short(name_index)
#include "jclass_const_gen.h"




#define class_name package
#define tag 20
#define class_field \
    set_short(name_index)
#include "jclass_const_gen.h"

};

#endif //JAVSVM_JCLASS_CONST_H
