

#include "bootstrap_loader.h"

#include "../object/jobject.h"
#include "../object/jclass.h"
#include "../object/jmethod.h"
#include "../object/jfield.h"
#include "../vm/jvm.h"
#include "../utils/arrays.h"
#include "../gc/gc_root.h"

#include <sys/stat.h>
#include <vector>
#include <memory>
#include <unistd.h>
#include <set>
#include <map>


#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif 

using namespace javsvm;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
#pragma ide diagnostic ignored "DanglingPointer"


bootstrap_loader::bootstrap_loader(jvm &mem) noexcept :
        m_jvm(mem),
        m_allocator(mem.method_area)
{
}



//static inline std::string trim(const char *name) noexcept
//{
//    std::string name_s(name);
//
//    // 规范 java 类名。如果输入的不是数组类型，末尾又没有 ';'，自动添加 'L' 和 ';'
//    if (name[0] != '[' && name[name_s.size() - 1] != ';') {
//        name_s.insert(0, "L").append(";");
//    }
//    return name_s;
//}







#if 0
jclass *bootstrap_loader::find_class(const char *name) noexcept
{
    std::string name_s = std::move(trim(name));

    {
        std::shared_lock rd_lock(m_lock);

        auto it = m_classes.find(name_s);
        if (it != m_classes.end()) {
            LOGD("class '%s' found in cache, return\n", name);
            return it->second;
        }
    }
    return nullptr;
}
#endif

/**
 * 打开磁盘上的 .class 文件
 */
static jclass_file* open_class_file(const char *dir, const char *name)
{
    std::string s;
    s.append(dir).append("/").append(name).append(".class");

    if (access(s.c_str(), R_OK) != 0) {
        LOGD("failed to open class file '%s'\n", s.c_str());
        return nullptr;
    }

    file_input in(s.c_str());
    in.big_endian();

    auto f = new jclass_file();
    if (f->read_from(in) != 0) {
        delete f;
        return nullptr;
    }
    return f;
}


/**
 * 获取 CLASSPATH 环境变量
 * 如果为空，则返回 "."
 * 返回的是复制后的字符串，因此需要调用者 delete[]
 */
static char* get_classpath()
{
    const char *classpath = getenv("CLASSPATH");
    size_t len;
    if (classpath == nullptr || (len = strlen(classpath)) == 0) {
        classpath = ".";
        len = 1;
    }

    char *buff = new char[len + 1];
    memcpy(buff, classpath, len + 1);

    return buff;
}

/**
 * 遍历 CLASSPATH 环境变量并尝试加载，成功即返回
 * 失败返回 nullptr
 */
static jclass_file* find_class_file(const char *name)
{
    // 准备在 CLASSPATH 环境变量下查找这个文件
    char *classpath = get_classpath();
    std::unique_ptr<char, void (*)(const char *)> buff_guard(classpath,
                        [](const char *p){ delete[] p; });

#ifdef _WIN32
        const char *token = ";";
#else
        const char *token = ":";
#endif

    jclass_file *class_file = nullptr;

    for (char *str = strtok(classpath, token); str; str = strtok(nullptr, token)) {
        struct stat st {  };
        if (stat(str, &st) != 0) {
            // something is wrong ?
        }
        else if (S_ISDIR(st.st_mode)) {
            // 是目录，我们要在它下面寻找这个 class 文件
            class_file = open_class_file(str, name);
        }
        // else if (S_ISREG(st.st_mode)) { // 可能是个 jar，先打开试试
            // class_file = open_jar_file(str, name);
        // }

        if (class_file != nullptr) {
            return class_file;
        }
    }
    return nullptr;
}



/**
 * 尝试打开 jar 中的 .class entry
 */
/*
jclass_file* bootstrap_loader::open_jar_file(const char *jar, const char *name)
{
    zip_file zip;
    if (zip.open(jar, zip_file::Z_RDONLY) != 0) {
        PLOGE("failed to open %s, is this a jar ?\n", jar);
        return nullptr;
    }

    char buff[_MAX_PATH + 1];
    snprintf(buff, _MAX_PATH, "%s.class", name);

    zip_entry *e = zip.find_entry(buff);

    if (e == nullptr) {
        PLOGE("failed to find zip_entry '%s' in zip_file '%s'\n", name, jar);
        return nullptr;
    }

    char *mem = new char[e->get_uncompressed_size()];
    std::unique_ptr<char, void (*)(char*)> mem_guard(mem, 
                        [](char *p) { delete[] p; });

    if (zip.uncompress(e, mem) < 0) {
        PLOGE("failed to extract zip_entry '%s' from zip_file '%s'\n", name, jar);
        return nullptr;
    }

    bytes_input in(mem, e->get_uncompressed_size());
    in.big_endian();
    jclass_file *f = new jclass_file;
    if (f->read_from(in) != 0) {
        delete f;
        return nullptr;
    }
    return f;
}
*/



