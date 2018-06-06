#include "id_generator.hpp"
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <atomic>

int64_t GenerateUniqueId() {
    static std::atomic<uint32_t> index(base::RandInt(0, kint32max));
    int32_t cur_index = index.fetch_add(1);
    
    int64_t id = ((int64_t)time(0) << 32) | (int64_t)cur_index;    
    return id;
}
