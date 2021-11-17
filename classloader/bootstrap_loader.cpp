

#include "bootstrap_loader.h"

#include "../object/jobject.h"
#include "../object/jclass.h"
#include "../object/jmethod.h"
#include "../object/jfield.h"
#include "../vm/jvm.h"
#include "../utils/array_utils.h"

#include <sys/stat.h>
#include <vector>
#include <shared_mutex>
#include <memory>
#include <unistd.h>
#include <set>

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif 

using namespace javsvm;



bootstrap_loader::bootstrap_loader(jvm &mem) noexcept :
    m_allocator(mem.method_area)
{
}


jclass* bootstrap_loader::load_class(const char *name)
{
    LOGI("load class '%s'...\n", name);
    std::string name_s(name);

    // 规范 java 类名。如果输入的不是数组类型，末尾又没有 ';'，自动添加 'L' 和 ';'
    if (name[0] != '[' && name[name_s.size() - 1] != ';') {
        name_s.insert(0, "L").append(";");
    }

    {
        std::shared_lock rd_lock(m_lock);

        auto it = m_classes.find(name_s);
        if (it != m_classes.end()) {
            LOGI("class '%s' found in cache, return\n", name);
            return it->second;
        }
    }

    std::lock_guard wr_lock(m_lock);
    {
        auto it = m_classes.find(name_s);
        if (it != m_classes.end()) {
            LOGI("class '%s' found when double-check, return\n", name);
            return it->second;
        }
    }

    // 如果是数组类型，走另外一条分支
    if (name[0] == '[') {
        return load_array_type(name_s);
    }

    // 先寻找相应的 .class 文件，如果没找到，或者 .class 文件解析失败，返回 nullptr
    jclass_file *cls = find_class(name_s.c_str());
    if (cls == nullptr) {
        LOGI("can't find class '%s'\n", name);
        return nullptr;
    }

    // 既然 .class 文件解析成功，接下来就没有回头路了。只要失败，整个虚拟机直接退出。
    // 虽然当前的类 X 加载失败，但其父类，父父类都可能加载成功而进入缓存池，我们不希望这些
    // 数据影响下一次加载过程。另外，jmethod_area 将已分配的内存回收进内存池目前也是没有实现的。
    jclass *klass = prepare_class(cls);
    if (klass == nullptr) {
        PLOGE("failed to load class '%s'，restore memory pointer\n", name);
        exit(1);
    }

    LOGI("class '%s' load success, store to map\n", name);
    m_classes[name_s] = klass;

    // 准备创建对应的 java 层对象
    new_class_object(klass);

    LOGI("class '%s' load finish\n", name);
    return klass;
}


/**
 * 打开磁盘上的 .class 文件
 */
