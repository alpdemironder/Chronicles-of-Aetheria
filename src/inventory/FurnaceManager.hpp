#pragma once

#include "ItemStack.hpp"
#include <vector>
#include <string>

namespace Aetheria {

struct SmeltRecipe {
    uint16_t inputId;
    ItemStack result;
    float cookTime = 8.0f; // Seconds to cook 1 item
    std::string name;
};

class FurnaceManager {
public:
    FurnaceManager();

    void update(float dt);

    // Slot 0: Input, Slot 1: Fuel, Slot 2: Output
    ItemStack& getInputSlot() { return input; }
    const ItemStack& getInputSlot() const { return input; }

    ItemStack& getFuelSlot() { return fuel; }
    const ItemStack& getFuelSlot() const { return fuel; }

    ItemStack& getOutputSlot() { return output; }
    const ItemStack& getOutputSlot() const { return output; }

    bool isBurning() const { return burnTimer > 0.0f; }
    float getBurnProgress() const { return (maxBurnDuration > 0.0f) ? (burnTimer / maxBurnDuration) : 0.0f; }
    float getCookProgress() const { return (targetCookTime > 0.0f) ? (cookProgress / targetCookTime) : 0.0f; }

    static float getFuelDuration(uint16_t fuelId);
    static const SmeltRecipe* findRecipe(uint16_t inputId);

private:
    ItemStack input;
    ItemStack fuel;
    ItemStack output;

    float burnTimer = 0.0f;
    float maxBurnDuration = 0.0f;
    float cookProgress = 0.0f;
    float targetCookTime = 8.0f;

    static std::vector<SmeltRecipe> recipes;
    static bool recipesInitialized;
    static void initRecipes();
};

} // namespace Aetheria
