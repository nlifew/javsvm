//
// Created by nlifew on 2020/10/16.
//

#ifndef JAVSVM_JCLASS_ATTRIBUTE_H
#define JAVSVM_JCLASS_ATTRIBUTE_H

#include "jclass_const.h"

namespace javsvm
{

struct jclass_attribute
{
    jclass_const_pool *pool = nullptr;
    u2 attribute_name_index = 0;
    u4 attribute_length = 0;

    virtual ~jclass_attribute() = default;

    static jclass_attribute* read_from(input_stream&, jclass_const_pool&);

    template <typename T>
    inline T* cast() const noexcept
    {
        auto utf8 = pool->cast<jclass_const_utf8>(attribute_name_index);
        return utf8 && strcmp((char *)utf8->bytes, T::NAME) == 0 ? (T *)this : nullptr;
    }
};


#define class_name unknown
#define name unknown
#define class_field \
    set_bytes(buff, attribute_length)
#include "jclass_attribute_gen.h"


#define class_name code
#define name Code
#define class_field \
    set_short(max_stack) \
    set_short(max_locals) \
    set_int(code_length) \
    set_bytes(code, code_length) \
    set_short(exception_table_count) \
    set_array(exception_table, exception_tables, exception_table_count)

#define class_body \
    struct exception_table \
    { \
        u2 start_pc; \
        u2 end_pc; \
        u2 handler_pc; \
        u2 catch_type; \
\
        void read_from(input_stream& in) \
        { \
            in >> start_pc; \
            in >> end_pc; \
            in >> handler_pc; \
            in >> catch_type; \
        } \
    }; \
    u2 attribute_count = 0; \
    jclass_attribute **attributes = nullptr;

#define constructor \
    r >> attribute_count; \
    attributes = new jclass_attribute*[attribute_count]; \
    for (int i = 0; i < attribute_count; ++i) { \
        attributes[i] = jclass_attribute::read_from(r, _pool); \
    }

#define destructor \
    for (int i = 0; i < attribute_count; ++i) { \
        delete attributes[i]; \
    } \
    delete[] attributes;

#include "jclass_attribute_gen.h"



#define class_name exception 
#define name Exceptions

#define class_body \
    u2 number_of_exceptions = 0; \
    u2 *exception_index_table = nullptr;
#define constructor \
    r >> number_of_exceptions; \
    exception_index_table = new u2[number_of_exceptions]; \
    for (int i = 0; i < number_of_exceptions; ++i) { \
        r >> (exception_index_table[i]); \
    }
#define destructor \
    delete[] exception_index_table;

#include "jclass_attribute_gen.h"



#define class_name line_number
#define name LineNumberTable
#define class_field \
    set_short(line_number_table_length) \
    set_array(line_number_info, line_number_table, line_number_table_length) \

#define class_body \
    struct line_number_info \
    { \
        u2 start_pc = 0; \
        u2 line_number = 0; \
\
        void read_from(input_stream& r) \
        { \
            r >> start_pc; \
            r >> line_number; \
        } \
    }; \

#include "jclass_attribute_gen.h"



#define class_name local_variable
#define name LocalVariableTable
#define class_field \
    set_short(local_variable_table_length) \
    set_array(local_variable_info, local_variable_table, local_variable_table_length)

#define class_body \
    struct local_variable_info \
    { \
        u2 start_pc = 0; \
        u2 length = 0; \
        u2 name_index = 0; \
        u2 descriptor_index = 0; \
        u2 index = 0; \
\
        void read_from(input_stream& r) \
        { \
            r >> start_pc; \
            r >> length; \
            r >> name_index; \
            r >> descriptor_index; \
            r >> index; \
        } \
    };

#include "jclass_attribute_gen.h"



#define class_name local_variable_type
#define name LocalVariableTypeTable
#define class_field \
    set_short(local_variable_table_length) \
    set_array(local_variable_info, local_variable_table, local_variable_table_length)

#define class_body \
    struct local_variable_info \
    { \
        u2 start_pc = 0; \
        u2 length = 0; \
        u2 name_index = 0; \
        u2 signature_index = 0; \
        u2 index = 0; \
\
        void read_from(input_stream& r) \
        { \
            r >> start_pc; \
            r >> length; \
            r >> name_index; \
            r >> signature_index; \
            r >> index; \
        } \
    }; 

#include "jclass_attribute_gen.h"



#define class_name source_file
#define name SourceFile
#define class_field \
    set_short(source_file_index)

#include "jclass_attribute_gen.h"



#define class_name source_debug
#define name SourceDebugExtension
#define class_field \
    set_bytes(debug_extension, attribute_length)

#include "jclass_attribute_gen.h"



#define class_name constant
#define name ConstantValue
#define class_field \
    set_short(constant_value_index)

#include "jclass_attribute_gen.h"



#define class_name inner_class
#define name InnerClasses
#define class_field \
    set_short(number_of_classes) \
    set_array(inner_class_info, inner_classes, number_of_classes)

#define class_body \
    struct inner_class_info \
    { \
        static constexpr u4 ACC_PUBLIC      = 0x0001; \
        static constexpr u4 ACC_PRIVATE     = 0x0002; \
        static constexpr u4 ACC_PROTECTED   = 0x0004; \
        static constexpr u4 ACC_STATIC      = 0x0008; \
        static constexpr u4 ACC_FINAL       = 0x0010; \
        static constexpr u4 ACC_INTERFACE   = 0x0020; \
        static constexpr u4 ACC_ABSTRACT    = 0x0400; \
        static constexpr u4 ACC_SYNTHETIC   = 0x1000; \
        static constexpr u4 ACC_ANNOTATION  = 0x2000; \
        static constexpr u4 ACC_ENUM        = 0x4000; \
\
        u2 inner_class_info_index = 0; \
        u2 outer_class_info_index = 0; \
        u2 inner_name_index = 0; \
        u2 inner_class_access_flag = 0; \
\
        void read_from(input_stream& r) \
        { \
            r >> inner_class_info_index; \
            r >> outer_class_info_index; \
            r >> inner_name_index; \
            r >> inner_class_access_flag; \
        } \
    };

#include "jclass_attribute_gen.h"



#define class_name deprecated
#define name Deprecated
#include "jclass_attribute_gen.h"



#define class_name synthetic
#define name Synthetic
#include "jclass_attribute_gen.h"



#define class_name signature
#define name Signature
#define class_field \
    set_short(signature_index)

#include "jclass_attribute_gen.h"



#define class_name bootstrap_method
#define name BootstrapMethods
#define class_field \
    set_short(num_bootstrap_methods) \
    set_array(bootstrap_method, bootstrap_methods, num_bootstrap_methods)

#define class_body \
    struct bootstrap_method \
    { \
        u2 bootstrap_method_ref = 0; \
        u2 num_bootstrap_arguments = 0; \
        u2 *bootstrap_arguments = 0; \
\
        void read_from(input_stream& r) \
        { \
            r >> bootstrap_method_ref; \
            r >> num_bootstrap_arguments; \
            bootstrap_arguments = new u2[num_bootstrap_arguments]; \
            for (int i = 0; i < num_bootstrap_arguments; ++i) { \
                r >> (bootstrap_arguments[i]); \
            } \
        } \
\
        ~bootstrap_method() \
        { \
            delete [] bootstrap_arguments; \
        } \
    };

#include "jclass_attribute_gen.h"




#define class_name method_argument
#define name MethodParameters
#define class_field \
    set_char(parameters_count) \
    set_array(parameter, parameters, parameters_count)

#define class_body \
    struct parameter \
    { \
        static constexpr u4 ACC_FINAL       = 0x0010; \
        static constexpr u4 ACC_SYNTHETIC   = 0x1000; \
        static constexpr u4 ACC_MANDATED    = 0x8000; \
\
        u2 name_index = 0; \
        u2 access_flag = 0; \
\
        void read_from(input_stream& r) \
        { \
            r >> name_index; \
            r >> access_flag; \
        } \
    };

#include "jclass_attribute_gen.h"


#define class_name enclosing_method
#define name EnclosingMethod
#define class_field \
    set_short(class_index) \
    set_short(method_index)
#include "jclass_attribute_gen.h"



struct jclass_attr_annotation
{
    struct element_value;