bool bootstrap_loader::is_primitive_type(const char *name) noexcept
{
    static std::set<std::string> key_set = {
            "boolean", "byte", "char", "short",
            "int", "long", "float", "double"
    };
    return key_set.find(name) != key_set.end();
}

jclass* bootstrap_loader::load_class(const char *name) noexcept
{
    LOGD("load class '%s'...\n", name);
    if (strcmp(name, "sun/nio/cs/StandardCharsets$Aliases") == 0) {
        LOGD("deubg");
    }

//    // 规范 java 类名
//    std::string name_s = std::move(trim(name));
    std::string name_s = name;

    {
        std::shared_lock rd_lock(m_lock);

        auto it = m_classes.find(name_s);
        if (it != m_classes.end()) {
            LOGD("class '%s' found in cache, return\n", name);
            return it->second;
        }
    }

    std::lock_guard wr_lock(m_lock);
    {
        auto it = m_classes.find(name_s);
        if (it != m_classes.end()) {
            LOGD("class '%s' found when double-check, return\n", name);
            return it->second;
        }
    }


    // 如果是数组类型，走另外一条分支
    if (is_primitive_type(name)) {
        return create_primitive_type(name);
    }

    if (name[0] == '[') {
        return load_array_type(name);
    }

    jclass *klass;
    // 先寻找相应的 .class 文件，如果没找到，或者 .class 文件解析失败，返回 nullptr
    jclass_file *cls = find_class_file(name_s.c_str());
    if (cls == nullptr || (klass = prepare_class(cls)) == nullptr) {
        LOGD("can't find class '%s'\n", name_s.c_str());
        return nullptr;
    }

    LOGD("class '%s' load success, store to map\n", name_s.c_str());
    m_classes[name] = klass;

    // 准备创建对应的 java 层对象
    new_class_object(klass);

    LOGD("class '%s' load finish\n", name_s.c_str());
    return klass;
}




static char *get_class_name(jclass_file *cls, int index)
{
    auto class_info = cls->constant_pool.cast<jclass_const_class>(index);
    if (class_info == nullptr) return nullptr;
    auto utf8 = cls->constant_pool.cast<jclass_const_utf8>(class_info->index);
    if (utf8 == nullptr) return nullptr;
    return (char*) utf8->bytes;
}



/**
 * 这个函数用来完成 class 的准备阶段
 * 比如初始化父类，接口类，形成自己的函数表，字段表，虚函数表，并初始化类变量
 */
jclass* bootstrap_loader::prepare_class(jclass_file *cls)
{
    // 对于非 Object 类需要先加载父类
    jclass *super = nullptr;
    if (cls->super_class != 0) {
        char *super_name = get_class_name(cls, cls->super_class);
        super = bootstrap_loader::load_class(super_name);
        if (super == nullptr) { // 父类加载失败
            return nullptr;
        }
    }

    // 加载各个接口
    jclass **interfaces = nullptr;
    std::unique_ptr<jclass*, void(*)(jclass**)> interfaces_guard(
            interfaces, [](jclass **p) { delete[] p; });

    if (cls->interface_count > 0) {
        interfaces = new jclass *[cls->interface_count];
        interfaces_guard.reset(interfaces);

        for (int i = 0; i < cls->interface_count; i++) {
            char *class_name = get_class_name(cls, cls->interfaces[i].name_index);
            interfaces[i] = bootstrap_loader::load_class(class_name);
            if (interfaces[i] == nullptr) { // 接口加载失败
                return nullptr;
            }
        }
    }

    // 到现在为止基本上已经完成了，接下来是准备阶段
    auto klass = m_allocator.calloc_type<jclass>();

    klass->class_file = cls;
    klass->access_flag = cls->access_flag;
    klass->name = get_class_name(cls, cls->this_class);

    // 将父类和接口信息拷贝过去
    klass->super_class = super;
    klass->interface_num = cls->interface_count;
    klass->interfaces = m_allocator.calloc_type<jclass*>(cls->interface_count);
    memcpy(klass->interfaces, interfaces, sizeof(jclass *) * cls->interface_count);

    // 创建继承树
    gen_parent_tree(klass);

    // init_const_pool(klass, cls, mem);
    gen_method_table(klass, cls);
    copy_super_vtable(klass);
    copy_super_itable(klass);
    gen_field_table(klass, cls);
    layout_static_fields(klass);
    layout_direct_fields(klass);
    collect_extra_info(klass);

    return klass;
}

