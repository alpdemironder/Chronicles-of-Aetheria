#include "BuildingManager.hpp"
#include "../core/Audio.hpp"
#include "../world/World.hpp"
#include "../inventory/Inventory.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cstring>

namespace Aetheria {

BuildingManager::BuildingManager(World* world, AudioEngine* audio)
    : world(world), audio(audio) {
    StructureRegistry::init();
}

BuildingManager::~BuildingManager() {
}

void BuildingManager::selectStructure(StructureType type) {
    selectedType = type;
    meshDirty = true;
}

void BuildingManager::rotatePreview() {
    previewRotation += 90.0f * DEG2RAD;
    if (previewRotation >= 2.0f * PI) {
        previewRotation -= 2.0f * PI;
    }
}

Vec3 BuildingManager::calculateSnapPosition(const Vec3& rawPos, StructureType type) const {
    const auto& def = StructureRegistry::get(type);

    // Look for nearby placed foundations or walls to snap to
    for (const auto& s : structures) {
        const auto& sDef = StructureRegistry::get(s.type);

        // Foundation-to-Foundation snap
        if (def.category == StructureCategory::Foundations && sDef.category == StructureCategory::Foundations) {
            float dist = (s.position - rawPos).length();
            if (dist < 3.0f) {
                // Snap to 2-meter grid relative to structure
                float dx = std::round((rawPos.x - s.position.x) / 2.0f) * 2.0f;
                float dz = std::round((rawPos.z - s.position.z) / 2.0f) * 2.0f;
                return Vec3(s.position.x + dx, s.position.y, s.position.z + dz);
            }
        }

        // Wall snapping to Foundation edge
        if (def.category == StructureCategory::WallsAndRoofs && sDef.category == StructureCategory::Foundations) {
            float dist = (s.position - rawPos).length();
            if (dist < 2.5f) {
                float wallY = s.position.y + sDef.size.y * 0.5f + def.size.y * 0.5f;
                return Vec3(s.position.x, wallY, s.position.z + 1.0f);
            }
        }
    }

    // Furniture finer snap to 0.5m grid
    if (def.category == StructureCategory::Furniture) {
        return Vec3(
            std::round(rawPos.x * 2.0f) / 2.0f,
            std::round(rawPos.y * 2.0f) / 2.0f,
            std::round(rawPos.z * 2.0f) / 2.0f
        );
    }

    // Default: grid snap to 1-meter integer positions
    return Vec3(
        std::round(rawPos.x),
        std::round(rawPos.y * 2.0f) / 2.0f,
        std::round(rawPos.z)
    );
}

void BuildingManager::updateGhostPlacement(const Ray& aimRay) {
    RaycastResult hit = world->raycast(aimRay, 10.0f);
    if (hit.hit) {
        Vec3 rawHitPos = Vec3(static_cast<float>(hit.adjacentPos.x) + 0.5f,
                              static_cast<float>(hit.adjacentPos.y),
                              static_cast<float>(hit.adjacentPos.z) + 0.5f);

        ghostPosition = calculateSnapPosition(rawHitPos, selectedType);
        ghostValid = true;
    } else {
        ghostPosition = aimRay.pointAt(5.0f);
        ghostValid = false;
    }
}

bool BuildingManager::tryPlaceStructure(Inventory* playerInv) {
    if (!ghostValid) return false;

    const auto& def = StructureRegistry::get(selectedType);

    // Verify and deduct required materials from player inventory
    if (playerInv) {
        for (const auto& cost : def.costs) {
            if (!playerInv->hasItem(cost.blockOrItemId, cost.count)) {
                if (audio) audio->playSound(SoundID::CreatureHit, 0.7f, 0.8f);
                std::cout << "Missing materials to place " << def.name << "!" << std::endl;
                return false;
            }
        }
        for (const auto& cost : def.costs) {
            playerInv->removeItem(cost.blockOrItemId, cost.count);
        }
    }

    PlacedStructure s;
    s.id = nextStructureId++;
    s.type = selectedType;
    s.position = ghostPosition;
    s.rotationY = previewRotation;
    s.buildProgress = 0.0f; // Starts as scaffold blueprint
    s.isCompleted = false;
    s.currentHealth = def.maxHealth;

    if (s.type == StructureType::Palbox_BaseCore) {
        baseCampActive = true;
        baseCampPos = s.position;
    }

    structures.push_back(s);
    meshDirty = true;

    if (audio) {
        audio->playSound(SoundID::BlockPlace, 1.2f, 1.0f);
    }

    std::cout << "Placed blueprint: " << def.name << " at ("
              << s.position.x << ", " << s.position.y << ", " << s.position.z << ")" << std::endl;

    return true;
}

const PlacedStructure* BuildingManager::getTargetedStructure(const Ray& aimRay, float maxDist) const {
    float closestDist = maxDist;
    const PlacedStructure* closest = nullptr;

    for (const auto& s : structures) {
        AABB box = s.getAABB();
        // Check simple ray step against box
        for (float t = 0.5f; t <= maxDist; t += 0.25f) {
            Vec3 p = aimRay.pointAt(t);
            if (box.contains(p) && t < closestDist) {
                closestDist = t;
                closest = &s;
                break;
            }
        }
    }
    return closest;
}

bool BuildingManager::tryDismantle(const Ray& aimRay, Inventory* playerInv) {
    const PlacedStructure* target = getTargetedStructure(aimRay, 7.0f);
    if (!target) return false;

    uint32_t targetId = target->id;
    for (auto it = structures.begin(); it != structures.end(); ++it) {
        if (it->id == targetId) {
            if (it->type == StructureType::Palbox_BaseCore) {
                baseCampActive = false;
            }

            // Refund 100% materials to player inventory
            if (playerInv) {
                const auto& def = StructureRegistry::get(it->type);
                for (const auto& cost : def.costs) {
                    playerInv->addItem(cost.blockOrItemId, cost.count);
                }
            }

            structures.erase(it);
            meshDirty = true;
            if (audio) audio->playSound(SoundID::BlockBreak, 0.8f, 1.0f);
            std::cout << "Dismantled structure, 100% resources refunded!" << std::endl;
            return true;
        }
    }
    return false;
}

bool BuildingManager::hammerTargetBlueprint(const Ray& aimRay, float workAmount) {
    const PlacedStructure* target = getTargetedStructure(aimRay, 5.0f);
    if (!target || target->isCompleted) return false;

    uint32_t targetId = target->id;
    for (auto& s : structures) {
        if (s.id == targetId && !s.isCompleted) {
            const auto& def = StructureRegistry::get(s.type);
            s.buildProgress += workAmount / def.buildWorkRequired;

            if (audio) {
                audio->playSound(SoundID::HammerBuild, 0.9f + (rand() % 20) / 100.0f, 0.9f);
            }

            if (s.buildProgress >= 1.0f) {
                s.buildProgress = 1.0f;
                s.isCompleted = true;
                if (audio) audio->playSound(SoundID::StructureComplete, 1.0f, 1.0f);
                std::cout << "Structure Completed: " << def.name << "!" << std::endl;
            }
            meshDirty = true;
            return true;
        }
    }
    return false;
}

void BuildingManager::creatureAssistBuilding(float dt) {
    // Friendly creatures in base camp territory help build blueprints
    if (!baseCampActive) return;

    for (auto& s : structures) {
        if (!s.isCompleted) {
            float dist = (s.position - baseCampPos).length();
            if (dist <= getBaseCampRadius()) {
                const auto& def = StructureRegistry::get(s.type);
                s.buildProgress += (dt * 0.4f) / def.buildWorkRequired;
                if (s.buildProgress >= 1.0f) {
                    s.buildProgress = 1.0f;
                    s.isCompleted = true;
                    if (audio) audio->playSound(SoundID::StructureComplete, 1.0f, 1.0f);
                }
                meshDirty = true;
            }
        }
    }
}

static void appendRotatedBox(
    std::vector<VoxelVertex>& verts,
    const Vec3& centerPos,
    const Vec3& boxRelPos,
    const Vec3& boxSize,
    const Vec4& col,
    float texIndex,
    float rotY = 0.0f
) {
    float cosA = std::cos(rotY);
    float sinA = std::sin(rotY);

    Vec3 rotOffset = {
        boxRelPos.x * cosA - boxRelPos.z * sinA,
        boxRelPos.y,
        boxRelPos.x * sinA + boxRelPos.z * cosA
    };
    Vec3 subCenter = centerPos + rotOffset;
    Vec3 h = boxSize * 0.5f;

    auto rotPt = [&](float lx, float ly, float lz) -> Vec3 {
        float rx = lx * cosA - lz * sinA;
        float rz = lx * sinA + lz * cosA;
        return { subCenter.x + rx, subCenter.y + ly, subCenter.z + rz };
    };

    auto rotNorm = [&](float nx, float ny, float nz) -> Vec3 {
        return { nx * cosA - nz * sinA, ny, nx * sinA + nz * cosA };
    };

    auto addQuad = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n, float shade) {
        Vec4 c = {col.x * shade, col.y * shade, col.z * shade, col.w};
        verts.push_back({p0.x, p0.y, p0.z, 0, 0, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p1.x, p1.y, p1.z, 1, 0, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p2.x, p2.y, p2.z, 1, 1, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p0.x, p0.y, p0.z, 0, 0, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p2.x, p2.y, p2.z, 1, 1, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p3.x, p3.y, p3.z, 0, 1, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
    };

    Vec3 p000 = rotPt(-h.x, -h.y, -h.z);
    Vec3 p100 = rotPt( h.x, -h.y, -h.z);
    Vec3 p110 = rotPt( h.x,  h.y, -h.z);
    Vec3 p010 = rotPt(-h.x,  h.y, -h.z);
    Vec3 p001 = rotPt(-h.x, -h.y,  h.z);
    Vec3 p101 = rotPt( h.x, -h.y,  h.z);
    Vec3 p111 = rotPt( h.x,  h.y,  h.z);
    Vec3 p011 = rotPt(-h.x,  h.y,  h.z);

    Vec3 nTop = rotNorm(0, 1, 0);
    Vec3 nBot = rotNorm(0, -1, 0);
    Vec3 nFront = rotNorm(0, 0, 1);
    Vec3 nBack = rotNorm(0, 0, -1);
    Vec3 nRight = rotNorm(1, 0, 0);
    Vec3 nLeft = rotNorm(-1, 0, 0);

    // Top face (+Y)
    addQuad(p011, p111, p110, p010, nTop, 1.0f);
    // Bottom face (-Y)
    addQuad(p000, p100, p101, p001, nBot, 0.5f);
    // Front face (+Z)
    addQuad(p001, p101, p111, p011, nFront, 0.85f);
    // Back face (-Z)
    addQuad(p100, p000, p010, p110, nBack, 0.8f);
    // Right face (+X)
    addQuad(p101, p100, p110, p111, nRight, 0.7f);
    // Left face (-X)
    addQuad(p000, p001, p011, p010, nLeft, 0.75f);
}

static void appendStructureGeometry(
    std::vector<VoxelVertex>& verts,
    StructureType type,
    const Vec3& pos,
    float rotY,
    const Vec4& col,
    float overrideTexIdx = -1.0f
) {
    const auto& def = StructureRegistry::get(type);
    int woodVariant = -1, woodSpecies = -1;
    bool isWood = StructureRegistry::isWoodVariant(type, woodVariant, woodSpecies);

    float tIdx = overrideTexIdx >= 0.0f ? overrideTexIdx : 370.0f;
    if (isWood) {
        tIdx = static_cast<float>(72 + woodSpecies * 3);
    } else {
        switch (type) {
        case StructureType::Foundation_Wood: tIdx = 370.0f; break;
        case StructureType::Foundation_Stone: tIdx = 371.0f; break;
        case StructureType::Foundation_Metal: tIdx = 373.0f; break;
        case StructureType::Wall_Wood:
        case StructureType::Doorway_Wood: tIdx = 372.0f; break;
        case StructureType::Wall_Stone: tIdx = 373.0f; break;
        case StructureType::Roof_Wood: tIdx = 374.0f; break;
        case StructureType::Stairs_Wood: tIdx = 376.0f; break;
        case StructureType::Palbox_BaseCore: tIdx = 378.0f; break;
        case StructureType::Workbench_Primitive:
        case StructureType::Workbench_Sphere: tIdx = 379.0f; break;
        case StructureType::Storage_Chest:
        case StructureType::Feed_Box: tIdx = 380.0f; break;
        case StructureType::Pal_Bed: tIdx = 381.0f; break;
        default: tIdx = 370.0f; break;
        }
    }

    if (overrideTexIdx >= 0.0f) {
        tIdx = overrideTexIdx;
    }

    if (isWood) {
        switch (static_cast<WoodPieceVariant>(woodVariant)) {
        case WoodPieceVariant::Stairs: {
            // Lower step
            appendRotatedBox(verts, pos, {0.0f, -0.5f, 0.0f}, {2.0f, 1.0f, 2.0f}, col, tIdx, rotY);
            // Upper step (back half)
            appendRotatedBox(verts, pos, {0.0f, 0.5f, 0.5f}, {2.0f, 1.0f, 1.0f}, col, tIdx, rotY);
            return;
        }
        case WoodPieceVariant::Trapdoor: {
            // Main slatted panel
            appendRotatedBox(verts, pos, {0.0f, 0.0f, 0.0f}, {1.96f, 0.16f, 1.96f}, col, tIdx, rotY);
            // 2 reinforcement cross battens
            appendRotatedBox(verts, pos, {-0.55f, 0.08f, 0.0f}, {0.18f, 0.06f, 1.96f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.55f, 0.08f, 0.0f}, {0.18f, 0.06f, 1.96f}, col, tIdx, rotY);
            return;
        }
        case WoodPieceVariant::Slab: {
            // Half-height bottom slab
            appendRotatedBox(verts, pos, {0.0f, -0.5f, 0.0f}, {2.0f, 1.0f, 2.0f}, col, tIdx, rotY);
            return;
        }
        case WoodPieceVariant::SideSlab: {
            // Half-width vertical partition slab
            appendRotatedBox(verts, pos, {-0.5f, 0.0f, 0.0f}, {1.0f, 2.0f, 2.0f}, col, tIdx, rotY);
            return;
        }
        case WoodPieceVariant::Chair: {
            // 4 legs
            appendRotatedBox(verts, pos, {-0.28f, -0.38f, -0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.28f, -0.38f, -0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, {-0.28f, -0.38f,  0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.28f, -0.38f,  0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            // Seat plank
            appendRotatedBox(verts, pos, {0.0f, -0.06f, 0.0f}, {0.74f, 0.10f, 0.74f}, col, tIdx, rotY);
            // Backrest posts
            appendRotatedBox(verts, pos, {-0.28f, 0.35f, -0.30f}, {0.08f, 0.72f, 0.08f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.28f, 0.35f, -0.30f}, {0.08f, 0.72f, 0.08f}, col, tIdx, rotY);
            // Backrest top splat
            appendRotatedBox(verts, pos, {0.0f, 0.50f, -0.30f}, {0.64f, 0.30f, 0.06f}, col, tIdx, rotY);
            return;
        }
        case WoodPieceVariant::Bench: {
            // 6 legs
            appendRotatedBox(verts, pos, {-0.85f, -0.38f, -0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.00f, -0.38f, -0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.85f, -0.38f, -0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, {-0.85f, -0.38f,  0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.00f, -0.38f,  0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.85f, -0.38f,  0.28f}, {0.10f, 0.54f, 0.10f}, col, tIdx, rotY);
            // Wide bench seat
            appendRotatedBox(verts, pos, {0.0f, -0.06f, 0.0f}, {1.94f, 0.10f, 0.74f}, col, tIdx, rotY);
            // Bench backrest
            appendRotatedBox(verts, pos, {0.0f, 0.40f, -0.30f}, {1.94f, 0.48f, 0.06f}, col, tIdx, rotY);
            // Armrests
            appendRotatedBox(verts, pos, {-0.90f, 0.16f, 0.02f}, {0.08f, 0.34f, 0.64f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.90f, 0.16f, 0.02f}, {0.08f, 0.34f, 0.64f}, col, tIdx, rotY);
            return;
        }
        case WoodPieceVariant::Table: {
            // 4 corner legs
            appendRotatedBox(verts, pos, {-0.62f, -0.08f, -0.62f}, {0.14f, 0.84f, 0.14f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.62f, -0.08f, -0.62f}, {0.14f, 0.84f, 0.14f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, {-0.62f, -0.08f,  0.62f}, {0.14f, 0.84f, 0.14f}, col, tIdx, rotY);
            appendRotatedBox(verts, pos, { 0.62f, -0.08f,  0.62f}, {0.14f, 0.84f, 0.14f}, col, tIdx, rotY);
            // Apron support frame
            appendRotatedBox(verts, pos, {0.0f, 0.30f, 0.0f}, {1.32f, 0.08f, 1.32f}, col, tIdx, rotY);
            // Table top
            appendRotatedBox(verts, pos, {0.0f, 0.40f, 0.0f}, {1.60f, 0.12f, 1.60f}, col, tIdx, rotY);
            return;
        }
        default:
            break;
        }
    }

    // Default: single box structure
    appendRotatedBox(verts, pos, {0.0f, 0.0f, 0.0f}, def.size, col, tIdx, rotY);
}

void BuildingManager::rebuildStructureMesh() {
    std::vector<VoxelVertex> verts;
    verts.reserve(structures.size() * 72);

    for (const auto& s : structures) {
        Vec4 col = {1.0f, 1.0f, 1.0f, 1.0f};
        if (!s.isCompleted) {
            // Scaffold blueprint look: bright translucent amber
            col = {1.2f, 0.9f, 0.3f, 0.75f};
        }

        appendStructureGeometry(verts, s.type, s.position, s.rotationY, col);
    }

    structureVertexCount = static_cast<uint32_t>(verts.size());
    if (structureVertexCount > 0) {
        structureMeshBuffer.uploadVoxelData(verts.data(), verts.size() * sizeof(VoxelVertex), structureVertexCount);
    }

    meshDirty = false;
}

void BuildingManager::update(float dt) {
    creatureAssistBuilding(dt);

    if (meshDirty) {
        rebuildStructureMesh();
    }
}

void BuildingManager::render(GLPipeline* pipeline, const Mat4& viewProj, const Vec3& camPos, float totalTime) {
    // 1. Render placed structures
    if (structureVertexCount > 0) {
        structureMeshBuffer.draw();
    }

    // 2. Render holographic ghost preview if in build mode
    if (isBuilding) {
        std::vector<VoxelVertex> ghostVerts;
        Vec4 holoColor = ghostValid ? Vec4(0.2f, 0.65f, 1.0f, 0.6f) : Vec4(1.0f, 0.2f, 0.25f, 0.6f);
        // Note: rotY is 0 here because the model matrix below applies rotationY(previewRotation)
        appendStructureGeometry(ghostVerts, selectedType, ghostPosition, 0.0f, holoColor, 370.0f);

        ghostVertexCount = static_cast<uint32_t>(ghostVerts.size());
        if (ghostVertexCount > 0) {
            ghostMeshBuffer.uploadVoxelData(ghostVerts.data(), ghostVerts.size() * sizeof(VoxelVertex), ghostVertexCount, true);

            Mat4 model = Mat4::translation(ghostPosition) * Mat4::rotationY(previewRotation) * Mat4::translation(ghostPosition * -1.0f);
            Mat4 mvp = viewProj * model;

            pipeline->useHologram();
            pipeline->setHologramUniforms(mvp, model, camPos, holoColor, totalTime);
            ghostMeshBuffer.draw();
            pipeline->useVoxel();
        }
    }
}

} // namespace Aetheria
