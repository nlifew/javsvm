

#include "../vm/jvm.h"
#include "../object/jclass.h"
#include "../object/jmethod.h"

using namespace javsvm;

int main()
{
    putenv(strdup("CLASSPATH=../../jre/out"));

    auto &vm = jvm::get();
    auto &env = vm.attach();

    auto java_lang_System = vm.bootstrap_loader.load_class("java/lang/System");
    assert(java_lang_System);

    auto ok = java_lang_System->invoke_clinit();
    assert(ok == 0);

    vm.detach();
    vm.wait_for();
    return 0;
}