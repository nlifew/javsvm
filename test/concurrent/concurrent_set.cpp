

#include "utils/log.h"
#include "concurrent/concurrent_set.h"

using namespace javsvm;

using set_type = concurrent_set<std::string>;

int main()
{
    set_type set;
    char buff[32];

    for (int i = 0; i < 8; ++i) {
        snprintf(buff, sizeof(buff), "%d", i);
        LOGI("main: add [%d] -> %d\n", i, set.add(buff));
    }

    for (int i = 0; i < 8; ++i) {
        snprintf(buff, sizeof(buff), "%d", i);
        LOGI("main: [%d] -> [%d]\n", i, set.contains(buff) != nullptr);
    }

    return 0;
}