

#ifndef JAVSVM_JMETHOD_H
#define JAVSVM_JMETHOD_H

#include "../utils/global.h"
#include "../class/jclass_file.h"

namespace javsvm
{

struct jclass;
struct jargs;
class jmethod_area;

struct jmethod
{
    /**
     * 指向该函数所在的类
     */ 
    jclass *clazz = nullptr;

    /**
     * 指向 .class 文件中的 jclass_method，用来保存字节码以及异常表等
     */ 
    jclass_method *orig = nullptr;


    /**
     * 该函数在虚函数表/接口函数表中的 index，不同的函数有不同的意义:
     * 1. 如果非虚函数(静态函数和私有函数)，始终为 -1;
     * 2. 如果该虚函数来自非接口类，表示其在 vtable 中的 index. 即:
     * *(this->clazz->vtable[index_in_table]) == *this;
     * 一旦在类加载阶段初始化，不再改变;
     * 3. 如果该虚函数来自接口类，初始值为 -1，但会随着传进来的 object 的变化而变化.
     * 也就是说，invoke_interface(object) 之后，index_in_table 表示其在 object->clazz->itable
     * 中的 index。为了和 [2] 区分开，取值范围是 [-1, -∞). 即:
     * *(object->clazz->itable[-index_in_table - 1]) == *this.
     * 这样的话下次如果传进来相同的对象，我们就能直接根据这个 index 快速确定接口实现，
     * 不用再查找接口函数表.
     */
    int index_in_table = -1;


    /**
     * 缓存下来的函数名和参数签名 (因为经常被使用)
     */ 
    const char *name = nullptr;
    const char *sig = nullptr;

    u4 access_flag = 0;
    

    /**
     * 是否是虚函数 (用来判断是否要查询虚函数表)
     */ 
    bool is_virtual = false;

    /**
     * 指向函数入口的指针，如果是一个普通的函数，它指向 jclass_attr_code
     * 如果是一个 native 函数，应该指向一个 jni 函数
     */ 
    union {
        void *jni_func;
        jclass_attr_code *code_func = nullptr;
    } entrance;

    /**
     * 函数参数和返回值要占用的变量槽数量
     */
    int args_slot = 0;
    int return_slot = 0;


    char *args_ref_table = nullptr;

    /**
     * 比较函数，查询虚函数表时使用
     */ 
    static int compare_to(const jmethod *o1, const jmethod *o2)
    {
        if (o1 == o2) return 0;
        int cmp = strcmp(o1->name, o2->name);
        if (cmp == 0) cmp = strcmp(o1->sig, o2->sig);
        return cmp;
    }
//    bool operator<(const jmethod *m) const { return compare_to(this, m) < 0; }
//    bool operator>(const jmethod *m) const { return compare_to(this, m) > 0; }
//    bool operator==(const jmethod *m) const { return compare_to(this, m) == 0; }

    /**
     * 初始化函数
     * @param klass 所属 jclass
     * @param cls 所在的原始 .class 文件
     * @param index 当前函数在 .class 文件中的位置
     */ 

    void bind(jmethod_area &allocator, jclass *klass, jclass_file *cls, int index) noexcept;

    // jvalue invoke(operand_t *args) { return invoke(this, args, is_virtual); }

    jvalue invoke_static(jargs &args);

    jvalue invoke_special(jref ref, jargs &args);

    jvalue invoke_virtual(jref ref, jargs &args) const;

    jvalue invoke_interface(jref ref, jargs &args);
};

} // namespace javsvm
#endif 

