

#ifndef JAVSVM_JFIELD_H
#define JAVSVM_JFIELD_H

#include "../utils/global.h"
#include "../class/jclass_file.h"

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
     * 该字段所占的变量槽大小
     * NOTE: 对于引用类型，这个值是 1，因为引用类型 slotof(jref) == 1
     */
    int slot_num = 0;


    /**
     * 初始化函数
     */ 
    void bind(jclass *clazz, jclass_file *cls, int index);


    static int compare_to(const jfield* o1, const jfield *o2)
    {
        if (o1 == o2) return 0;
        return strcmp(o1->name, o2->name);
    }

    // todo:
    jvalue get(jref obj) { return {}; }

//    jvalue get(void *values) { return jvalue {}; }

    void set(jref obj, jvalue val) {  }

//    void set(void *values, jvalue val) {}
};

} // namespace javsvm
#endif

