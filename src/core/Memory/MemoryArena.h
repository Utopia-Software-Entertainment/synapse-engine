#pragma once

#include <cstddef>

namespace synapse::core {

class MemoryArena {
public:
    explicit MemoryArena(size_t size);
    ~MemoryArena();
    void* allocate(size_t bytes);
    void reset();
private:
    size_t m_size;
};

} // namespace synapse::core
