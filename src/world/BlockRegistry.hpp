#pragma once

#include "Block.hpp"
#include <vector>
#include <unordered_map>

namespace Aetheria {

class BlockRegistry {
public:
    static void init();
    static const BlockDef& get(uint16_t id);
    static const std::vector<BlockDef>& getAll();
    static size_t getCount();

private:
    static void registerBlock(const BlockDef& def);
    static std::vector<BlockDef> blocks;
    static bool initialized;
};

} // namespace Aetheria
