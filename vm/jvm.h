

#ifndef JAVSVM_JVM_H
#define JAVSVM_JVM_H

#include "jenv.h"
#include "jheap.h"
#include "jmethod_area.h"
#include "../classloader/bootstrap_loader.h"
#include "../object/jarray.h"
#include "../object/jstring.h"
#include "../dll/dll_loader.h"

namespace javsvm
{

class jvm
{
private:
    jvm() noexcept;


    /**
     * jni 使用的保留区域
     */
    char m_jni_reserved[10 * sizeof(void*)];

public:
    /**
     * 堆
     */
    jheap heap;

    /**
     * 方法区
     */
    jmethod_area method_area;

    /**
     * 启动类加载器
     */
    bootstrap_loader bootstrap_loader;

    /**
     * 数组辅助类
     */
    jarray array;

    /**
     * 动态库加载类
     */
    dll_loader dll_loader;

    /**
     * 字符串池
     */
    jstring string;


    ~jvm() = default;
    jvm(const jvm&) = delete;
    jvm& operator=(const jvm&) = delete;

    /**
     * 提供一个全局的 jvm 实例，每个进程有且只有一个 jvm 实例
     */ 
    static jvm& get() noexcept;

    /**
     * 根据 jvm 实例拿到一个线程私有的 env 引用
     * 线程之前必须已经调用过 attach(), 否则会导致虚拟机退出
     */
    jenv& env() const noexcept;

    /**
     * 讲一个本地进程附加到该虚拟机实例
     * 每个线程在调用 detach() 和 env() 之前都必须调用此函数
     */
    jenv& attach() noexcept;

    /**
     * 将线程从虚拟机实例上分离。分离之后允许调用 attach() 重新附加。
     */
    void detach() const noexcept;

    /**
     * 测试当前线程有没有附加到该虚拟机实例。
     * 如果已经附加到，会返回其 env 的指针；否则返回 nullptr。
     * 参数 int 无意义，仅用来函数重载
     */
    jenv *env(int) const noexcept;

    /**
     * 获取 jni 层 JavaVM 指针
     */
     void *jni() const noexcept;
};

} // namespace javsvm



#endif
