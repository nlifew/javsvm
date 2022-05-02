
#include "jni/jni_utils.h"
#include "java/lang/Thread.h"

#include "object/jclass.h"
#include "object/jmethod.h"
#include "object/jfield.h"

#include <pthread.h>
#include <unistd.h>

struct
{
    javsvm::jclass *klass = nullptr;
    javsvm::jfield *tid = nullptr;
    javsvm::jfield *daemon = nullptr;
    javsvm::jfield *name = nullptr;
    javsvm::jfield *priority = nullptr;
    javsvm::jfield *stackSize = nullptr;

    javsvm::jmethod *_init_ = nullptr;
    javsvm::jmethod *run = nullptr;
    javsvm::jmethod *exit = nullptr;
} java_lang_Thread;


/**
 * class: java/lang/Thread
 * method: registerNatives
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_Thread_registerNatives
        (JNIEnv *, jclass)
{
    safety_area_guard guard;

    auto &vm = javsvm::jvm::get();
    auto klass = vm.bootstrap_loader.load_class("java/lang/Thread");

    java_lang_Thread.klass = klass;
    java_lang_Thread.tid = klass->get_field("tid", "J");
    java_lang_Thread.name = klass->get_field("name", "Ljava/lang/String;");
    java_lang_Thread.daemon = klass->get_field("daemon", "Z");
    java_lang_Thread.priority = klass->get_field("priority", "I");
    java_lang_Thread.stackSize = klass->get_field("stackSize", "J");

    java_lang_Thread._init_ = klass->get_method("<init>", "(J)V");
    java_lang_Thread.run = klass->get_method("run", "()V");
    java_lang_Thread.exit = klass->get_method("exit", "()V");
}

/**
 * class: java/lang/Thread
 * method: currentThread
 * sig: ()Ljava/lang/Thread;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Thread_currentThread
    (JNIEnv *, jclass)
{
    safety_area_guard guard;
    auto &env = javsvm::jvm::get().env();

    // 先检查 jenv::thread 是否为空，不是则返回
    // 对于 java 创建的线程，这个字段一定不是 nullptr
    if (env.thread != nullptr) {
        return to_object(env.thread);
    }
    // 此处注意: 先创建个 Thread 对象，设置给 jenv::thread, 然后再调构造函数
    // 避免 Thread init() 递归调用 currentThread()
    auto thread = java_lang_Thread.klass->new_instance();
    env.thread = thread;

    javsvm::slot_t _args[3];
    javsvm::jargs args = _args;
    args.next<javsvm::jref>() = thread;
    args.next<javsvm::jlong>() = reinterpret_cast<javsvm::jlong>(pthread_self());

    java_lang_Thread._init_->invoke_special(thread, args);
    return to_object(thread);
}




/**
 * 新线程的入口点
 */
static void* run(void *args) noexcept
{
    LOGI("new thread start %p\n", pthread_self());
    javsvm::gc_root &self = *reinterpret_cast<javsvm::gc_root*>(args);

    // NOTE: 不要在栈上保留 jref, gc 可能会修改它
//    auto self = this_object->get();
    std::unique_ptr<javsvm::gc_root, void(*)(const javsvm::gc_root *)> self_guard(
            &self, [](const javsvm::gc_root *ptr) { delete ptr; });

    // 设置 pthread 参数
    LOGI("setup pthread attrs %p\n", pthread_self());
    javsvm::jref name = java_lang_Thread.name->get(*self).l;
    const char *name_str = javsvm::jstring::utf8(name);
    std::unique_ptr<const char, void(*)(const char *)> name_guard(
            name_str, [](const char *ptr) { delete[] ptr; });

    pthread_setname_np(name_str);

    // 准备附加到虚拟机
    LOGI("thread '%s' try to attach to vm instance %p\n", name_str, pthread_self());
    {
        javsvm::jref thread = *self;
        bool daemon = java_lang_Thread.daemon->get(thread).z;
        size_t stack_size = java_lang_Thread.stackSize->get(thread).j;

        javsvm::jvm::attach_info info = {
                .is_daemon = daemon,
                .vm = nullptr,
                .stack_size = stack_size,
        };
        auto &env = javsvm::jvm::get().attach(&info);
        env.thread = thread;
    }

    // 其余一些字段的初始化
    LOGI("sync to java Thread object %p\n", pthread_self());
    {
        javsvm::jvalue val;
        val.j = reinterpret_cast<javsvm::jlong>(pthread_self());
        java_lang_Thread.tid->set(*self, val);
    }

    // 回调 Thread.run()
    LOGI("call run() [%s](%p)\n", name_str, pthread_self());
    {
        javsvm::slot_t _args[1];
        javsvm::jargs jargs = _args;
        jargs.next<javsvm::jref>() = *self;
        java_lang_Thread.run->invoke_virtual(*self, jargs);
    }

    // 回调 Thread.exit(), 执行 java 层的清理工作
    LOGI("call Thread.exit() [%s](%p)\n", name_str, pthread_self());
    {
        javsvm::slot_t _args[1];
        javsvm::jargs jargs = _args;
        jargs.next<javsvm::jref>() = *self;
        java_lang_Thread.exit->invoke_virtual(*self, jargs);
    }

    // 最后的清理工作，脱离 vm 实例等
    LOGI("detach from java vm [%s](%p)\n", name_str, pthread_self());
    {
        java_lang_Thread.tid->set(*self, { .j = 0 });
        javsvm::jvm::get().detach();
    }

    LOGI("Thread [%s](%p) end\n", name_str, pthread_self());
    
    return nullptr;
}