static void collect_interface(jclass *klass, std::set<jclass*> &set)
{
    for (jclass *i = klass; i; i = i->super_class) {
        for (int j = 0; j < i->interface_num; j ++) {
            collect_interface(i->interfaces[j], set);
        }
        set.insert(i);
    }
}

void bootstrap_loader::gen_parent_tree(jclass *klass)
{
    std::set<jclass*> set;
    collect_interface(klass, set);
    set.erase(klass);

    klass->parent_tree_size = (int) set.size();
    klass->parent_tree = m_allocator.calloc_type<jclass*>((int) set.size());

    // 由于 object 类比任何类都先加载，其地址也比其它类要低
    // 如果正向遍历，object 类会出现在第一个位置————我们当然不想这么做，
    // 毕竟没有人会写出 o instanceof Object 这种代码
    int i = (int) set.size() - 1;
    for (auto it : set) {
        klass->parent_tree[i --] = it;
    }
}


/**
 * 遍历 class 中所有的函数，形成自己的函数表
 * jmethod 按照字母顺序排序，用来优化查找性能
 */
void bootstrap_loader::gen_method_table(jclass *klass, jclass_file *cls)
{
    if (cls->method_count == 0) {
        return;
    }

    klass->method_table_size = cls->method_count;
    klass->method_tables = m_allocator.calloc_type<jmethod>(cls->method_count);

//    LOGD("gen_method_table: class '%s'\n", klass->name);

    for (int i = 0; i < klass->method_table_size; i++) {
        klass->method_tables[i].bind(m_allocator, klass, cls, i);
    }

    qsort(klass->method_tables, klass->method_table_size, sizeof(jmethod),
          (int(*)(const void*, const void*)) jmethod::compare_to);
}



struct method_comparator
{
    bool operator()(const jmethod *m1, const jmethod *m2) const noexcept {
        return jmethod::compare_to(m1, m2) < 0;
    }
};

/**
 * 创建虚函数表
 * 虚函数表的生成比较简单，就是先拷贝一份父类的虚函数表，然后遍历当前类的函数表，寻找虚函数。
 * 如果虚函数在父虚函数表中出现过，说明是重载函数，在表中替换掉父类的；反之则添加到虚函数表的最后面。
 * 需要注意的是，虚函数表是无序的，只是为了 invoke-virtual 时随机访问能相当快。无序也就意味着不能使用
 * 二分查找的方式加快搜索速度。
 */
void bootstrap_loader::copy_super_vtable(jclass *klass)
{
    // 接口函数不需要创建虚函数表
    if ((klass->access_flag & jclass_file::ACC_INTERFACE) != 0) {
        return;
    }

    std::set<jmethod*, method_comparator> super_vtable;
    std::vector<jmethod*> vtable;

    // 先将父类的虚函数表放进 set 中，方便快速索引
    auto *super_class = klass->super_class;
    if (super_class != nullptr) {
        int n = super_class->vtable_size;
        vtable.reserve(n + klass->method_table_size);
        for (int i = 0; i < n; i ++) {
            auto *method = super_class->vtable[i];
            super_vtable.insert(method);
            vtable.push_back(method);
        }
    }

    // 一点小小的优化: 如果当前类里没有虚函数，直接继承自父类的虚函数表
    bool has_virtual_method = false;

    // 遍历函数表，寻找虚函数
    for (int i = 0, z = klass->method_table_size; i < z; i ++) {
        auto *method = klass->method_tables + i;
        if (!method->is_virtual) continue;
        has_virtual_method = true;
        auto old = super_vtable.find(method);
        if (old == super_vtable.end()) {
            method->index_in_table = vtable.size();
            vtable.push_back(method);
        }
        else {
            auto idx = (*old)->index_in_table;
            method->index_in_table = idx;
            vtable[idx] = method;
        }
    }
    // 更新 vtable
    if (! has_virtual_method) {
        assert(super_class != nullptr);
        klass->vtable_size = super_class->vtable_size;
        klass->vtable = super_class->vtable;
    }
    else {
        size_t z = vtable.size();
        klass->vtable_size = z;
        klass->vtable = m_allocator.calloc_type<jmethod*>(z);
        memcpy(klass->vtable, &vtable[0], sizeof(jmethod*) * z);
    }
}


