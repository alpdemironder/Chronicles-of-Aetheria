#pragma once

#include <cstdint>

namespace Aetheria {

struct ItemStack {
    uint16_t id = 0;        // 0 = Empty, 1..365 = Blocks, 500..599 = Custom Items
    uint32_t count = 0;     // Number of items in stack
    uint32_t maxStack = 64; // Max capacity per slot

    constexpr ItemStack() = default;
    constexpr ItemStack(uint16_t id, uint32_t count = 1, uint32_t maxStack = 64)
        : id(id), count(count), maxStack(maxStack) {}

    bool isEmpty() const { return id == 0 || count == 0; }
    void clear() { id = 0; count = 0; }
};

} // namespace Aetheria