static int setup_pthread_attr(pthread_attr_t *attr, jobject self)
{
    auto object = to_object(self);

//    bool daemon = java_lang_Thread.daemon->get(object).z;

    int priority = java_lang_Thread.priority->get(object).i;
    size_t stack_size = java_lang_Thread.stackSize->get(object).j;

    // name 属性不在 attr 里设置
//    javsvm::jref name = java_lang_Thread.name->get(object).l;
//
//    const char *name_str = javsvm::jstring::utf8(name);
//    std::unique_ptr<const char, void(*)(const char*)> name_guard(
//            name_str, [](const char *ptr) { delete[] ptr; });

    int inited = 0;

    pthread_attr_init(attr);

    if (stack_size > 0) {
        inited = 1;
        pthread_attr_setstacksize(attr, stack_size);
    }
    if (priority > 0) {
        inited = 1;
        // todo: 实现线程优先级设置
//        pthread_attr_setschedpolicy();
//        pthread_attr_setschedparam(&attr, &sched);
    }
    return inited;
}

/**
 * class: java/lang/Thread
 * method: start0
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_Thread_start0
        (JNIEnv *, jobject self)
{
    safety_area_guard guard;

    // 创建一个新的全局引用。父线程可能比子线程提前返回导致局部引用失效
    auto &vm = javsvm::jvm::get();
    auto this_object = new javsvm::gc_root(to_object(self));

    // 准备参数
    pthread_attr_t attr;
    int attr_setup = setup_pthread_attr(&attr, self);
    std::unique_ptr<pthread_attr_t, void(*)(pthread_attr_t *)> attr_guard(
            &attr, [](pthread_attr_t *ptr) { pthread_attr_destroy(ptr); });

    pthread_t tid;
    auto ret = pthread_create(&tid, attr_setup ? &attr : nullptr, run, this_object);
    if (UNLIKELY(ret != 0)) {
        PLOGE("failed to create a new pthread");
        exit(1);
    }
    // 通知 vm 即将有一个新线程加入
    vm.placeholder(tid);
}

/**
 * class: java/lang/Thread
 * method: dumpThreads
 * sig: ([Ljava/lang/Thread;)[[Ljava/lang/StackTraceElement;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Thread_dumpThreads
    (JNIEnv *env, jclass klass, jobjectArray)
{
    // todo
    return nullptr;
}

/**
 * class: java/lang/Thread
 * method: getThreads
 * sig: ()[Ljava/lang/Thread;
 */
extern "C" JNIEXPORT jobjectArray JNICALL
Java_java_lang_Thread_getThreads
    (JNIEnv *env, jclass klass)
{
    // todo
    return nullptr;
}

/**
 * class: java/lang/Thread
 * method: holdsLock
 * sig: (Ljava/lang/Object;)Z
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_java_lang_Thread_holdsLock
    (JNIEnv *env, jclass klass, jobject)
{
    // todo
    return false;
}

/**
 * class: java/lang/Thread
 * method: interrupt0
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_Thread_interrupt0
    (JNIEnv *env, jobject self)
{
    // todo
}

/**
 * class: java/lang/Thread
 * method: isInterrupted
 * sig: (Z)Z
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_java_lang_Thread_isInterrupted
    (JNIEnv *env, jobject self, jboolean)
{
    // todo
    return false;
}


/**
 * class: java/lang/Thread
 * method: setNativeName
 * sig: (Ljava/lang/String;)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_Thread_setNativeName
    (JNIEnv *, jobject self, jstring name)
{
    safety_area_guard guard;

    auto name_str = javsvm::jstring::utf8(to_object(name));
    std::unique_ptr<const char, void(*)(const char *)> name_guard(
            name_str, [](const char *ptr) { delete[] ptr; });

    if (javsvm::check_exception()) {
        return;
    }

    jlong tid = java_lang_Thread.tid->get(to_object(self)).j;
    auto pid = reinterpret_cast<pthread_t>(tid);

    // 只支持给本线程设置线程名
    if (pthread_equal(pthread_self(), pid)) {
        pthread_setname_np(name_str);
    }
}

/**
 * class: java/lang/Thread
 * method: setPriority0
 * sig: (I)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_Thread_setPriority0
    (JNIEnv *, jobject, jint)
{
    // todo: 实现线程优先级设置
}

/**
 * class: java/lang/Thread
 * method: sleep
 * sig: (J)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_Thread_sleep
    (JNIEnv *, jclass, jlong milli)
{
    safety_area_guard guard;

    // 不使用 std::this_thread::sleep_for(),
    // 如果在 sleep_for() 期间发生了中断，它是不会返回的。
    auto ret = usleep(milli * 1000);
    if (ret != 0) {
        javsvm::throw_exp(InterruptedException, "");
    }
}


/**
 * class: java/lang/Thread
 * method: yield
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_Thread_yield
    (JNIEnv *, jclass)
{
    pthread_yield_np();
}