/**
 * 创建接口函数表
 *
 * 如果这个类是个接口，事实上我们没有必要为其创建虚函数表和接口函数表
 * 因为在合法的字节码中，来自接口的函数一定是通过 invoke-interface 方式调用的，
 * 这种方式会在其 *实现类（一定是非抽象类）* 中查询接口函数表，*接口类* 中的虚函数表和接口函数表
 * 实际上都没有用到。但我们还是选择性地为其创建了接口函数表，仅仅是为了加载非抽象类时能更快一些。
 *
 * 需要注意的是，如果子接口和父接口中含有相同的函数，按照以下逻辑逻辑：
 * 1. 如果子接口和父接口的函数都是抽象的（非 default 接口），子类覆盖父类的
 * 2. 如果子接口和父接口的函数都不是抽象的（default 接口），子类覆盖父类的
 * 3. default 接口函数覆盖非 default 接口函数
 *
 * 对于非接口类，相比接口类更加复杂，此时不仅要继承自父类的接口函数表，还要将自己的虚函数
 * 替换掉父接口函数表的相同项（毕竟实现变了函数指针就要变），最后再引入自己的接口表。
 */
void bootstrap_loader::copy_super_itable(jclass *klass) noexcept
{
#define IS_ABSTRACT(x) (((x) & jclass_method::ACC_ABSTRACT) != 0)
#define IS_INTERFACE(x) (((x) & jclass_file::ACC_INTERFACE) != 0)

    // 我们并不使用 std::set，而是 std::map, 原因是 std::set
    // 插入重复元素时，新的 value 不会替换掉老的 value。但我们又确实依赖这个机制
    // 更新接口函数表
    std::map<jmethod*, jmethod*, method_comparator> itable;

    if (IS_INTERFACE(klass->access_flag)) {
        // 遍历所有的父接口
        for (int i = 0, z = klass->interface_num; i < z; i ++) {
            auto *interface = klass->interfaces[i];
            for (int j = 0, y = interface->itable_size; j < y; j ++) {
                auto *method = interface->itable[j];

                const auto &it = itable.find(method);
                auto *old = it->second;

                if (it == itable.end() || IS_ABSTRACT(old->access_flag)) {
                    itable[method] = method;
                }
                else if (!IS_ABSTRACT(method->access_flag)) {
                    /**
                     * 坏了，进入这个分支就说明从父接口处找到了两个一毛一样的 default 函数
                     * 比如下面的示例代码:
                     * interface A { default void run() { System.out.println("A->run"); } }
                     * interface B extends A { default void run() { System.out.println("B->run"); } }
                     * interface C extends A, B {}
                     * 在加载接口 C 时，会在 A 的接口函数表和 B 的接口函数表中找到两个一模一样的 run() 函数。
                     * 此时我们要做些额外工作，决定哪个函数要被替换掉。
                     * 在上面的例子中，来自 B 接口的 B->run()V 将会替换掉 A->run()V，因为 B 相比于 A 在继承链
                     * 更前面的位置。事实上我们可以推断出，对于这两个函数 runA 和 runB，它们所属的类 class A 和 class B，
                     * 一定有着非此即彼的继承关系——要么 A 继承自 B，要么 B 继承自 A，没有其它可能(javac 会拒绝编译)。
                     */
                    if (old->clazz->is_assign_from(method->clazz)) { // [1].
                        itable[method] = method;
                        /**
                         * [1]. 注意是 method->clazz 而不是 interface. 比如再添加一个类
                         * interface D implements A, C { }
                         * 在加载 class C 时，先经过一次判决，留在 class C 的接口函数表中的 run 是 B->run()V.
                         * 加载 class D 时还要经过一次判决，此时应该判断 B 和 A 的继承关系，而不是 C 和 A 的.
                         */
                    }
                    else {
                        // 断言二者之间一定存在继承关系.
                        assert(method->clazz->is_assign_from(old->clazz));
                    }
                }
            }
        }
        // 遍历自己的函数表，寻找虚函数
        for (int i = 0, z = klass->method_table_size; i < z; i ++) {
            auto *method = klass->method_tables + i;
            if (!method->is_virtual) continue;

            const auto &it = itable.find(method);
            auto *old = it->second;
            if (it == itable.end() || IS_ABSTRACT(old->access_flag) || ! IS_ABSTRACT(method->access_flag)) {
                itable[method] = method;
            }
        }

        // 更新 itable
        {
            int i = 0;
            klass->itable_size = itable.size();
            klass->itable = m_allocator.calloc_type<jmethod*>(itable.size());
            for (const auto &it : itable) {
                klass->itable[i ++] = it.second;
            }
        }
        return;
    }

    // 先继承父类的接口函数表
    jclass *super_class = klass->super_class;
    if (UNLIKELY(super_class == nullptr)) {
        // java/lang/Object 类不实现任何接口
        return;
    }

    for (int i = 0, z = super_class->itable_size; i < z; ++i) {
        auto m = super_class->itable[i];
        itable[m] = m;
    }

    bool itable_changed = false;

    // 遍历所有接口，将新增接口函数添加到 itable 中
    for (int i = 0, z = klass->interface_num; i < z; ++i) {
        auto *interface = klass->interfaces[i];
        for (int j = 0, y = interface->itable_size; j < y; ++j) {
            auto *method = interface->itable[j];

            const auto &it = itable.find(method);
            auto *old_method = it->second;

            // 如果是新增的函数，直接插入进去
            if (it == itable.end()) {
                itable_changed = true;
                itable[method] = method;
                continue;
            }
            // 如果老函数来源于非接口类，不管该接口是不是 default 函数，都不执行插入操作
            if (!IS_INTERFACE(old_method->clazz->access_flag)) {
                continue;
            }
            // 现在可以确定，老函数也是个接口函数。接下来需要比较是否是 default 函数
            // 如果老函数是抽象的，直接替换掉
            if (IS_ABSTRACT(old_method->access_flag)) {
                itable_changed = true;
                itable[method] = method;
                continue;
            }
            // 执行到这里说明老函数是 default 函数。如果我们不是 default 函数，不替换
            if (IS_ABSTRACT(method->access_flag)) {
                continue;
            }
            // 两个函数都是 default 函数，和上面的一样
            if (old_method->clazz->is_assign_from(method->clazz)) {
                itable_changed = true;
                itable[method] = method;
                continue;
            }
            // 断言二者之间一定存在继承关系.
            assert(method->clazz->is_assign_from(old_method->clazz));
        }
    }

    // 遍历自己的虚函数，替换掉父接口函数表的同类项
    for (int i = 0, z = klass->method_table_size; i < z; ++i) {
        auto *m = klass->method_tables + i;
        if (! m->is_virtual) continue;
        const auto &it = itable.find(m);
        if (it != itable.end()) {
            itable_changed = true;
            itable[m] = m;
        }
    }
    // 更新 itable
    if (! itable_changed) {
        klass->itable_size = super_class->itable_size;
        klass->itable = super_class->itable;
    }
    else {
        int i = 0;
        klass->itable_size = itable.size();
        klass->itable = m_allocator.calloc_type<jmethod*>(itable.size());
        for (const auto &it : itable) {
            klass->itable[i ++] = it.second;
        }
    }
#undef IS_INTERFACE
#undef IS_ABSTRACT
}


