

#include <cstdio>
#include <unistd.h>

#include "vm/jvm.h"
#include "object/jobject.h"
#include "object/jclass.h"
#include "object/jfield.h"
#include "object/jmethod.h"
#include "object/jarray.h"
#include "engine/engine.h"
#include "dll/dll_loader.h"

#include <thread>
#include <queue>


int main() {
    chdir("..");
    putenv(strdup("CLASSPATH=jre"));

    printf("start\n");
//    jvm &vm = jvm::get();
//    vm.attach();


#if 0
    dll_loader loader;
    void *dll = loader.load_library("home/whoami/../../../../../../../Users/./././edz/desktop//////main.dylib");
    auto add = loader.find_symbol<int(*)(int, int)>("add", dll);

    if (add) {
        printf("call add() %d\n", add(2, 3));
    }
#endif
    return 0;
}
