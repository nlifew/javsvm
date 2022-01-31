
#include "gc_root.h"

using namespace javsvm;

gc_root::static_field_set gc_root::s_field_pool = concurrent_set<jref *>();

gc_root::ref_set gc_root::s_ref_pool = concurrent_set<gc_root*>();

