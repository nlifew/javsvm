

#ifndef JAVSVM_JVM_H
#define JAVSVM_JVM_H

#include "jenv.h"
#include "jheap.h"
#include "jmethod_area.h"
#include "../classloader/bootstrap_loader.h"
#include "../object/jarray.h"
#include "../object/jstring.h"
#include "../dll/dll_loader.h"

#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <vector>
#include <pthread.h>


namespace javsvm
{

class jvm
{
private:
    jvm() noexcept;

    std::mutex m_mutex;
    std::condition_variable m_cond;

    /**
     * 和当前虚拟机实例绑定的线程数，不包含 daemon 线程
     */
    volatile int m_active_threads_count = 0;

    struct pthread_equals {
        bool operator()(const pthread_t &p, const pthread_t &q) const noexcept
        {
            return pthread_equal(p, q);
        }
    };

    std::unordered_map<pthread_t, jenv*, std::hash<pthread_t>, pthread_equals> m_threads;
    std::vector<pthread_t> m_placeholder_threads;
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


    ~jvm() noexcept = default;
    jvm(const jvm&) = delete;
    jvm& operator=(const jvm&) = delete;

    /**
     * 提供一个全局的 jvm 实例，每个进程有且只有一个 jvm 实例
     */ 
    static inline jvm& get() noexcept
    {
        static jvm vm;
        return vm;
    }

    /**
     * 根据 jvm 实例拿到一个线程私有的 test 引用
     * 线程之前必须已经调用过 attach(), 否则会导致虚拟机退出
     */
    jenv& env() const noexcept;


    struct attach_info
    {
        bool is_daemon;
        class jvm *vm;
        size_t stack_size;
    };

    static attach_info DEFAULT_ATTACH_INFO;

    /**
     * 将一个非 java 线程附加到该虚拟机实例
     * 每个线程在调用 detach() 和 env() 之前都必须调用此函数
     * 如果已经附加过的线程再次调用 attach(), 会导致虚拟机退出
     *
     * PS: 如果想新开一个 java 线程，可以用以下代码:
     *
     * struct thread_args
     * {
     *      jmethod *entrance;
     *      jargs *args;
     * };
     *
     * static void* run(void *args) noexcept
     * {
     *      auto *t_args = (thread_args *) args;
     *
     *      jvm::get().attach();
     *
     *      // 运行 java 代码
     *      t_args->entrance->invoke(* (t_args->args));
     *
     *      // 自己回收资源
     *      pthread_detach(pthread_self());
     *      jvm::get().detach();
     *      return nullptr;
     * }
     *
     * int start_new_java_thread(jmethod *entrance, jargs &args)
     * {
     *      pthread_t tid;
     *      int status;
     *
     *      thread_args t_args = {
     *          .entrance = entrance,
     *          .args = &args,
     *      };
     *
     *      status = pthread_create(&tid, nullptr, run, &t_args);
     *      if (status != 0) {
     *          // 抛出对应的 java 异常
     *          return -1;
     *      }
     *
     *      // 通知 vm 马上会有一个新线程要附加到 vm 上
     *      jvm::get().placeholder(tid);
     *
     *      return 0;
     * }
     */
    jenv& attach(attach_info *info = nullptr) noexcept;

    /**
     * 通知 vm，即将有一个线程会附加到该虚拟机实例上。如果在附加成功之前，
     * 虚拟机内已经没有活跃的非 daemon 线程，也不会退出
     */
    void placeholder(pthread_t tid) noexcept;

    /**
     * 将线程从虚拟机实例上分离。调用前必须保证已经调用过 attach(), 否则会导致虚拟机退出.
     * 线程分离之后允许调用 attach() 重新附加
     */
    void detach() noexcept;

    /**
     * 测试当前线程有没有附加到该虚拟机实例。
     * 如果已经附加到，会返回其 env 的指针；否则返回 nullptr
     */
    jenv *test() const noexcept;

    /**
     * 主线程退出前必须调用的，等待所有的非 daemon 线程结束
     */
     void wait_for() noexcept;

     /**
      * 获取附加到该虚拟机实例上的所有线程
      * @param out 要写入的地址
      * @return 一共有多少线程数
      */
     int all_threads(std::vector<jenv*> *out) noexcept;
};

} // namespace javsvm



#endif
