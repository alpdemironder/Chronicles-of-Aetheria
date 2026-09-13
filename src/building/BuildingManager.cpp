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

static void appendBox(std::vector<VoxelVertex>& verts, const Vec3& pos, const Vec3& size, const Vec4& col, float texIndex = 370.0f) {
    Vec3 h = size * 0.5f;
    float x0 = pos.x - h.x, x1 = pos.x + h.x;
    float y0 = pos.y - h.y, y1 = pos.y + h.y;
    float z0 = pos.z - h.z, z1 = pos.z + h.z;

    // 6 Faces
    auto addQuad = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n, float shade) {
        Vec4 c = {col.x * shade, col.y * shade, col.z * shade, col.w};
        verts.push_back({p0.x, p0.y, p0.z, 0, 0, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p1.x, p1.y, p1.z, 1, 0, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p2.x, p2.y, p2.z, 1, 1, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p0.x, p0.y, p0.z, 0, 0, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p2.x, p2.y, p2.z, 1, 1, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
        verts.push_back({p3.x, p3.y, p3.z, 0, 1, n.x, n.y, n.z, c.x, c.y, c.z, c.w, texIndex});
    };

    addQuad({x0, y1, z1}, {x1, y1, z1}, {x1, y1, z0}, {x0, y1, z0}, {0, 1, 0}, 1.0f);  // Top
    addQuad({x0, y0, z0}, {x1, y0, z0}, {x1, y0, z1}, {x0, y0, z1}, {0, -1, 0}, 0.5f); // Bottom
    addQuad({x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}, {0, 0, 1}, 0.85f); // Front
    addQuad({x1, y0, z0}, {x0, y0, z0}, {x0, y1, z0}, {x1, y1, z0}, {0, 0, -1}, 0.8f); // Back
    addQuad({x1, y0, z1}, {x1, y0, z0}, {x1, y1, z0}, {x1, y1, z1}, {1, 0, 0}, 0.7f);  // Right
    addQuad({x0, y0, z0}, {x0, y0, z1}, {x0, y1, z1}, {x0, y1, z0}, {-1, 0, 0}, 0.75f);// Left
}

void BuildingManager::rebuildStructureMesh() {
    std::vector<VoxelVertex> verts;
    verts.reserve(structures.size() * 36);

    for (const auto& s : structures) {
        const auto& def = StructureRegistry::get(s.type);
        Vec4 col = {1.0f, 1.0f, 1.0f, 1.0f};
        if (!s.isCompleted) {
            // Scaffold blueprint look: bright translucent amber
            col = {1.2f, 0.9f, 0.3f, 0.75f};
        }

        float tIdx = 370.0f; // Default wood plank
        switch (s.type) {
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

        appendBox(verts, s.position, def.size, col, tIdx);
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
        const auto& def = StructureRegistry::get(selectedType);

        std::vector<VoxelVertex> ghostVerts;
        Vec4 holoColor = ghostValid ? Vec4(0.2f, 0.65f, 1.0f, 0.6f) : Vec4(1.0f, 0.2f, 0.25f, 0.6f);
        appendBox(ghostVerts, ghostPosition, def.size, holoColor);

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
