

#ifndef class_name
#error Please define 'class_name'
#endif

#ifndef name
#define name class_name
#endif


#define _TO_STR(x) #x
#define TO_STR(x) _TO_STR(x)
#define _MICRO_ADD(x, y) x##y
#define _(x) _MICRO_ADD(jclass_attr_, x)


struct _(class_name) : jclass_attribute
{

#ifdef class_body
    class_body
#endif


#ifdef class_field
#define set_char(n) u1 n = 0;
#define set_short(n) u2 n = 0;
#define set_int(n) u4 n = 0;
#define set_long(n) u8 n = 0;
#define set_bytes(n, len) u1 *n = nullptr;
#define set_array(t, n, len) t *n = nullptr;
    class_field
#undef set_char
#undef set_short
#undef set_int
#undef set_long
#undef set_bytes
#undef set_array
#endif

    _(class_name)(input_stream& r, jclass_const_pool& _pool)
    {
        // attribute_length = r.read_int(true);
        pool = &_pool;
        r >> attribute_length;

    #ifdef class_field
    #define set_char(n) r >> n;
    #define set_short(n) r >> n;
    #define set_int(n) r >> n;
    #define set_long(n) r >> n;
    #define set_bytes(n, len) \
            n = new u1[len + 1]; \
            r.read_bytes(n, len); \
            n[len] = '\0';
    #define set_array(t, n, len) \
            n = new t[len]; \
            for (int i = 0; i < len; ++i) n[i].read_from(r);
        class_field
    #undef set_char
    #undef set_short
    #undef set_int
    #undef set_long
    #undef set_bytes
    #undef set_array
    #endif

    #ifdef constructor
        constructor
    #endif
    }

    ~_(class_name)() override
    {
    #ifdef class_field
    #define set_char(n)
    #define set_short(n)
    #define set_int(n)
    #define set_long(n)
    #define set_bytes(n, len) delete[] n; n = nullptr; len = 0;
    #define set_array(t, n, len) delete[] n; n = nullptr; len = 0;
        class_field
    #undef set_char
    #undef set_short
    #undef set_int
    #undef set_long
    #undef set_bytes
    #undef set_array
    #endif

    #ifdef destructor
        destructor
    #endif
    }

    static constexpr const char* NAME = TO_STR(name);
    static _(class_name) *make(input_stream& r, jclass_const_pool& _pool) 
    {
        // LOGI("jclass_attr_%s instance at %#x\n", TO_STR(class_name), r.where());
        return new _(class_name)(r, _pool); 
    }

    _(class_name)(const _(class_name)&) = delete;
    _(class_name)& operator=(const _(class_name)&) = delete;
};


#undef _TO_STR
#undef TO_STR

#undef _MICRO_ADD
#undef _

#undef name
#undef class_name
#undef class_body
#undef class_field
#undef constructor
#undef destructor
