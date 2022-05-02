//
// Created by edz on 2022/1/29.
//

#include <chrono>
#include <csignal>
#include <atomic>

#include "gc_root.h"
#include "gc_thread.h"
#include "../vm/jvm.h"
#include "../object/jfield.h"
#include "../object/jmethod.h"


using namespace javsvm;



static gc_thread *self = nullptr;



void gc_thread::register_sigsegv_handler() noexcept
{
    struct sigaction sig {};
    sig.sa_handler = [](int) {
        self->handle_sigsegv();
    };
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;

    if (sigaction(SIGSEGV, &sig, &m_sigsegv_backup) < 0) {
        PLOGE("register_sigsegv_handler: fail and abort\n");
        exit(1);
    }
}

void gc_thread::unregister_sigsegv_handler() noexcept
{
    sigaction(SIGSEGV, &m_sigsegv_backup, nullptr);
}

void gc_thread::handle_sigsegv() noexcept
{
    // 先判断当前这个中断是不是 gc 允许的. 如果不是，走之前的逻辑
    if (javsvm::safety_point_trap != nullptr) {
        PLOGE("handle_sigsegv: SIGSEGV without gc\n");
        exit(1);
    }
    // signal handler 内的变量必须是 volatile 类型
    thread_local volatile int is_doing_gc = 0;
    if (is_doing_gc) {
        PLOGE("handle_sigsegv: SIGSEGV when gc\n");
        exit(1);
    }
    is_doing_gc = 1;

    std::unique_lock lck(m_mutex);
    ++ m_blocked_threads_count;

    if (is_the_world_stopped()) {
        m_stw_cond.notify_one();
    }
    while (javsvm::safety_point_trap == nullptr) {
        m_trap_cond.wait(lck);
    }

    -- m_blocked_threads_count;
    is_doing_gc = 0;
}


bool gc_thread::is_the_world_stopped() const noexcept
{
    if (m_gc_frozen_counter != 0) {
        return false;
    }

    int blocked = m_blocked_threads_count;
    int total = jvm::get().all_threads(nullptr);

    assert(blocked <= total);
    return blocked == total;
}


void javsvm::enter_safety_area() noexcept
{
    ++ self->m_blocked_threads_count;
    if (javsvm::safety_point_trap != nullptr) {
        return;
    }

    std::unique_lock lck(self->m_mutex);
    if (self->is_the_world_stopped()) {
        self->m_stw_cond.notify_one();
    }
}

void javsvm::leave_safety_area() noexcept
{
    -- self->m_blocked_threads_count;
    if (javsvm::safety_point_trap != nullptr) {
        return;
    }

    std::unique_lock lck(self->m_mutex);
    while (javsvm::safety_point_trap == nullptr) {
        self->m_trap_cond.wait(lck);
    }
}

void javsvm::freeze_gc_thread() noexcept
{
    ++ self->m_gc_frozen_counter;
    if (javsvm::safety_point_trap != nullptr) {
        return;
    }
    std::unique_lock lck(self->m_mutex);
    if (self->is_the_world_stopped()) {
        self->m_stw_cond.notify_one();
    }
}

void javsvm::unfreeze_gc_thread() noexcept
{
    -- self->m_gc_frozen_counter;
    if (javsvm::safety_point_trap != nullptr) {
        return;
    }
    std::unique_lock lck(self->m_mutex);
    if (self->is_the_world_stopped()) {
        self->m_stw_cond.notify_one();
    }
}


static char dummy_trap = '\0';
volatile char *javsvm::safety_point_trap = nullptr;

gc_thread::gc_thread() noexcept
{
    assert(self == nullptr);
    self = this;
    safety_point_trap = &dummy_trap;
    register_sigsegv_handler();

    m_alive_objects.reserve(64);
}

gc_thread::~gc_thread() noexcept
{
    unregister_sigsegv_handler();
    self = nullptr;
}


void gc_thread::gc() noexcept
{
    std::unique_lock lck(m_mutex);

    if (javsvm::safety_point_trap != nullptr) {
        javsvm::safety_point_trap = nullptr;
        m_trap_cond.notify_all(); // 按理说 notify_one() 也可以，毕竟此时只有一个 gc 线程在等待
    }
    if (m_native_thread == nullptr) {
        auto t = new (m_native_thread_buff) std::thread([=] {
            run();
        });
        t->detach();
        m_native_thread = t;
    }
    ++ m_blocked_threads_count;
    if (is_the_world_stopped()) {
        m_stw_cond.notify_one();
    }

    while (javsvm::safety_point_trap == nullptr) {
        m_trap_cond.wait(lck);
    }
    -- m_blocked_threads_count;
}