/**
 * 遍历所有字段，建立自己的字段表，并按照名字排序以加快符号搜索速度
 * 注：字段不参与重载，因此每个类的字段都是属于自己的
 */
void bootstrap_loader::gen_field_table(jclass *klass, jclass_file *cls)
{
    if (cls->field_count == 0) {
        return;
    }

    klass->field_table_size = cls->field_count;
    klass->field_tables = m_allocator.calloc_type<jfield>(cls->field_count);

    for (int i = 0; i < klass->field_table_size; i++) {
        klass->field_tables[i].bind(klass, cls, i);
    }
    qsort(klass->field_tables, klass->field_table_size, sizeof(jfield),
          (int (*)(const void*, const void*)) jfield::compare_to);
}

/**
 * 遍历 class 中所有 static 字段，并按照一定的顺序摆放在一起
 * 摆放的原则是相同字节大小的放到一起，先大后小
 */ 
void bootstrap_loader::layout_static_fields(jclass *klass)
{
    if (klass->field_table_size == 0) {
        return;
    }

    // 遍历类中所有的 static 变量
    std::vector<jfield *> u1, u2;
    std::vector<jfield *> u4, u8;

    for (int i = 0; i < klass->field_table_size; i++) {
        auto it = klass->field_tables + i;
        
        if ((it->access_flag & jclass_field::ACC_STATIC) == 0) {
            continue;
        }
        switch (it->mem_size) {
            case 1: u1.push_back(it); break;
            case 2: u2.push_back(it); break; 
            case 4: u4.push_back(it); break;
            case 8: u8.push_back(it); break;
        }
    }

    // 摆放时把相同大小的字段放到一起，顺序为降序
    int size = 0;
    for (auto it : u8) { it->mem_offset = size; size += 8; }
    for (auto it : u4) { it->mem_offset = size; size += 4; }
    for (auto it : u2) { it->mem_offset = size; size += 2; }
    for (auto it : u1) { it->mem_offset = size; size += 1; }

    // 8 字节对齐
    // size = ((size - 1) | 7) + 1;
    klass->data = m_allocator.calloc_type<char>(size);
    memset(klass->data, 0, size);
}



