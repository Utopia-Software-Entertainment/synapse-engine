#pragma once

#include <cstddef>

namespace synapse::core {

class StackAllocator {
public:
    explicit StackAllocator(size_t size);
    ~StackAllocator();
    void* allocate(size_t bytes);
    void reset();
private:
    size_t m_size;
};

} // namespace synapse::core
