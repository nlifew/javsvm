


#include "concurrent/concurrent_map.h"
#include "utils/log.h"
#include <string>


using namespace javsvm;

using Key = std::string;
using Value = int;
using map_t = concurrent_map<Key, Value, std::shared_mutex, std::hash<Key>, std::equal_to<>, std::allocator<std::pair<const Key, Value>>>;

int main()
{
    map_t map;
    char buff[4];
    for (int i = 0; i < 10; ++i) {
        snprintf(buff, sizeof(buff), "%d", i);
        map.put(buff, i);
    }

    map.lookup([](const auto &it) -> bool {
        LOGI("[%s] -> [%d]\n", it.first.c_str(), it.second);
        return true;
    });


    LOGI("\n\n");


    int idx = 0;
    map.lookup([&idx](const auto &it) -> bool {
        LOGI("[%s] -> [%d]\n", it.first.c_str(), it.second);
        return idx ++ < 4;
    });


    return 0;
}