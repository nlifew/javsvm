

#ifndef JAVSVM_JVM_H
#define JAVSVM_JVM_H

#include "jenv.h"
#include "jheap.h"
#include "jmethod_area.h"
#include "../classloader/bootstrap_loader.h"
#include "../object/jarray.h"
//#include "../object/jstring.h"

namespace javsvm
{

class jvm
{
private:
    jvm() noexcept: class_loader(*this), array(*this)
    {
    }
public:
    jheap heap;                       /* 堆 */
    jmethod_area method_area;         /* 方法区 */
    bootstrap_loader class_loader;    /* 启动类加载器 */
    jarray array;

//    dll_loader m_dll_loader;            /* 动态库加载类 */
//    jstring m_string_pool;              /* 字符串池 */

    ~jvm() = default;
    jvm(const jvm&) = delete;
    jvm& operator=(const jvm&) = delete;

    /**
     * 提供一个全局的 jvm 实例，每个进程有且只有一个 jvm 实例
     */ 
    static jvm& get();

    /**
     * 根据 jvm 实例拿到一个线程私有的 env 引用
     */
    jenv& env();

    jenv& attach();

    void detach();

//    void load_library(const char *name) { return m_dll_loader.load_library(name); }

//    jstring &string_pool() { return m_string_pool; }
};

} // namespace javsvm



#endif
