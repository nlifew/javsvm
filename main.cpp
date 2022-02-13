

#include <cstdio>
#include <unistd.h>

#include "vm/jvm.h"

using namespace javsvm;

/**
 * main 函数主要逻辑:
 * a) 解析命令行参数，得到 Main 类的类名
 * b) 设置 classpath，加载 java/lang/Object 类
 * c) jvm::get().attach()
 * d) 绑定 jni 函数
 * e) 运行 java 代码
 * f) jvm::get().detach() & jvm::get().wait_for()
 */
int main() {
    chdir("..");
    putenv(strdup("CLASSPATH=jre"));
    printf("start\n");

    auto &vm = jvm::get();
    vm.attach();


    vm.detach();
    vm.wait_for();
    return 0;
}