[[noreturn]]
void gc_thread::run() noexcept
{
    while (true) {
        std::unique_lock lck(m_mutex);
        while (javsvm::safety_point_trap != nullptr) {
            m_trap_cond.wait(lck);
        }
        while (! is_the_world_stopped()) {
            m_stw_cond.wait(lck);
        }

        // 撒，细数你的罪恶吧
        mark();

        compact();

        javsvm::safety_point_trap = &dummy_trap;
        m_trap_cond.notify_all();
    }
}

// 三色标记法，如果一个对象没有被 gc 访问过，为白色;
// 如果该对象的全部字段都已经被访问过，则为黑色; 如果正在遍历，则为灰色
// 这里我们增加了一个新的颜色，紫色。当该对象的全部字段都被遍历完，且该对象是
// 数组类型，则染成紫色。

enum color_t
{
    WHITE = 0,
    GREY = 1,
    PURPLE = 2,
    BLACK = 3,
    MASK = 3,
};


#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"

void gc_thread::trace_and_mark(jobject *root) noexcept
{
    if (root == nullptr) {
        return;
    }
    jclass *klass = root->klass;
    auto klass_val = (uint64_t) klass;

    int color = (int) (color_t::MASK & klass_val);

    // 如果这个节点不是白色（灰色或者黑色），不再遍历
    if (color != color_t::WHITE) {
        return;
    }

    // 染成灰色
    root->klass = (jclass *) (color_t::GREY | klass_val);

//    assert(klass->name[0] == 'L' || klass->name[0] == '['); // 字节码中的类名不是以 L 开头 :(

    // root 可能是对象引用，也可能是数组引用，我们要区分下
    if (klass->name[0] != '[') {
        // 对象类型。虚拟机当然知道哪个位置存在引用（在类加载器的配合下）
        for (jclass *cls = klass; cls; cls = cls->super_class) {
            for (int i = 0, z = cls->ref_table_size; i < z; ++i) {
                uint64_t ref = *(uint64_t *) (root->values + cls->ref_tables[i]);
                int ret_type = (int) (jheap::R_MSK & ref);
                if (ret_type == jheap::R_STR) { // 强引用，递归处理
                    trace_and_mark((jobject *) ref);
                }
                else {
                    special_for_java_lang_ref((jref) ref, cls->ref_tables[i]);
                }
            }
        }
        // 染成黑色
        root->klass = (jclass *) (color_t::BLACK | klass_val);
    }
    else if (klass->name[1] == '[' || klass->name[1] == 'L') {
        // 对象数组要遍历数组中的每一个元素
        int len = 0, ele_size = 0;
        auto array = (jref *) jarray::storage_of(root, &len, &ele_size);
        assert(ele_size == sizeof(jref));

        for (int i = 0; i < len; i ++) {
            jref ref = array[i];
            // 数组中的引用一定是强引用
            assert((jheap::R_MSK & (uint64_t) ref) == jheap::R_STR);
            trace_and_mark((jobject *) ref);
        }

        // 对象数组类型要染成紫色
        root->klass = (jclass *) (color_t::PURPLE | klass_val);
    }
    else {
        // 普通数组对象，染成黑色
        root->klass = (jclass *) (color_t::BLACK | klass_val);
    }

    // 添加进队列
    m_alive_objects.push_back(root);
    m_alive_memory += root->size();
}
#pragma clang diagnostic pop


void gc_thread::special_for_java_lang_ref(jref ref, int offset) noexcept
{

    // todo: 增加对软引用，弱引用和虚引用的处理

//    // 一定来自 java/lang/ref 的类，而且一定是最后一个字段
//    switch (jheap::R_MSK & (uint64_t) ref) {
//        case jheap::R_SFT:
//        case jheap::R_WEK:
//        case jheap::R_PHA:
//            break;
//    }
}




#define MS(t) ((int) std::chrono::duration_cast<std::chrono::milliseconds>(t).count())



