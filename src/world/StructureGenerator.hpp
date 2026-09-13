#pragma once

#include <cstdint>

namespace Aetheria {

class Chunk;

enum class StructureType : uint8_t {
    None = 0,
    AncientShrine,
    DesertPyramid,
    WizardTower,
    WoodlandCottage,
    CatacombCrypt,
    MineshaftOutpost
};

class StructureGenerator {
public:
    // Attempts to generate structures in the given chunk based on biome and deterministic seed hash
    static void generateStructures(Chunk* chunk, uint32_t seed);

private:
    static void buildAncientShrine(Chunk* chunk, int startX, int surfaceY, int startZ);
    static void buildDesertPyramid(Chunk* chunk, int startX, int surfaceY, int startZ);
    static void buildWizardTower(Chunk* chunk, int startX, int surfaceY, int startZ);
    static void buildWoodlandCottage(Chunk* chunk, int startX, int surfaceY, int startZ);
    static void buildCatacombCrypt(Chunk* chunk, int startX, int depthY, int startZ);
    static void buildMineshaftOutpost(Chunk* chunk, int startX, int depthY, int startZ);

    static void setBlockSafe(Chunk* chunk, int x, int y, int z, uint16_t blockId);
};

} // namespace Aetheria