//struct bootstrap_loader::vector_queue
//{
//    using v_type = std::vector<jfield *>;
//
//    vector_queue(v_type &_u1, v_type &_u2, v_type &_u4, v_type &_u8)
//    {
//        table[0] = &_u1;
//        table[1] = &_u2;
//        table[2] = &_u4;
//        table[3] = &_u8;
//
//        size = _u1.size();
//    }
//
//    v_type* table[4];
//    int index = 0;
//    int size;
//
//    jfield *get(int *p_size)
//    {
//        while (size == 0) {
//            if (index == 3) return nullptr;
//            size = table[++ index]->size();
//        }
//        auto it = table[index]->operator[](size - 1);
//        if (p_size) *p_size = 1 << index;
//        return it;
//    }
//
//    jfield *next(int *p_size)
//    {
//        auto it = get(p_size);
//        if (it) {
//            size --;
//            table[index]->pop_back();
//        }
//        return it;
//    }
//};



/**
 * 遍历 static 中所有的非 static 字段，并按照顺序摆放在一起
 * 摆放时要求放在父类非静态字段的后面。
 */ 
void bootstrap_loader::layout_direct_fields(jclass *klass)
{
    int size = klass->super_class ? align<8>(klass->super_class->object_size) : 0;

    if (klass->field_table_size == 0) {
        klass->object_size = size;
        return;
    }

    // 遍历类中所有的非 static 变量
    std::vector<jfield *> u1, u2;
    std::vector<jfield *> u4, u8;

    for (int i = 0; i < klass->field_table_size; i++) {
        auto it = klass->field_tables + i;
        if ((it->access_flag & jclass_field::ACC_STATIC) != 0) {
            continue;
        }
        switch (it->mem_size) {
            case 1: u1.push_back(it); break;
            case 2: u2.push_back(it); break; 
            case 4: u4.push_back(it); break;
            case 8: u8.push_back(it); break;
        }
    }

//    // 当父对象大小不满足 8 字节对齐时，可以插入一些小字节的字段
//
//    vector_queue queue(u1, u2, u4, u8);
//    while ((size & 7) != 0) {
//        int padding_size = 4 - (size & 3), item_size;
//        jfield *it = queue.get(&item_size);
//        if (it == nullptr) {                // 所有的字段都已经摆放下了 ?
//            klass->object_size = size;
//            return;
//        }
//        else if (padding_size < item_size) { // 剩余空间不足以存放这个字段
//            size += padding_size;
//        }
//        else {                              // 可以存放的下，下一个
//            it->mem_offset = size;
//            size += item_size;
//            queue.next(&item_size);
//        }
//    }

    // 剩余的字段按照所占空间大小放到一起，字节大的优先
    for (auto it : u8) { it->mem_offset = size; size += 8; }
    for (auto it : u4) { it->mem_offset = size; size += 4; }
    for (auto it : u2) { it->mem_offset = size; size += 2; }
    for (auto it : u1) { it->mem_offset = size; size += 1; }

    // 8 字节对齐
    // size = ((size - 1) | 7) + 1;
    klass->object_size = size;
}


static inline jmethod* find_finalize(jclass *klass) noexcept
{
    jmethod m = { .name = "finalize", .sig = "()V" };
    using cmp_t = int(*)(const void*, const void*);
    return (jmethod *) bsearch(&m,klass->method_tables,
                          klass->method_table_size,sizeof(jmethod),
                          (cmp_t) jmethod::compare_to);
}