template <typename T>
static void lookup_gc_root(std::list<jref> &v, const T &t) noexcept
{
    // 遍历手动指定的 GcRoots
    gc_root::ref_pool.lookup([&](gc_root *it) {
        t(it->original());
    });

    // 遍历所有的静态字段
    gc_root::static_field_pool.lookup([&](jref *it) {
        t(* it);
    });

    // 字符串池
    jvm::get().string.pool().lookup([&](std::pair<const std::string, jref> &pair) {
        t(pair.second);
    });

    // 所有的 java 线程
    std::vector<jenv*> threads;
    jvm::get().all_threads(&threads);
    for (const auto &env : threads) {
        for (jstack_frame *frame = env->stack.top(); frame; frame = frame->next) {
            t(frame->exp);
            t(frame->lock);

            if ((frame->method->access_flag & jclass_method::ACC_NATIVE) != 0) {
                auto *f = (jni_stack_frame *) frame;
                for (int i = 0, z = f->local_ref_table_size; i < z; ++i) {
                    t(f->local_ref_table[i]);
                }
            }
            else {
                // 此时是 java 函数
                jclass_attr_code *code = frame->method->entrance.code_func;
                auto *f = (java_stack_frame *) frame;

                // 局部变量表
                auto local_variable_table = f->variable_table;
                auto local_ref_table = f->variable_ref_table;
                for (int i = 0, z = code->max_locals; i < z; ++i) {
                    if (local_ref_table[i]) {
                        t(* (jref *) &local_variable_table[i]);
                    }
                }

                // 操作数栈
                auto operand_stack = f->operand_stack;
                auto operand_ref_stack = f->operand_ref_stack;
                for (int i = 0, z = (int) (f->operand_stack - f->operand_stack_orig); i < z; ++i) {
                    if (operand_ref_stack[i]) {
                        t(* (jref *) &operand_stack[i]);
                    }
                }
            }

        }
    }

    // finalize 队列
    for (auto &it : v) {
        t(it);
    }
}


static std::vector<jref> remove_white_objects(std::vector<jref> *pv) noexcept
{
    if (pv->empty()) {
        return {};
    }

    // 双指针法双向遍历，将白色对象移动到队列一端
    size_t l = 0, r = pv->size() - 1;
    auto &v = *pv;

#define color_of(obj) (color_t::MASK & (uint64_t) ((jobject *)obj)->klass)
    while (true) {
        while (l < r && color_of(v[l]) != color_t::WHITE) l ++;
        if (l >= r) break;
        while (l < r && color_of(v[r]) == color_t::WHITE) r --;
        if (l >= r) break;

        auto swap = v[l];
        v[l] = v[r];
        v[r] = swap;
    }

    // 此时左指针一定没有越界
    if (color_of(v[l]) != color_t::WHITE) l ++;
    if (l >= v.size()) return {};

    std::vector<jref> out;
    out.reserve(v.size() - l);

    for (size_t i = l, z = v.size(); i < z; ++ i) {
        out.push_back(v[i]);
    }
    v.resize(l);
    return out;
#undef color_of
}

void gc_thread::mark() noexcept
{
    const auto t0 = std::chrono::steady_clock::now();

    m_alive_memory = 0;
    m_alive_objects.clear();
    std::vector<jobject*> gc_root_queue;
    gc_root_queue.reserve(m_last_gc_root_num + (m_last_gc_root_num >> 1));

    lookup_gc_root(m_finalize_queue, [&gc_root_queue] (jref &ref) {
        if (ref == nullptr) {
            return;
        }
        assert(jheap::R_STR == (jheap::R_MSK & (uint64_t) ref));
        gc_root_queue.push_back((jobject *) ref);
    });
    m_last_gc_root_num = gc_root_queue.size();

    const auto t1 = std::chrono::steady_clock::now();
    LOGI("mark: root node enumeration %lu cost %d ms.\n", m_alive_objects.size(), MS(t1 - t0));

    for (const auto &it : gc_root_queue) {
        trace_and_mark(it);
    }

    // 追踪完所有的 GcRoot 之后，我们就需要遍历堆里的所有 finalize 对象
    // 如果此对象是白色，就需要移动到 finalize 队列，等待 finalizer 线程在空闲时执行
    auto white_finalize_object = remove_white_objects(&m_heap->m_finalize_object);
    for (const auto &it : white_finalize_object) {
        m_finalize_queue.push_back(it);
        trace_and_mark((jobject *) it);
    }

    const auto t2 = std::chrono::steady_clock::now();
    LOGI("mark: trace reference chain cost %d ms, %lu objects alive, %lu bytes reserved\n",
         MS(t2 - t1), m_alive_objects.size(), m_alive_memory);
}


