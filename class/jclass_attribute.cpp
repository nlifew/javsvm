

#include "../io/input_stream.h"
#include "../utils/log.h"
#include "../utils/global.h"
#include "jclass_attribute.h"

#include <unordered_map>
#include <string>

using namespace javsvm;


typedef jclass_attribute* (*jclass_attribute_factory)(input_stream&, jclass_const_pool&);


static std::unordered_map<std::string, jclass_attribute_factory> new_attribute_factory()
{
    std::unordered_map<std::string, jclass_attribute_factory> map;
#define _(x) map[jclass_attr_##x::NAME] = (jclass_attribute_factory) jclass_attr_##x::make;
    _(code)
    _(exception)
    _(line_number)
    _(local_variable)
    _(local_variable_type)
    _(source_file)
    _(source_debug)
    _(constant)
    _(inner_class)
    _(deprecated)
    _(synthetic)
    _(signature)
    _(bootstrap_method)
    _(method_argument)
    _(enclosing_method)
    _(runtime_visible_annotation)
    _(runtime_invisible_annotation)
    _(runtime_visible_param_annotation)
    _(runtime_invisible_param_annotation)
    _(annotation_default)
#undef _
    return map;
}


jclass_attribute* jclass_attribute::read_from(input_stream& in, jclass_const_pool& pool)
{
    auto name_index = in.read<u2>();
    jclass_const *_name = pool.child_at(name_index - 1); 

    // class 文件中常量池的序号从 1 开始而不是从 0 开始
    // 因此需要 -1 来访问到正确的常量池

    if (_name == nullptr || _name->tag != jclass_const_utf8::TAG) {
        LOGE("invalid attribute name_index %u, which is NOT a jclass_const_utf8\n", name_index);
        return nullptr;
    }
    auto *utf8 = (jclass_const_utf8 *)_name;
    std::string key((char*) utf8->bytes, utf8->length);

    static std::unordered_map<std::string, jclass_attribute_factory> factory = new_attribute_factory();
    auto it = factory.find(key);
    jclass_attribute *object = it == factory.end() ? jclass_attr_unknown::make(in, pool) : (it->second)(in, pool);

    object->attribute_name_index = name_index;
    return object;
}


void jclass_attr_annotation::annotation::read_from(input_stream& in)
{
    in >> type_index;
    in >> num_element_value_pairs;
    element_value_pairs = new element_value_pair[num_element_value_pairs];
    for (int i = 0; i < num_element_value_pairs; ++i) {
        auto pair = element_value_pairs + i;
        in >> (pair->name_index);
        pair->value = new element_value();
        pair->value->read_from(in);
    }
}


jclass_attr_annotation::annotation::~annotation()
{
    for (int i = 0; i < num_element_value_pairs; ++i) {
        auto pair = element_value_pairs + i;
        delete pair->value;
    }
    delete[] element_value_pairs;
}


void jclass_attr_annotation::element_value::read_from(input_stream& in)
{
    in >> tag;
    switch (tag) {
        case 'B':
        case 'C':
        case 'D':
        case 'F':
        case 'I':
        case 'J':
        case 'S':
        case 'Z':
        case 's':
            in >> constant_value_index;
            break;
        case 'e':
            in >> enum_const_value.type_name_index;
            in >> enum_const_value.const_name_index;
            break;
        case 'c':
            in >> class_info_index;
            break;
        case '@':
            annotation_value.read_from(in);
            break;
        case '[':
            in >> array_value.num_values;
            array_value.values = new element_value[array_value.num_values];
            for (int i = 0; i < array_value.num_values; ++i) {
                array_value.values[i].read_from(in);
            }
            break;
    }
}


jclass_attr_annotation::element_value::~element_value()
{
    switch (tag) {
        case '[':
            delete[] array_value.values;
            break;
    }
}
