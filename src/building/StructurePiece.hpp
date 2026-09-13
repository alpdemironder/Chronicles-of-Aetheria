#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "../core/Math.hpp"

namespace Aetheria {

enum class StructureCategory : uint8_t {
    Foundations,
    WallsAndRoofs,
    Production,
    PalInfrastructure,
    StorageAndDefense,
    Furniture
};

enum class StructureType : uint16_t {
    Foundation_Wood,
    Foundation_Stone,
    Foundation_Metal,
    Wall_Wood,
    Wall_Stone,
    Doorway_Wood,
    Roof_Wood,
    Stairs_Wood,
    Palbox_BaseCore,
    Workbench_Primitive,
    Workbench_Sphere,
    Smelting_Furnace,
    Pal_Bed,
    Feed_Box,
    Storage_Chest,
    Campfire,
    COUNT
};

struct MaterialCost {
    uint16_t blockOrItemId;
    uint16_t count;
};

struct StructureDef {
    StructureType type;
    std::string name;
    StructureCategory category;
    Vec3 size{2.0f, 2.0f, 2.0f};
    float buildWorkRequired = 5.0f; // Seconds to hammer
    float maxHealth = 100.0f;
    Vec4 baseColor{0.6f, 0.4f, 0.2f, 1.0f};
    std::vector<MaterialCost> costs;
};

struct PlacedStructure {
    uint32_t id = 0;
    StructureType type = StructureType::Foundation_Wood;
    Vec3 position{0, 0, 0};
    float rotationY = 0.0f; // Radians
    float buildProgress = 0.0f; // 0.0f to 1.0f
    bool isCompleted = false;
    float currentHealth = 100.0f;
    AABB getAABB() const;
};

class StructureRegistry {
public:
    static void init();
    static const StructureDef& get(StructureType type);
    static const std::vector<StructureDef>& getAll();

private:
    static std::vector<StructureDef> defs;
    static bool initialized;
};

} // namespace Aetheria
