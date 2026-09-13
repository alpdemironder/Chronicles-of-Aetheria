#pragma once

#include "StructurePiece.hpp"
#include "../render/GLBuffer.hpp"
#include "../render/GLPipeline.hpp"
#include <vector>
#include <memory>

namespace Aetheria {

class World;
class AudioEngine;

class BuildingManager {
public:
    BuildingManager(World* world, AudioEngine* audio);
    ~BuildingManager();

    void update(float dt);
    void render(GLPipeline* pipeline, const Mat4& viewProj, const Vec3& camPos, float totalTime);

    // Build Mode Controls
    void setBuildMode(bool active) { isBuilding = active; }
    bool getIsBuilding() const { return isBuilding; }

    void setDismantleMode(bool active) { isDismantling = active; }
    bool getIsDismantling() const { return isDismantling; }

    void selectStructure(StructureType type);
    StructureType getSelectedStructure() const { return selectedType; }

    void rotatePreview();
    void updateGhostPlacement(const Ray& aimRay);
    bool tryPlaceStructure(class Inventory* playerInv = nullptr);
    bool tryDismantle(const Ray& aimRay, class Inventory* playerInv = nullptr);

    // Blueprint Hammering
    bool hammerTargetBlueprint(const Ray& aimRay, float workAmount = 1.0f);
    void creatureAssistBuilding(float dt);

    // Base Camp Territory
    bool hasBaseCamp() const { return baseCampActive; }
    Vec3 getBaseCampCenter() const { return baseCampPos; }
    float getBaseCampRadius() const { return 30.0f; }

    const std::vector<PlacedStructure>& getPlacedStructures() const { return structures; }
    const PlacedStructure* getTargetedStructure(const Ray& aimRay, float maxDist = 6.0f) const;

private:
    void rebuildStructureMesh();
    Vec3 calculateSnapPosition(const Vec3& rawPos, StructureType type) const;

    World* world = nullptr;
    AudioEngine* audio = nullptr;

    bool isBuilding = false;
    bool isDismantling = false;
    StructureType selectedType = StructureType::Foundation_Wood;
    float previewRotation = 0.0f;
    Vec3 ghostPosition{0, 0, 0};
    bool ghostValid = false;

    bool baseCampActive = false;
    Vec3 baseCampPos{0, 0, 0};

    std::vector<PlacedStructure> structures;
    uint32_t nextStructureId = 1;

    // Rendering buffers
    GLBuffer structureMeshBuffer;
    uint32_t structureVertexCount = 0;
    bool meshDirty = true;

    GLBuffer ghostMeshBuffer;
    uint32_t ghostVertexCount = 0;
};

} // namespace Aetheria