void bootstrap_loader::collect_extra_info(jclass *klass) noexcept
{
    // 虚拟机应当知道对象的哪个位置存放着引用
    std::vector<int> ref_table;
    ref_table.reserve(klass->field_table_size);

    for (int i = 0, z = klass->field_table_size; i < z; i ++) {
        auto *field = klass->field_tables + i;
        if (field->sig[0] != 'L' && field->sig[0] != '[') {
            continue;
        }
        if ((field->access_flag & jclass_field::ACC_STATIC) == 0) {
            ref_table.push_back(field->mem_offset);
        }
        else {
            // 静态字段作为 GcRoot
            gc_root::static_field_pool.add((jref*) (klass->data + field->mem_offset));
        }
    }
    {
        int z = (int) ref_table.size();
        klass->ref_tables = m_allocator.calloc_type<int>(z);
        klass->ref_table_size = z;
        memcpy(klass->ref_tables, &ref_table[0], z * sizeof(int));
    }

    // 判断此类是否重写了 Ljava/lang/Object->finalize()V,
    // 堆和 gc 需要这个标志做特殊处理
    if ((klass->access_flag & jclass_file::ACC_INTERFACE) == 0) {
        jclass *super = klass->super_class;
        if (super == nullptr) {
            // nothing to do
        }
        else if ((super->access_flag & jclass_file::ACC_FINALIZE) != 0) {
            // 父类重写了，子类理所当然也要重写
            klass->access_flag |= jclass_file::ACC_FINALIZE;
        }
        else if (find_finalize(klass) != nullptr) {
            // 如果函数表里找到了 finalize(), 添加 flag
            klass->access_flag |= jclass_file::ACC_FINALIZE;
        }
    }
}




jref bootstrap_loader::new_class_object(jclass *klass)
{
    static jclass *java_lang_Class = nullptr;
    static jmethod *java_lang_Class_init = nullptr;

    /**
     * 如果 java/lang/Object 已经初始化完成，则直接调用 new java.lang.Class() 的方式构造 Class 对象
     */
    if (m_has_full_object_class) { // [1]
        // 不用检查空指针，因为 java/lang/Object 初始化时会检查
        LOGD("java/lang/Object is inited, new instance directly\n");
        jref ref = java_lang_Class->new_instance(java_lang_Class_init, (jlong) klass);
        klass->object = ref;
        return ref;
    }
    /**
     * 执行到这里说明 java/lang/Object 类正在初始化。我们只需要把自己加入到初始化队列，等待 Object 类帮我们初始化即可
     */
    if (strcmp(klass->name, "java/lang/Object") != 0) { // [2]
        // 同样不需要检查空指针
        LOGD("java/lang/Object is doing init, just place in queue\n");
        ((std::vector<jclass*> *) m_uninitialized_class_queue)->push_back(klass);
        return nullptr;
    }
    // 万恶之源，Object 竟是我自己
    // 先尝试加载下 java/lang/Class 类。在 Class 类加载的过程中，先尝试加载 java/lang/Object，命中缓存，直接返回；
    // 接下来就是 Class 实现的那些接口，比如 Type。Type 类去加载 Object 类，也是直接返回，然后尝试为 Type 类建立 java 层的 Class
    // 对象，命中分支 [2]，返回；最后是为 Class 类建立 java 层对象，同样命中 [2]，然后 Class 类返回。
    LOGD("I'm java/lang/Object, init ...\n");

    std::vector<jclass*> uninitialized_class_queue;
    uninitialized_class_queue.push_back(klass);
    m_uninitialized_class_queue = &uninitialized_class_queue;

    java_lang_Class = load_class("java/lang/Class");
    if (java_lang_Class == nullptr) {
        PLOGE("failed to load java/lang/Class, abort\n");
        exit(1);
    }

    java_lang_Class_init = java_lang_Class->get_method("<init>", "(J)V");
    if (java_lang_Class_init == nullptr) {
        LOGE("failed to find method: Ljava/lang/Class-><init>((J)V)\n");
        exit(1);
    }
    // 接下来就需要遍历队列，为每个类创建 class 对象
    m_uninitialized_class_queue = nullptr;
    m_has_full_object_class = true;

    for (auto it : uninitialized_class_queue) {
        jref ref = java_lang_Class->new_instance(java_lang_Class_init, (jlong) klass);
        it->object = ref;
    }
    return klass->object.get();
}





struct
{
    jclass *java_lang_Object = nullptr;
    jclass *java_lang_Cloneable = nullptr;
    jclass *java_io_Serializable = nullptr;
    jmethod *java_lang_Object_clone = nullptr;

    void ensure_classes_loaded(bootstrap_loader *loader) noexcept
    {
        if (java_lang_Object != nullptr) {
            return;
        }

        java_lang_Object = loader->load_class("java/lang/Object");
        java_lang_Cloneable = loader->load_class("java/lang/Cloneable");
        java_io_Serializable = loader->load_class("java/io/Serializable");
        java_lang_Object_clone = java_lang_Object->get_method("clone", "()Ljava/lang/Object;");
    }
} class_holder;

