#pragma once

#include <cstddef>

namespace synapse::core {

class StackAllocator {
public:
    explicit StackAllocator(size_t size);
    ~StackAllocator();

    StackAllocator(const StackAllocator&) = delete;
    StackAllocator& operator=(const StackAllocator&) = delete;

    void* allocate(size_t bytes);
    void reset();

private:
    void* m_memory = nullptr;
    size_t m_size = 0;
    size_t m_offset = 0;
};

} // namespace synapse::core
