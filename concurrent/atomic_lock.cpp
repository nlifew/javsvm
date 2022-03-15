
#include "atomic_lock.h"

using namespace javsvm;

unsigned atomic_lock::CONCURRENT_THREAD = std::thread::hardware_concurrency();

