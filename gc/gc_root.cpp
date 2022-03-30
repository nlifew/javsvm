
#include "gc_root.h"

using namespace javsvm;

gc_root::static_field_set_type gc_root::static_field_pool = concurrent_set<jref *>(8);

gc_root::ref_set_type gc_root::ref_pool = concurrent_set<gc_root*>(8);

gc_weak::ref_set_type gc_weak::ref_pool = concurrent_set<gc_weak*>(1);