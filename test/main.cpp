

#include "../vm/jvm.h"
#include "../object/jmethod.h"
#include "../engine/engine.h"

using namespace javsvm;



static void invoke_main(const char *main_method) noexcept
{
    auto &vm = jvm::get();

    auto klass = vm.bootstrap_loader.load_class(main_method);
    assert(klass != nullptr);

    auto main = klass->get_static_method("main", "([Ljava/lang/String;)V");
    assert(main != nullptr);

    // String 类
    auto java_lang_String = vm.bootstrap_loader.load_class("java/lang/String");
    assert(java_lang_String != nullptr);

    // 解析需要传递到 java 层的参数
    slot_t slots[1];
    jargs args = slots;
    args.next<jref>() = vm.array.new_object_array(java_lang_String, 0);

    // 执行 main 函数
    main->invoke_static(args);
}

int main(int argc, const char *argv[])
{
    putenv(strdup("CLASSPATH=../../jre/out"));

    auto &vm = jvm::get();
    auto &env = vm.attach();

    auto java_lang_System = vm.bootstrap_loader.load_class("java/lang/System");
    assert(java_lang_System);

    auto ok = java_lang_System->invoke_clinit();
    assert(ok == 0);

    assert(argc == 2);
    invoke_main(argv[1]);

    vm.detach();
    vm.wait_for();
    return 0;
}