    struct annotation
    {
        struct element_value_pair
        {
            u2 name_index = 0;
            element_value *value = nullptr;
        };

        u2 type_index = 0;
        u2 num_element_value_pairs = 0;
        element_value_pair *element_value_pairs = nullptr;

        void read_from(input_stream& r);

        ~annotation();
    };

    struct element_value
    {
        u1 tag = 0;

        u2 constant_value_index = 0;

        struct {
            u2 type_name_index = 0;
            u2 const_name_index = 0;
        } enum_const_value;

        u2 class_info_index = 0;

        annotation annotation_value;

        struct {
            u2 num_values = 0;
            element_value *values = nullptr;
        } array_value;

        void read_from(input_stream& r);
        ~element_value();
    };

    struct parameter_annotation
    {
        u2 num_annotations = 0;
        annotation *annotations = nullptr;

        void read_from(input_stream& in)
        {
            in >> num_annotations;
            annotations = new annotation[num_annotations];
            for (int i = 0; i < num_annotations; ++i) {
                annotations[i].read_from(in);
            }
        }

        ~parameter_annotation()
        {
            delete[] annotations;
        }
    };
};


#define class_name runtime_visible_annotation
#define name RuntimeVisibleAnnotations
#define class_field \
    set_short(num_annotations) \
    set_array(jclass_attr_annotation::annotation, annotations, num_annotations)

#include "jclass_attribute_gen.h"



#define class_name runtime_invisible_annotation
#define name RuntimeInvisibleAnnotations
#define class_field \
    set_short(num_annotations) \
    set_array(jclass_attr_annotation::annotation, annotations, num_annotations)

#include "jclass_attribute_gen.h"



#define class_name runtime_visible_param_annotation
#define name RuntimeVisibleParameterAnnotations
#define class_field \
    set_short(num_parameters) \
    set_array(jclass_attr_annotation::parameter_annotation, parameter_annotations, num_parameters)

#include "jclass_attribute_gen.h"


#define class_name runtime_invisible_param_annotation
#define name RuntimeInvisibleParameterAnnotations
#define class_field \
    set_short(num_parameters) \
    set_array(jclass_attr_annotation::parameter_annotation, parameter_annotations, num_parameters)

#include "jclass_attribute_gen.h"


#define class_name annotation_default
#define name AnnotationDefault
#define class_body \
    jclass_attr_annotation::element_value *element;

#define constructor \
    element = new jclass_attr_annotation::element_value(); \
    element->read_from(r);

#define destructor \
    delete element;

#include "jclass_attribute_gen.h"

}

#endif //JAVSVM_JCLASS_ATTRIBUTE_H