jclass* bootstrap_loader::create_primitive_type(const std::string &type) noexcept
{
    LOGD("create primitive type '%s'\n", type.c_str());

    char *name = m_allocator.calloc_type<char>(type.size() + 1);
    memcpy(name, type.c_str(), type.size() + 1);

    auto *klass = m_allocator.calloc_type<jclass>();
    klass->name = name;
    klass->clinit = jclass::INIT_DONE;
    klass->access_flag = jclass_file::ACC_PUBLIC | jclass_file::ACC_ABSTRACT | jclass_file::ACC_FINAL;

    new_class_object(klass);
    m_classes[type] = klass;

    // 不为基本数据类型添加构造函数和类构造函数
    LOGD("primitive type '%s' created\n", type.c_str());
    return klass;
}

jclass *bootstrap_loader::load_array_type(const std::string &name)
{
    const char *type = name.c_str();
    LOGD("loading array type '%s'\n", type);

    // 数组要包裹的类型
    jclass *component_type = nullptr;

#define FIND_OR_CREATE_PRIMITIVE_TYPE(NAME) \
    auto it = m_classes.find(NAME);         \
    if (it == m_classes.end()) {            \
        component_type = create_primitive_type(NAME); \
    }                                       \
    else {                                  \
        component_type = it->second;        \
    }

    // 创建最内层的包裹类
    const auto index = name.find_last_of('[') + 1; // type_s.find_last_not_of('[');

    switch (name[index]) {
        case 'Z': { FIND_OR_CREATE_PRIMITIVE_TYPE("boolean")    break; }
        case 'B': { FIND_OR_CREATE_PRIMITIVE_TYPE("byte")       break; }
        case 'C': { FIND_OR_CREATE_PRIMITIVE_TYPE("char")       break; }
        case 'S': { FIND_OR_CREATE_PRIMITIVE_TYPE("short")      break; }
        case 'I': { FIND_OR_CREATE_PRIMITIVE_TYPE("int")        break; }
        case 'J': { FIND_OR_CREATE_PRIMITIVE_TYPE("long")       break; }
        case 'F': { FIND_OR_CREATE_PRIMITIVE_TYPE("float")      break; }
        case 'D': { FIND_OR_CREATE_PRIMITIVE_TYPE("double")     break; }
        case 'L': {
            std::string sub = name.substr(index + 1, name.size() - index - 2);
            component_type = load_class(sub.c_str());
            break;
        }
    }
#undef FIND_OR_CREATE_PRIMITIVE_TYPE

    if (component_type == nullptr) {
        LOGE("failed to create component_type '%s' of array '%s'\n", type + index, type);
        exit(1);
    }
    LOGD("the component type of array '%s' is '%s'\n", type, component_type->name);

    class_holder.ensure_classes_loaded(this);

    // 从最内层开始遍历，逐渐生成每个数组类
    for (int i = (int) index - 1; i >= 0; --i) {
        // 检查缓存，如果发现该数组类已经被加载过，忽略掉
        auto it = m_classes.find(type + i);
        if (it != m_classes.end()) {
            LOGD("[%d/%lu]: type '%s' of array '%s' found in cache, continue\n", i + 1, index, type + i, type);
            continue;
        }
        LOGD("[%d/%lu]: create type '%s' of array '%s'\n", i + 1, index, type + i, type);
        auto *klass = create_primitive_type(type + i);
        klass->super_class = class_holder.java_lang_Object;
        klass->component_type = component_type;

        // 数组类型要实现 java.io.Serializable 和 java.lang.Cloneable 接口
        // 但不需要真正创建 jmethod
        klass->interface_num = 2;
        klass->interfaces = m_allocator.calloc_type<jclass*>(2);
        klass->interfaces[0] = class_holder.java_io_Serializable;
        klass->interfaces[1] = class_holder.java_lang_Cloneable;

        // 虚函数表
        klass->vtable_size = class_holder.java_lang_Object->vtable_size;
        klass->vtable = class_holder.java_lang_Object->vtable;

        // 创建继承树
        klass->parent_tree_size = 3;
        klass->parent_tree = m_allocator.calloc_type<jclass*>(3);
        klass->parent_tree[0] = class_holder.java_lang_Object;
        klass->parent_tree[1] = class_holder.java_io_Serializable;
        klass->parent_tree[2] = class_holder.java_lang_Cloneable;

//        new_class_object(klass);
        m_classes[type + i] = klass;
        component_type = klass;
    }
    LOGD("load array type '%s' finish, result is '%s'\n", name.c_str(), component_type->name);
    return component_type;
}

#pragma clang diagnostic pop
