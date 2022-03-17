

#ifndef JAVSVM_JFIELD_H
#define JAVSVM_JFIELD_H

#include "../utils/global.h"
#include "../class/jclass_file.h"
#include "../vm/jheap.h"
#include "../engine/engine.h"
#include "jobject.h"

namespace javsvm
{

struct jclass;

struct jfield
{
    /**
     * 指向该字段所在的 class
     */ 
    jclass *clazz = nullptr;

    /**
     * 指向 .class 文件中的 jclass_field 信息，用来获取 access_flag 等信息
     */ 
    jclass_field *orig = nullptr;

    /**
     * 缓存下来的这个字段的名称和类型签名
     */ 
    const char *name = nullptr;

    const char *sig = nullptr;

    u4 access_flag = 0;


    /**
     * 这个字段的偏移量
     * 对于 static 类型，指的是相对于 @{link javsvm::jclass::extra} 的偏移量；
     * 对于非 static 类型，指的是相对于 @{link javsvm::jobject::values} 的偏移量
     */ 

    int mem_offset = 0;

    /**
     * 该字段所占的真实内存大小
     * NOTE: 对于引用类型，这个值的大小就是 sizeof(jref)
     */ 
    int mem_size = 0;


    /**
     * 表示当前这个字段的类型。
     * 虽说我们只根据 sig[0] 就能判断出这个字段是什么类型，但由于
     * 'B', 'Z', 'S' 等每个 case 在数值上相差较大，编译器可能不会对 switch 做查表优化。
     */
    enum flat_type {
        BOOLEAN = 0,
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        OBJECT,
        ARRAY
    };
    enum flat_type type = BOOLEAN;

    /**
     * 初始化函数
     */ 
    void bind(jclass *clazz, jclass_file *cls, int index);


    static int compare_to(const jfield* o1, const jfield *o2)
    {
        if (o1 == o2) return 0;
        return strcmp(o1->name, o2->name);
    }


    inline jvalue get(jref obj) const noexcept
    {
        auto ptr = jheap::cast(obj);
        if (ptr == nullptr) {
            throw_exp("java/lang/NullPointerException", name);
            return { 0 };
        }
        return get0(ptr->values + mem_offset, mem_size);
    }

    inline jvalue get_static() const noexcept
    {
        if (clazz->invoke_clinit() < 0) {
            return { 0 };
        }
        return get0((char*) clazz->data + mem_offset, mem_size);
    }


    inline void set(jref obj, jvalue val) const noexcept
    {
        auto ptr = jheap::cast(obj);
        if (ptr == nullptr) {
            throw_exp("java/lang/NullPointerException", name);
            return;
        }
        set0(ptr->values + mem_offset, val, mem_size);
    }

    inline void set_static(jvalue val) const noexcept
    {
        if (clazz->invoke_clinit() < 0) {
            return;
        }
        set0(clazz->data + mem_offset, val, mem_size);
    }

private:
    static inline jvalue get0(const void *src, int size) noexcept
    {
        jvalue val;
        switch (size) {
            case 1: val.b = *(jbyte *) src; break;
            case 2: val.s = *(jshort *) src ;break;
            case 4: val.i = *(jint *) src; break;
            case 8: val.j = *(jlong *) src; break;
            default: val.j = 0; break;
        }
        return val;
    }

    static inline void set0(void *dst, jvalue val, int size) noexcept
    {
        switch (size) {
            case 1: *(jbyte *) dst = val.b; break;
            case 2: *(jshort *) dst = val.s; break;
            case 4: *(jint *) dst = val.i; break;
            case 8: *(jlong *) dst = val.j; break;
            default:
                break;
        }
    }
};

} // namespace javsvm
#endif

