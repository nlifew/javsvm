

#include <cstdio>
#include <unistd.h>

#include "vm/jvm.h"
#include "utils/global.h"
#include "object/jmethod.h"
#include "engine/engine.h"

using namespace javsvm;

#define VERSION "0.0.1"

static void usage(const char *self) noexcept
{
#if platform_os_arch == platform_linux_x64
    const char *os_arch = "linux_x64";
#elif platform_os_arch == platform_windows_x64
    const char *os_arch = "windows_x64";
#elif platform_os_arch == platform_macos_arm64
    const char *os_arch = "macos_arm64";
#elif platform_os_arch == platform_macos_x64
    const char *os_arch = "macos_x64";
#endif
    printf("Just A Very Simple Virtual Machine !\n");
    printf("platform: %s, version: %s, build %s %s\n",
           os_arch, VERSION, __TIME__, __DATE__);
}


int main(int argc, const char *argv[])
{
    if (argc < 2) {
        usage(argv[0]);
        return 0;
    }

    auto &vm = jvm::get();
    auto &env = vm.attach();

    // 加载系统类
    auto java_lang_System = vm.bootstrap_loader.load_class("java/lang/System");
    if (java_lang_System == nullptr) {
        fprintf(stderr, "failed to load java/lang/System, abort\n");
        return 1;
    }

    if (java_lang_System->invoke_clinit() < 0) {
        fprintf(stderr, "an Exception occurred when initiating java/lang/System, abort\n");
        return 1;
    }

    // 加载 Main 类
    // todo: 使用新的 classloader
    auto Main = vm.bootstrap_loader.load_class(argv[1]);
    if (Main == nullptr) {
        fprintf(stderr, "failed to load Main class '%s', abort\n", argv[1]);
        return 1;
    }

    auto main_method = Main->get_static_method("main", "([Ljava/lang/String;)V");
    if (main_method == nullptr) {
        fprintf(stderr, "is this class '%s' has main(String[]) ?\n", argv[1]);
        return 1;
    }

    // 解析需要传递到 java 层的参数
    auto java_lang_String = vm.bootstrap_loader.load_class("java/lang/String");
    jref args = vm.array.new_object_array(java_lang_String, argc - 2);
    {
        jref *temp = new jref[argc - 2];
        std::unique_ptr<jref, void(*)(const jref*)> guard(
                temp, [](const jref *ptr) { delete[] ptr; });

        for (int i = 2; i < argc; ++i) {
            temp[i - 2] = vm.string.new_string(argv[i]);
        }
        vm.array.set_array_region(args, 0, argc - 2, temp);
    }

    // 执行 main 函数
    {
        slot_t slots[1];
        jargs wrap = slots;
        wrap.next<jref>() = args;
        main_method->invoke_static(wrap);
    }

    // 等待所有的非 daemon 线程结束
    vm.detach();
    vm.wait_for();

    // 有两个和虚拟机退出有关的 API，分别是
    // Ljava/lang/System;->runFinalizersOnExit()V 和
    // Ljava/lang/Runtime;->addShutdownHook(Ljava/lang/Thread;)V
    // 我们可能要做一些处理
    // todo:
    return 0;
}