static int compare_pointer(const void *p, const void *q) noexcept
{
    return (int) (*(jobject **) p - *(jobject **) q);
}

void gc_thread::compact() noexcept
{
    const auto t0 = std::chrono::steady_clock::now();

    // 先对所有存活对象按照地址排序
    qsort(&m_alive_objects[0], m_alive_objects.size(),
          sizeof(jobject*), compare_pointer);

    const auto t1 = std::chrono::steady_clock::now();
    LOGI("qsort %lu objects cost %d ms\n", m_alive_objects.size(), MS(t1 - t0));

    // 创建一个临时的映射，用来保存对象在整理前的地址和整理后的地址
    auto mapping = new jobject*[m_alive_objects.size()];
    std::unique_ptr<jobject*, void(*)(jobject**)> mapping_guard(
            mapping, [](jobject** ptr) { delete[] ptr; });

    assert(m_heap != nullptr);
    char *mem_from = m_heap->m_buff;

    // todo: 我们可能要对堆进行扩容/缩容


    m_heap->m_floating_ptr = m_alive_memory;

    int idx = 0;
    for (const auto &src : m_alive_objects) {
        auto object_size = src->size();

        // 复制内存
        memcpy(mem_from, src, object_size);
        mapping[idx ++] = (jobject*) mem_from;
        mem_from += object_size;
    }

    const auto t2 = std::chrono::steady_clock::now();
    LOGI("memcpy cost %d ms\n", MS(t2 - t1));

    // 修改散落在各处的对象引用，klass 指针脱染色
    lookup_gc_root(m_finalize_queue, [&] (jref &ref) {
        trace_and_restore(mapping, ref);
    });

    // finalize 队列
    for (auto &it : m_heap->m_finalize_object) {
        trace_and_restore(mapping, it);
    }

    const auto t3 = std::chrono::steady_clock::now();
    LOGI("trace and update pointer: %d ms\n", MS(t3 - t2));

    // 最后一步，清空 gc_weak
    gc_weak::ref_pool.lookup([&](gc_weak *ptr) {
        ptr->reset(nullptr);
    });
}



#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
void gc_thread::trace_and_restore(jobject **mapping, jref &ref) noexcept
{
    if (ref == nullptr) {
        return;
    }

    auto ref_type = jheap::R_MSK & (uint64_t) ref;
    auto old_object = (jobject *) ((~jheap::R_MSK) & ((uint64_t) ref));

    auto search = (jobject **) bsearch(
            &old_object, &m_alive_objects[0], m_alive_objects.size(),
            sizeof(jobject *), compare_pointer);

    assert(search != nullptr);
    jobject *object = mapping[search - &m_alive_objects[0]];
    int color = (int) (color_t::MASK & (uint64_t) object->klass);

    if (color == color_t::WHITE) {
        return;
    }

    ref = (jref) (ref_type | (uint64_t) object);
    object->klass = (jclass *) (((uint64_t) object->klass) & ~color_t::MASK);

    assert(color == color_t::BLACK || color == color_t::PURPLE);

    if (color == color_t::BLACK) {
        // 如果是黑色，说明这是个对象类型
        for (jclass *cls = object->klass; cls; cls = cls->super_class) {
            for (int i = 0, z = cls->ref_table_size; i < z; ++i) {
                jref &r = *(jref *) (object->values + cls->ref_tables[i]);
                trace_and_restore(mapping, r);
            }
        }
    }
    else {
        // 说明是对象数组. 对象数组要遍历数组中的每一个元素
        int len = 0, ele_size = 0;
        auto array = (jref *) jarray::storage_of(object, &len, &ele_size);
        assert(ele_size == sizeof(jref));

        for (int i = 0; i < len; i ++) {
            trace_and_restore(mapping, array[i]);
        }
    }
}
#pragma clang diagnostic pop