static jclass_file* open_class_file(const char *dir, const char *name)
{
    std::string s;
    s.append(dir).append("/").append(name, 1, strlen(name) - 2).append(".class");

    if (access(s.c_str(), R_OK) != 0) {
        LOGE("failed to open class file '%s'\n", s.c_str());
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
 * 返回值是一个原始的字符串，因此不需要 delete[]
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
jclass_file* bootstrap_loader::find_class(const char *name)
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
    gen_field_table(klass, cls);
    layout_static_fields(klass);
    layout_direct_fields(klass);
    // init_const_field(klass, cls);

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

    klass->parent_tree_size = (int) set.size();
    klass->parent_tree = m_allocator.calloc_type<jclass*>((int) set.size());

    int i = 0;
    for (auto it : set) {
        klass->parent_tree[i ++] = it;
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

    for (int i = 0; i < klass->method_table_size; i++) {
        klass->method_tables[i].bind(klass, cls, i);
    }

    qsort(klass->method_tables, klass->method_table_size, sizeof(jmethod),
          (int(*)(const void*, const void*)) jmethod::compare_to);

    for (int i = 0; i < klass->method_table_size; i ++) {
        klass->method_tables[i].index_in_table = i;
    }
}

/**
 * 拷贝父类的虚函数表，并将覆盖掉的函数指向自己 
 */
void bootstrap_loader::copy_super_vtable(jclass *klass)
{
    std::vector<jmethod*> table;
    jclass *super_class = klass->super_class;

    // 复制从父类继承过来的虚函数表
    if (super_class != nullptr) {
        int z = super_class->vtable_size;
        table.resize(z);
        memcpy(&table[0], super_class->vtable, z * sizeof(jmethod *));
    }

    // 遍历所有的虚函数，并添加到虚函数表
    for (int i = 0, z = klass->method_table_size; i < z; i++) {
        auto it = klass->method_tables + i;
        if (! it->is_virtual) {
            continue;
        }
        // 二分查找，判断该虚函数是否出现在了父表中
        // 因为虚函数表的建立是在函数表建立之后，因此也是有序的
        int index = -1;
        if (super_class != nullptr) {
             int (*cmp)(jmethod* const*, jmethod* const*) = [](jmethod* const *p, jmethod* const *q) -> int {
                return jmethod::compare_to(*p, *q);
            };
            index = array_utils::binary_search<jmethod*>(it, &table[0], super_class->vtable_size, cmp);
        }
        if (index == -1) {
            // 新增的虚函数，添加到父虚函数表的后面
            it->index_in_table = (int) table.size();
            table.push_back(it);
        }
        else {
            // 重写父类的虚函数，覆盖在相应位置
            table[index] = it;
        }
    }

    // 生成新的虚函数表
    int z = (int) table.size();
    klass->vtable_size = z;
    klass->vtable = m_allocator.calloc_type<jmethod *>(z);
    memcpy(klass->vtable, &table[0], sizeof(jmethod*) * z);
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
    klass->class_size = size;
    klass->data = m_allocator.malloc_bytes(size);
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


static int align_8(int size)
{
    return ((size - 1) | 7) + 1;
}

/**
 * 遍历 static 中所有的非 static 字段，并按照顺序摆放在一起
 * 摆放时要求放在父类非静态字段的后面。
 */ 
void bootstrap_loader::layout_direct_fields(jclass *klass)
{
    int size = klass->super_class ? align_8(klass->super_class->object_size) : 0;

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



//static jvalue get_constant_value(jclass_attr_constant *attr, jclass_const_pool &pool)
//{
//    jclass_const *const_value = pool.child_at(attr->constant_value_index - 1);
//    jvalue value = {0};
//
//    switch (const_value->tag) {
//        case jclass_const_int::TAG: {
//            u4 i = ((jclass_const_int *)const_value)->bytes;
//            value.i = *(int *)&i;
//            break;
//        }
//        case jclass_const_float::TAG: {
//            u4 f = ((jclass_const_float *)const_value)->bytes;
//            value.f = *(jfloat *)&f;
//            break;
//        }
//        case jclass_const_long::TAG: {
//            u8 l = ((::jclass_const_long *)const_value)->bytes;
//            value.j = *(jlong *)&l;
//            break;
//        }
//        case jclass_const_double::TAG: {
//            u8 d = ((jclass_const_double *)const_value)->bytes;
//            value.d = *(jdouble *)&d;
//            break;
//        }
//        case jclass_const_string::TAG: {
//            // 这里比较复杂，因为要创建出一个新的 java.lang.String 对象
//            auto *s = ((jclass_const_string *) attr);
//            auto *utf8 = pool.cast<jclass_const_utf8>(s->index);
//            value.l = jvm::get().string_pool().find_or_new((char *)utf8->bytes);
//            break;
//        }
//        default:
//            LOGE("unknown const_value_tag %d\n", const_value->tag);
//            break;
//    }
//    return value;
//}


/**
 * 遍历 class 中所有 static 字段，如果是常量则初始化
 */ 
//void bootstrap_loader::init_const_field(jclass *klass)
//{
//    jclass_const_pool &pool = klass->class_file->constant_pool;
//
//    for (int i = 0; i < klass->field_table_size; i++) {
//        jfield *it = klass->field_tables + i;
//        jclass_field *src = it->orig;
//
//        for (int j = 0, z = src->attributes_count; j < z; j++) {
//            auto attr = src->attributes[j]->cast<jclass_attr_constant>();
//            if (attr != nullptr) {
//                //todo: 设置常量值
////                it->set(nullptr, get_constant_value(attr, pool));
//                break;
//            }
//        }
//    }
//}


jref bootstrap_loader::new_class_object(jclass *klass)
{
    static jclass *java_lang_Class = nullptr;
    static jmethod *java_lang_Class_init = nullptr;

    /**
     * 如果 java/lang/Object 已经初始化完成，则直接调用 new java.lang.Class() 的方式构造 Class 对象
     */
    if (m_has_full_object_class) { // [1]
        // 不用检查空指针，因为 java/lang/Object 初始化时会检查
        LOGI("java/lang/Object is inited, new instance directly\n");
        jref ref = java_lang_Class->new_instance(java_lang_Class_init, (jlong) klass);
        klass->object = ref;
        return ref;
    }
    /**
     * 执行到这里说明 java/lang/Object 类正在初始化。我们只需要把自己加入到初始化队列，等待 Object 类帮我们初始化即可
     */
    if (strcmp(klass->name, "java/lang/Object") != 0) { // [2]
        // 同样不需要检查空指针
        LOGI("java/lang/Object is doing init, just place in queue\n");
        ((std::vector<jclass*> *) m_uninitialized_class_queue)->push_back(klass);
        return nullptr;
    }
    // 万恶之源，Object 竟是我自己
    // 先尝试加载下 java/lang/Class 类。在 Class 类加载的过程中，先尝试加载 java/lang/Object，命中缓存，直接返回；
    // 接下来就是 Class 实现的那些接口，比如 Type。Type 类去加载 Object 类，也是直接返回，然后尝试为 Type 类建立 java 层的 Class
    // 对象，命中分支 [2]，返回；最后是为 Class 类建立 java 层对象，同样命中 [2]，然后 Class 类返回。
    LOGI("I'm java/lang/Object, init ...\n");

    std::vector<jclass*> uninitialized_class_queue;
    uninitialized_class_queue.push_back(klass);
    m_uninitialized_class_queue = &uninitialized_class_queue;

    java_lang_Class = load_class("java/lang/Class");
    if (java_lang_Class == nullptr) {
        PLOGE("failed to load java/lang/Class, abort\n");
        exit(1);
    }

    java_lang_Class_init = java_lang_Class->get_method("<init>", "()V");
    if (java_lang_Class_init == nullptr) {
        LOGE("failed to find method: Ljava/lang/Class-><init>(()V)\n");
        exit(1);
    }
    // 接下来就需要遍历队列，为每个类创建 class 对象
    m_uninitialized_class_queue = nullptr;
    m_has_full_object_class = true;

    for (auto it : uninitialized_class_queue) {
        jref ref = java_lang_Class->new_instance(java_lang_Class_init, (jlong) klass);
        it->object = ref;
    }
    return klass->object;
}





struct class_holder
{
    jclass *java_lang_Object = nullptr;
    jclass *java_lang_Class = nullptr;
    jmethod *java_lang_Class_init = nullptr;
    jclass *java_lang_Cloneable = nullptr;
    jclass *java_io_Serializable = nullptr;

#define _CHECK_NULL(x)  \
    if (x == nullptr) { \
        LOGE("field '%s' of class_holder is null\n", #x); \
        exit(1);        \
    }

    explicit class_holder(bootstrap_loader &loader) noexcept
    {
        java_lang_Object = loader.load_class("java/lang/Object");
        java_lang_Class = loader.load_class("java/lang/Class");
        java_lang_Cloneable = loader.load_class("java/lang/Cloneable");
        java_io_Serializable = loader.load_class("java/io/Serializable");

        _CHECK_NULL(java_lang_Object)
        _CHECK_NULL(java_lang_Class)
        _CHECK_NULL(java_lang_Cloneable)
        _CHECK_NULL(java_io_Serializable)

        java_lang_Class_init = java_lang_Class->get_method("<init>", "()V");
        _CHECK_NULL(java_lang_Class_init)
    }
#undef _CHECK_NULL
};

static class_holder &get_class_holder(bootstrap_loader *loader) {
    static class_holder instance(*loader);
    return instance;
}


jclass* bootstrap_loader::create_primitive_type(const char *type)
{
    LOGI("create primitive type '%s'\n", type);

    int name_len = (int) strlen(type);
    char *name = m_allocator.calloc_type<char>(name_len + 1);
    memcpy(name, type, name_len + 1);

    auto &holder = get_class_holder(this);

    auto *klass = m_allocator.calloc_type<jclass>();
    klass->name = name;
    klass->access_flag = jclass_file::ACC_PUBLIC | jclass_file::ACC_ABSTRACT | jclass_file::ACC_FINAL;
    klass->object = holder.java_lang_Class->new_instance();

    // 不为基本数据类型添加构造函数和类构造函数
    LOGI("primitive type '%s' created\n", type);
    return klass;
}


jclass *bootstrap_loader::load_array_type(const std::string &type_s)
{
    const char *type = type_s.c_str();
    LOGI("loading array type '%s'\n", type);

    // 数组要包裹的类型
    jclass *component_type = nullptr;

#define FIND_OR_CREATE_PRIMITIVE_TYPE(NAME) \
    auto it = m_classes.find(NAME);         \
    if (it == m_classes.end()) {            \
        component_type = create_primitive_type(NAME); \
        m_classes[NAME] = component_type;   \
    }                                       \
    else {                                  \
        component_type = it->second;        \
    }

    // 创建最内层的包裹类
    const int index = (int) type_s.find_last_of('[') + 1; // type_s.find_last_not_of('[');

    switch (type[index]) {
        case 'Z': { FIND_OR_CREATE_PRIMITIVE_TYPE("boolean")    break; }
        case 'B': { FIND_OR_CREATE_PRIMITIVE_TYPE("type")       break; }
        case 'C': { FIND_OR_CREATE_PRIMITIVE_TYPE("char")       break; }
        case 'S': { FIND_OR_CREATE_PRIMITIVE_TYPE("short")      break; }
        case 'I': { FIND_OR_CREATE_PRIMITIVE_TYPE("int")        break; }
        case 'J': { FIND_OR_CREATE_PRIMITIVE_TYPE("long")       break; }
        case 'F': { FIND_OR_CREATE_PRIMITIVE_TYPE("float")      break; }
        case 'D': { FIND_OR_CREATE_PRIMITIVE_TYPE("double")     break; }
        default: {
            component_type = load_class(type + index);
            break;
        }
    }
#undef FIND_OR_CREATE_PRIMITIVE_TYPE

    if (component_type == nullptr) {
        LOGE("failed to create component_type '%s' of array '%s'\n", type + index, type);
        exit(1);
    }
    LOGI("the component type of array '%s' is '%s'\n", type, component_type->name);

    auto &holder = get_class_holder(this);

    // 从最内层开始遍历，逐渐生成每个数组类
    for (int i = index - 1; i >= 0; --i) {
        // 检查缓存，如果发现该数组类已经被加载过，忽略掉
        auto it = m_classes.find(type + i);
        if (it != m_classes.end()) {
            LOGI("[%d/%d]: type '%s' of array '%s' found in cache, continue\n", i + 1, index, type + i, type);
            continue;
        }
        LOGI("[%d/%d]: create type '%s' of array '%s'\n", i + 1, index, type + i, type);
        auto *klass = create_primitive_type(type + i);
        klass->super_class = holder.java_lang_Object;
        klass->component_type = component_type;

        // 数组类型要实现 java.io.Serializable 和 java.lang.Cloneable 接口
        // 但不需要真正创建 jmethod
        klass->interface_num = 2;
        klass->interfaces = m_allocator.calloc_type<jclass*>(2);
        klass->interfaces[0] = holder.java_io_Serializable;
        klass->interfaces[1] = holder.java_lang_Cloneable;

        // 创建继承树
        klass->parent_tree_size = 3;
        klass->parent_tree = m_allocator.calloc_type<jclass*>(3);
        klass->parent_tree[0] = holder.java_lang_Object;
        klass->parent_tree[1] = holder.java_io_Serializable;
        klass->parent_tree[2] = holder.java_lang_Cloneable;

        m_classes[type + i] = klass;
        component_type = klass;
    }
    LOGI("load array type '%s' finish, result is '%s'\n", type, component_type->name);
    return component_type;
}
