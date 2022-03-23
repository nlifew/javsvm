


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

    map.lookup([](std::pair<const Key, Value> &pair) {
        LOGI("[%s] -> [%d]\n", pair.first.c_str(), pair.second);
        pair.second = -pair.second;
    });
    map.lookup([](std::pair<const Key, Value> &pair) {
        LOGI("[%s] -> [%d]\n", pair.first.c_str(), pair.second);
    });


    LOGI("\n\n");


    int idx = 0;
    map.lookup_when([&idx](std::pair<const Key, Value> &pair) {
        LOGI("[%s] -> [%d]\n", pair.first.c_str(), pair.second);
        return idx ++ < 3;
    });


    return 0;
}