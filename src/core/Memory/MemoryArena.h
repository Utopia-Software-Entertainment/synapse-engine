#pragma once

#include <cstddef>

namespace synapse::core {

class MemoryArena {
public:
    explicit MemoryArena(size_t size);
    ~MemoryArena();

    MemoryArena(const MemoryArena&) = delete;
    MemoryArena& operator=(const MemoryArena&) = delete;

    void* allocate(size_t bytes);
    void reset();

private:
    void* m_memory = nullptr;
    size_t m_size = 0;
    size_t m_offset = 0;
};

} // namespace synapse::core
