

#include "../utils/global.h"
#include "../utils/log.h"
#include "../io/input_stream.h"

#include "jclass_const.h"
#include <unordered_map>


using namespace javsvm;


typedef jclass_const* (*jclass_const_factory)(input_stream&);


static std::unordered_map<int, jclass_const_factory> new_const_factory()
{
    std::unordered_map<int, jclass_const_factory> map;

#define _(x) map[jclass_const_##x::TAG] = (jclass_const_factory) jclass_const_##x::make;
    _(utf8)
    _(int)
    _(float)
    _(long)
    _(double)
    _(class)
    _(string)
    _(field_ref)
    _(method_ref)
    _(interface_ref)
    _(name_and_type)
    _(method_handle)
    _(method_type)
    _(dynamic)
    _(invoke_dynamic)
    _(module)
    _(package)
#undef _

    return map;
}


jclass_const* jclass_const_pool::read_from(input_stream& in)
{
    static std::unordered_map<int, jclass_const_factory> factory = new_const_factory();

    u1 tag = in.read<u1>();

    auto make = factory.find(tag);
    if (make == factory.end()) {
        return nullptr;
    }
    auto object = (make->second)(in);
    object->tag = tag;
    return object;
}


int jclass_const_pool::read_from(input_stream& in, int length)
{
    m_pools = new jclass_const*[length]; // is always nullptr ?
//    memset(m_pools, 0, m_size * sizeof(jclass_const*));

    for (m_size = 0; m_size < length; ++m_size) {
        // LOGI("[%d/%d][%#08x]\n", i, length, in.where());

        auto object = read_from(in);
        if (object == nullptr) {
            LOGE("unknown jclass_const tag at %#lx\n", in.where());
            return -1;
        }
        m_pools[m_size] = object;
        switch (object->tag) {
            case jclass_const_long::TAG: 
            case jclass_const_double::TAG:
                m_pools[++m_size] = nullptr;
        }
    }
    return 0;
}


jclass_const_pool::~jclass_const_pool()
{
    for (int i = 0; i < m_size; i ++) {
        delete m_pools[i];
    }
    delete[] m_pools;

    m_pools = nullptr;
    m_size = 0;
}
