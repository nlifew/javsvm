

#ifndef class_name
#error Please define `class_name`
#endif

#ifndef tag
#error Please define `tag`
#endif


#define _MICRO_ADD(x, y) x##y
#define _(x) _MICRO_ADD(jclass_const_, x)


struct _(class_name) : jclass_const
{

#ifdef class_field
#define set_char(n)         u1 n = 0;
#define set_short(n)        u2 n = 0;
#define set_int(n)          u4 n = 0;
#define set_long(n)         u8 n = 0;
#define set_bytes(n, len)   u1 *n = nullptr;
    class_field
#undef set_char
#undef set_short
#undef set_int
#undef set_long
#undef set_bytes
#endif

#ifdef class_body
    class_body
#endif

    explicit _(class_name)(input_stream& r)
    {
    #ifdef class_field
    #define set_char(n)     r >> n;
    #define set_short(n)    r >> n;
    #define set_int(n)      r >> n;
    #define set_long(n)     r >> n;
    #define set_bytes(n, len) \
                n = new u1[len + 1];  \
                r.read_bytes(n, len); \
                n[len] = '\0';
        class_field
    #undef set_char
    #undef set_short
    #undef set_int
    #undef set_long
    #undef set_bytes
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
        class_field
    #undef set_char
    #undef set_short
    #undef set_int
    #undef set_long
    #undef set_bytes
    #endif

    #ifdef destructor
        destructor
    #endif
    }
    

    static const u4 TAG = tag;
    static _(class_name) *make(input_stream& r) 
    { 
        return new _(class_name)(r); 
    }

    _(class_name)(const _(class_name)&) = delete;
    _(class_name)& operator=(const _(class_name)&) = delete;
};


#undef _MICRO_ADD
#undef _

#undef tag
#undef class_name
#undef class_body
#undef class_field
#undef constructor
#undef destructor
