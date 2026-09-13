#pragma once

#include "ItemStack.hpp"
#include "Inventory.hpp"
#include <string>
#include <vector>

namespace Aetheria {

struct RecipeIngredient {
    uint16_t itemId = 0;
    uint32_t count = 1;
};

struct GridRecipe {
    uint32_t id = 0;
    std::string name;
    std::string category;
    int patternW = 0;
    int patternH = 0;
    std::vector<uint16_t> pattern; // patternW * patternH IDs (0 = empty)
    ItemStack result;
    bool isShapeless = false;
    std::vector<uint16_t> shapelessIngredients; // for shapeless matching
};

struct Recipe {
    uint32_t id = 0;
    std::string name;
    std::string category; // "Materials", "Weapons", "Tools", "Armor", "Survival"
    ItemStack result;
    std::vector<RecipeIngredient> ingredients;
    std::string description;

    bool canCraft(const Inventory& inv) const {
        for (const auto& ing : ingredients) {
            if (inv.countItem(ing.itemId) < ing.count) return false;
        }
        return true;
    }

    uint32_t maxCraftable(const Inventory& inv) const {
        uint32_t maxTimes = 999;
        for (const auto& ing : ingredients) {
            if (ing.count == 0) continue;
            uint32_t available = inv.countItem(ing.itemId) / ing.count;
            if (available < maxTimes) maxTimes = available;
        }
        return maxTimes;
    }
};

class CraftingRegistry {
public:
    static void init();
    static const std::vector<Recipe>& getAll();
    static const Recipe* get(uint32_t recipeId);
    static std::vector<Recipe> getByCategory(const std::string& category);
    static bool craft(uint32_t recipeId, Inventory& inv, uint32_t times = 1);

    // Minecraft Grid Crafting (2x2 and 3x3)
    static ItemStack matchGrid(const std::vector<ItemStack>& grid, int gridW, int gridH, uint32_t* matchedRecipeId = nullptr);
    static void consumeGrid(std::vector<ItemStack>& grid);

private:
    static void registerRecipe(const Recipe& recipe);
    static void registerGridRecipe(const GridRecipe& recipe);

    static std::vector<Recipe> recipes;
    static std::vector<GridRecipe> gridRecipes;
    static bool initialized;
};

} // namespace Aetheria
