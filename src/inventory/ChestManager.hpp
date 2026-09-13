#pragma once

#include "Inventory.hpp"
#include "../core/Math.hpp"
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace Aetheria {

class ChestManager {
public:
    static constexpr int CHEST_SLOT_COUNT = 27; // 3 rows of 9 slots

    static ChestManager& instance() {
        static ChestManager inst;
        return inst;
    }

    std::vector<ItemStack>& getOrCreateChest(const IVec3& pos) {
        int64_t key = hashPos(pos.x, pos.y, pos.z);
        auto it = containers.find(key);
        if (it == containers.end()) {
            containers[key] = std::vector<ItemStack>(CHEST_SLOT_COUNT);
            return containers[key];
        }
        return it->second;
    }

    bool hasChest(const IVec3& pos) const {
        int64_t key = hashPos(pos.x, pos.y, pos.z);
        return containers.find(key) != containers.end();
    }

    std::vector<ItemStack> extractChestContents(const IVec3& pos) {
        int64_t key = hashPos(pos.x, pos.y, pos.z);
        auto it = containers.find(key);
        if (it == containers.end()) {
            return {};
        }
        std::vector<ItemStack> items = it->second;
        containers.erase(it);
        return items;
    }

    void removeChest(const IVec3& pos) {
        int64_t key = hashPos(pos.x, pos.y, pos.z);
        containers.erase(key);
    }

    void clearAll() {
        containers.clear();
    }

private:
    ChestManager() = default;

    static int64_t hashPos(int x, int y, int z) {
        return (static_cast<int64_t>(x) & 0x1FFFFF) |
               ((static_cast<int64_t>(y) & 0x3FF) << 21) |
               ((static_cast<int64_t>(z) & 0x1FFFFF) << 31);
    }

    std::unordered_map<int64_t, std::vector<ItemStack>> containers;
};

} // namespace Aetheria
