
#include "utils/log.h"
#include "object/jclass.h"
#include "object/jmethod.h"
#include "engine/engine.h"
#include "vm/jvm.h"

using namespace javsvm;

int main()
{
    putenv(strdup("CLASSPATH=../../../test/engine:../../../jre"));

    LOGI("main: start\n");

    jvm &vm = jvm::get();
    vm.attach();

    auto lib = vm.dll_loader.load_library("./libtest_native.dylib");
    assert(lib != nullptr);


    jclass *Main = vm.bootstrap_loader.load_class("Native");
    assert(Main != nullptr);

    {
        jmethod *add = Main->get_static_method("nAdd", "(II)I");
        assert(add != nullptr);

        slot_t args[2];
        jargs _args(args);
        _args.next<jint>() = 12345;
        _args.next<jint>() = 43210;
        _args.reset();

        jvalue result = add->invoke_static(_args);
        assert(result.i == 55555);
    }
    {
        jmethod *add = Main->get_static_method("nAdd", "(ZBCSIJFDZBCSIJFD)D");
        assert(add != nullptr);

        slot_t args[20];
        jargs _args(args);
        _args.next<jboolean>() = 1;
        _args.next<jbyte>() = 127;
        _args.next<jchar>() = 233;
        _args.next<jshort>() = 12345;
        _args.next<jint>() = 12345678;
        _args.next<jlong>() = -11223344556677;
        _args.next<jfloat>() = 2.345678F;
        _args.next<jdouble>() = 0.4321;

        _args.next<jboolean>() = 0;
        _args.next<jbyte>() = -128;
        _args.next<jchar>() = 67;
        _args.next<jshort>() = 4321;
        _args.next<jint>() = 876543210;
        _args.next<jlong>() = -7766554433221100;
        _args.next<jfloat>() = 2.5F;
        _args.next<jdouble>() = 0.4;

        _args.reset();

        jvalue result = add->invoke_static(_args);
        char buff[64];
        snprintf(buff, sizeof(buff), "%.15E", result.d);
        LOGI("main: %s\n", buff);
        assert(strcmp(buff, "-9.198521985646686E+27") == 0);
    }
    return 0;
}