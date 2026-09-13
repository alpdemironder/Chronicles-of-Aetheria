#include "FurnaceManager.hpp"
#include <algorithm>

namespace Aetheria {

std::vector<SmeltRecipe> FurnaceManager::recipes;
bool FurnaceManager::recipesInitialized = false;

FurnaceManager::FurnaceManager() {
    initRecipes();
}

float FurnaceManager::getFuelDuration(uint16_t fuelId) {
    if (fuelId == 552) return 80.0f; // Coal
    if (fuelId == 553) return 80.0f; // Charcoal
    if (fuelId == 123) return 60.0f; // Coal Ore
    if (fuelId >= 71 && fuelId <= 100 && (fuelId % 6 == 5 || fuelId % 6 == 1)) return 20.0f; // Wood Logs
    if (fuelId >= 72 && fuelId <= 100 && (fuelId % 6 == 0 || fuelId % 6 == 2)) return 15.0f; // Wood Planks
    if (fuelId == 550) return 5.0f;  // Wooden Stick
    if (fuelId == 510 || fuelId == 573 || fuelId == 577 || fuelId == 580) return 10.0f; // Wooden Tools
    return 0.0f;
}

const SmeltRecipe* FurnaceManager::findRecipe(uint16_t inputId) {
    initRecipes();
    for (const auto& r : recipes) {
        if (r.inputId == inputId) return &r;
    }
    return nullptr;
}

void FurnaceManager::initRecipes() {
    if (recipesInitialized) return;
    recipesInitialized = true;

    recipes.clear();

    // 1. Iron Ore -> Iron Ingot
    recipes.push_back({ 124, { 501, 1, 64 }, 8.0f, "Iron Ingot" });
    recipes.push_back({ 125, { 501, 1, 64 }, 8.0f, "Iron Ingot" }); // Deepslate Iron

    // 2. Copper Ore -> Copper Ingot
    recipes.push_back({ 121, { 502, 1, 64 }, 7.0f, "Copper Ingot" });

    // 3. Gold Ore -> Gold Ingot
    recipes.push_back({ 126, { 503, 1, 64 }, 9.0f, "Gold Ingot" });
    recipes.push_back({ 127, { 503, 1, 64 }, 9.0f, "Gold Ingot" }); // Deepslate Gold

    // 4. Sand -> Glass
    recipes.push_back({ 21, { 281, 1, 64 }, 6.0f, "Glass Block" });
    recipes.push_back({ 22, { 281, 1, 64 }, 6.0f, "Glass Block" });

    // 5. Cobblestone -> Smooth Stone
    recipes.push_back({ 37, { 36, 1, 64 }, 6.0f, "Stone" });

    // 6. Raw Meat -> Cooked Feast
    recipes.push_back({ 508, { 522, 1, 16 }, 7.0f, "Cooked Feast" });

    // 7. Wood Logs -> Charcoal
    recipes.push_back({ 71, { 553, 1, 64 }, 8.0f, "Charcoal" });
    recipes.push_back({ 77, { 553, 1, 64 }, 8.0f, "Charcoal" });
    recipes.push_back({ 83, { 553, 1, 64 }, 8.0f, "Charcoal" });
}

void FurnaceManager::update(float dt) {
    // Tick down burning fuel
    if (burnTimer > 0.0f) {
        burnTimer -= dt;
        if (burnTimer < 0.0f) burnTimer = 0.0f;
    }

    const SmeltRecipe* r = findRecipe(input.id);

    bool canSmelt = false;
    if (r && !input.isEmpty()) {
        targetCookTime = r->cookTime;
        // Check if output slot can accept result
        if (output.isEmpty()) {
            canSmelt = true;
        } else if (output.id == r->result.id && output.count + r->result.count <= output.maxStack) {
            canSmelt = true;
        }
    }

    // If not burning but can smelt and fuel available, ignite fuel
    if (burnTimer <= 0.0f && canSmelt && !fuel.isEmpty()) {
        float fDur = getFuelDuration(fuel.id);
        if (fDur > 0.0f) {
            burnTimer = fDur;
            maxBurnDuration = fDur;
            fuel.count--;
            if (fuel.count == 0) fuel.clear();
        }
    }

    // Cook item while furnace is burning
    if (burnTimer > 0.0f && canSmelt) {
        cookProgress += dt;
        if (cookProgress >= targetCookTime) {
            cookProgress = 0.0f;

            // Produce output
            if (output.isEmpty()) {
                output = r->result;
            } else {
                output.count += r->result.count;
            }

            // Deduct input
            input.count--;
            if (input.count == 0) input.clear();
        }
    } else {
        // Slowly cool down if not burning
        if (cookProgress > 0.0f) {
            cookProgress = std::max(0.0f, cookProgress - dt * 2.0f);
        }
    }
}

} // namespace Aetheria
