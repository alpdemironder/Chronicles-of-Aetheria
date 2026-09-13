#include "CraftingRegistry.hpp"
#include <iostream>
#include <algorithm>

namespace Aetheria {

std::vector<Recipe> CraftingRegistry::recipes;
std::vector<GridRecipe> CraftingRegistry::gridRecipes;
bool CraftingRegistry::initialized = false;

void CraftingRegistry::registerRecipe(const Recipe& recipe) {
    recipes.push_back(recipe);
}

void CraftingRegistry::registerGridRecipe(const GridRecipe& recipe) {
    gridRecipes.push_back(recipe);
}

const std::vector<Recipe>& CraftingRegistry::getAll() {
    if (!initialized) init();
    return recipes;
}

const Recipe* CraftingRegistry::get(uint32_t recipeId) {
    if (!initialized) init();
    for (const auto& r : recipes) {
        if (r.id == recipeId) return &r;
    }
    return nullptr;
}

std::vector<Recipe> CraftingRegistry::getByCategory(const std::string& category) {
    if (!initialized) init();
    if (category == "All" || category.empty()) {
        return recipes;
    }

    std::vector<Recipe> filtered;
    for (const auto& r : recipes) {
        if (r.category == category) {
            filtered.push_back(r);
        }
    }
    return filtered;
}

bool CraftingRegistry::craft(uint32_t recipeId, Inventory& inv, uint32_t times) {
    const Recipe* r = get(recipeId);
    if (!r || times == 0) return false;

    // Verify all ingredients available
    for (const auto& ing : r->ingredients) {
        uint32_t needed = ing.count * times;
        if (inv.countItem(ing.itemId) < needed) {
            return false;
        }
    }

    // Deduct ingredients
    for (const auto& ing : r->ingredients) {
        inv.removeItem(ing.itemId, ing.count * times);
    }

    // Add result to inventory
    uint32_t totalResult = r->result.count * times;
    inv.addItem(r->result.id, totalResult);

    return true;
}

ItemStack CraftingRegistry::matchGrid(const std::vector<ItemStack>& grid, int gridW, int gridH, uint32_t* matchedRecipeId) {
    if (!initialized) init();
    if (matchedRecipeId) *matchedRecipeId = 0;

    // 1. Check shapeless recipes
    std::vector<uint16_t> activeItems;
    for (const auto& s : grid) {
        if (!s.isEmpty()) activeItems.push_back(s.id);
    }
    if (activeItems.empty()) return ItemStack();

    std::sort(activeItems.begin(), activeItems.end());

    for (const auto& gr : gridRecipes) {
        if (gr.isShapeless) {
            std::vector<uint16_t> ing = gr.shapelessIngredients;
            std::sort(ing.begin(), ing.end());
            if (activeItems == ing) {
                if (matchedRecipeId) *matchedRecipeId = gr.id;
                return gr.result;
            }
        }
    }

    // 2. Shaped matching: find non-empty bounding box
    int minR = gridH, maxR = -1;
    int minC = gridW, maxC = -1;

    for (int r = 0; r < gridH; ++r) {
        for (int c = 0; c < gridW; ++c) {
            if (!grid[r * gridW + c].isEmpty()) {
                if (r < minR) minR = r;
                if (r > maxR) maxR = r;
                if (c < minC) minC = c;
                if (c > maxC) maxC = c;
            }
        }
    }

    if (maxR == -1) return ItemStack(); // Completely empty

    int subW = maxC - minC + 1;
    int subH = maxR - minR + 1;

    for (const auto& gr : gridRecipes) {
        if (gr.isShapeless) continue;
        if (gr.patternW != subW || gr.patternH != subH) continue;

        // Check exact match
        bool matched = true;
        for (int dr = 0; dr < subH && matched; ++dr) {
            for (int dc = 0; dc < subW && matched; ++dc) {
                uint16_t required = gr.pattern[dr * subW + dc];
                uint16_t actual = grid[(minR + dr) * gridW + (minC + dc)].id;
                if (required != actual) matched = false;
            }
        }

        // Check horizontal mirror if not matched
        if (!matched) {
            matched = true;
            for (int dr = 0; dr < subH && matched; ++dr) {
                for (int dc = 0; dc < subW && matched; ++dc) {
                    uint16_t required = gr.pattern[dr * subW + (subW - 1 - dc)];
                    uint16_t actual = grid[(minR + dr) * gridW + (minC + dc)].id;
                    if (required != actual) matched = false;
                }
            }
        }

        if (matched) {
            if (matchedRecipeId) *matchedRecipeId = gr.id;
            return gr.result;
        }
    }

    return ItemStack();
}

void CraftingRegistry::consumeGrid(std::vector<ItemStack>& grid) {
    for (auto& s : grid) {
        if (!s.isEmpty()) {
            s.count--;
            if (s.count == 0) s.clear();
        }
    }
}

void CraftingRegistry::init() {
    if (initialized) return;
    initialized = true;

    recipes.clear();
    gridRecipes.clear();

    // ====================================================
    // SHAPELESS RECIPES (Planks, Torches, Bread, etc.)
    // ====================================================
    // 1 Wood Log -> 4 Wood Planks
    // 1 Wood Log -> 4 Wood Planks (All 16 Wood Species)
    const char* speciesNames[16] = {
        "Oak", "Birch", "Spruce", "Pine", "Jungle", "Acacia", "Dark Oak", "Mangrove",
        "Cherry", "Maple", "Willow", "Fungal", "Petrified", "Bamboo", "Palm", "Astral"
    };
    uint16_t logTypes[16];
    uint16_t plankTypes[16];
    for (int i = 0; i < 16; ++i) {
        logTypes[i] = static_cast<uint16_t>(71 + i * 3);
        plankTypes[i] = static_cast<uint16_t>(72 + i * 3);
    }

    for (int i = 0; i < 16; ++i) {
        GridRecipe gr;
        gr.id = 1000 + i;
        gr.name = std::string(speciesNames[i]) + " Planks";
        gr.isShapeless = true;
        gr.shapelessIngredients = { logTypes[i] };
        gr.result = { plankTypes[i], 4, 64 };
        registerGridRecipe(gr);
    }

    // 1 Coal + 1 Stick -> 4 Torches
    {
        GridRecipe gr;
        gr.id = 110;
        gr.name = "Torches";
        gr.isShapeless = true;
        gr.shapelessIngredients = { 552, 550 };
        gr.result = { 551, 4, 64 };
        registerGridRecipe(gr);
    }
    // 1 Charcoal + 1 Stick -> 4 Torches
    {
        GridRecipe gr;
        gr.id = 111;
        gr.name = "Torches";
        gr.isShapeless = true;
        gr.shapelessIngredients = { 553, 550 };
        gr.result = { 551, 4, 64 };
        registerGridRecipe(gr);
    }

    // 3 Wheat -> 1 Bread (Shapeless or 3x1)
    {
        GridRecipe gr;
        gr.id = 112;
        gr.name = "Bread";
        gr.isShapeless = true;
        gr.shapelessIngredients = { 559, 559, 559 };
        gr.result = { 560, 1, 32 };
        registerGridRecipe(gr);
    }

    // 1 Bone Shards -> 3 Bone Meal (Shapeless)
    {
        GridRecipe gr;
        gr.id = 113;
        gr.name = "Bone Meal";
        gr.isShapeless = true;
        gr.shapelessIngredients = { 509 };
        gr.result = { 525, 3, 64 };
        registerGridRecipe(gr);
    }

    // ====================================================
    // 2x2 SHAPED RECIPES (Sticks, Crafting Table, Bricks)
    // ====================================================
    // 2 Planks vertical -> 4 Wooden Sticks (All 16 Wood Species)
    for (int i = 0; i < 16; ++i) {
        GridRecipe gr;
        gr.id = 2000 + i;
        gr.name = std::string(speciesNames[i]) + " Wooden Sticks";
        gr.patternW = 1;
        gr.patternH = 2;
        gr.pattern = { plankTypes[i], plankTypes[i] };
        gr.result = { 550, 4, 64 };
        registerGridRecipe(gr);
    }

    // 4 Planks 2x2 -> 1 Crafting Table (331) (All 16 Wood Species)
    for (int i = 0; i < 16; ++i) {
        GridRecipe gr;
        gr.id = 2100 + i;
        gr.name = std::string(speciesNames[i]) + " Crafting Table";
        gr.patternW = 2;
        gr.patternH = 2;
        gr.pattern = { plankTypes[i], plankTypes[i],
                       plankTypes[i], plankTypes[i] };
        gr.result = { 331, 1, 64 };
        registerGridRecipe(gr);
    }

    // 4 Stone 2x2 -> 4 Stone Bricks (39)
    {
        GridRecipe gr;
        gr.id = 220;
        gr.name = "Stone Bricks";
        gr.patternW = 2;
        gr.patternH = 2;
        gr.pattern = { 36, 36,
                       36, 36 };
        gr.result = { 39, 4, 64 };
        registerGridRecipe(gr);
    }

    // ====================================================
    // 3x3 SHAPED RECIPES (Furnace, Chest, Weapons, Tools, Armor)
    // ====================================================
    // 8 Cobblestone -> 1 Furnace (Ocak) (332)
    {
        GridRecipe gr;
        gr.id = 300;
        gr.name = "Furnace";
        gr.patternW = 3;
        gr.patternH = 3;
        gr.pattern = { 37, 37, 37,
                       37,  0, 37,
                       37, 37, 37 };
        gr.result = { 332, 1, 64 };
        registerGridRecipe(gr);
    }

    // 8 Planks -> 1 Storage Chest (337) across all 16 wood species!
    uint16_t allWoodPlanks[] = { 72, 75, 78, 81, 84, 87, 89, 92, 95, 98, 101, 104, 107, 110, 113, 116 };
    for (size_t p = 0; p < 16; ++p) {
        GridRecipe gr;
        gr.id = 301 + static_cast<uint32_t>(p);
        gr.name = "Chest";
        gr.patternW = 3;
        gr.patternH = 3;
        uint16_t pl = allWoodPlanks[p];
        gr.pattern = { pl, pl, pl,
                       pl,  0, pl,
                       pl, pl, pl };
        gr.result = { 337, 1, 64 };
        registerGridRecipe(gr);
    }

    // --- WEAPONS (SWORDS: 1x3: Mat, Mat, Stick) ---
    struct WeaponRecipeDef {
        uint16_t mat;
        uint16_t res;
    };
    WeaponRecipeDef swordTiers[] = {
        { 72,  510 }, // Wood Sword
        { 37,  570 }, // Stone Sword
        { 501, 513 }, // Iron Sword
        { 503, 571 }, // Gold Sword
        { 554, 572 }, // Diamond Sword
        { 524, 517 }  // Aetherium Greatsword
    };
    for (int i = 0; i < 6; ++i) {
        GridRecipe gr;
        gr.id = 400 + i;
        gr.name = "Sword";
        gr.patternW = 1;
        gr.patternH = 3;
        gr.pattern = { swordTiers[i].mat,
                       swordTiers[i].mat,
                       550 }; // Stick at bottom
        gr.result = { swordTiers[i].res, 1, 1 };
        registerGridRecipe(gr);
    }

    // --- TOOLS (PICKAXES: 3x3: Mat Mat Mat / 0 Stick 0 / 0 Stick 0) ---
    WeaponRecipeDef pickTiers[] = {
        { 72,  573 }, // Wood Pickaxe
        { 37,  512 }, // Stone Pickaxe
        { 501, 514 }, // Iron Pickaxe
        { 503, 574 }, // Gold Pickaxe
        { 554, 575 }, // Diamond Pickaxe
        { 524, 576 }  // Aetherium Pickaxe
    };
    for (int i = 0; i < 6; ++i) {
        GridRecipe gr;
        gr.id = 410 + i;
        gr.name = "Pickaxe";
        gr.patternW = 3;
        gr.patternH = 3;
        gr.pattern = { pickTiers[i].mat, pickTiers[i].mat, pickTiers[i].mat,
                                      0,              550,                0,
                                      0,              550,                0 };
        gr.result = { pickTiers[i].res, 1, 1 };
        registerGridRecipe(gr);
    }

    // --- TOOLS (AXES: 2x3: Mat Mat / Mat Stick / 0 Stick) ---
    WeaponRecipeDef axeTiers[] = {
        { 72,  577 }, // Wood Axe
        { 37,  511 }, // Stone Axe
        { 501, 578 }, // Iron Axe
        { 554, 579 }  // Diamond Axe
    };
    for (int i = 0; i < 4; ++i) {
        GridRecipe gr;
        gr.id = 420 + i;
        gr.name = "Axe";
        gr.patternW = 2;
        gr.patternH = 3;
        gr.pattern = { axeTiers[i].mat, axeTiers[i].mat,
                       axeTiers[i].mat,             550,
                                     0,             550 };
        gr.result = { axeTiers[i].res, 1, 1 };
        registerGridRecipe(gr);
    }

    // --- TOOLS (SHOVELS: 1x3: Mat / Stick / Stick) ---
    WeaponRecipeDef shovelTiers[] = {
        { 72,  580 }, // Wood Shovel
        { 37,  581 }, // Stone Shovel
        { 501, 582 }, // Iron Shovel
        { 554, 583 }  // Diamond Shovel
    };
    for (int i = 0; i < 4; ++i) {
        GridRecipe gr;
        gr.id = 430 + i;
        gr.name = "Shovel";
        gr.patternW = 1;
        gr.patternH = 3;
        gr.pattern = { shovelTiers[i].mat,
                                      550,
                                      550 };
        gr.result = { shovelTiers[i].res, 1, 1 };
        registerGridRecipe(gr);
    }

    // Bow (515): 3x3
    {
        GridRecipe gr;
        gr.id = 440;
        gr.name = "Hunter Bow";
        gr.patternW = 3;
        gr.patternH = 3;
        gr.pattern = {   0, 550, 556,
                       550,   0, 556,
                         0, 550, 556 };
        gr.result = { 515, 1, 1 };
        registerGridRecipe(gr);
    }

    // Arrows (516): 1x3: Flint (558), Stick (550), Feather (557) -> 4 Arrows
    {
        GridRecipe gr;
        gr.id = 441;
        gr.name = "Wooden Arrows";
        gr.patternW = 1;
        gr.patternH = 3;
        gr.pattern = { 558,
                       550,
                       557 };
        gr.result = { 516, 4, 128 };
        registerGridRecipe(gr);
    }

    // --- ARMOR (HELMETS, CHESTS, LEGS, BOOTS) ---
    struct ArmorCraftTiers {
        uint16_t mat;
        uint16_t helm;
        uint16_t chest;
        uint16_t legs;
        uint16_t boots;
    };
    ArmorCraftTiers armorTiers[] = {
        { 506, 530, 531, 532, 533 }, // Leather
        { 501, 534, 535, 536, 537 }, // Iron
        { 503, 538, 539, 540, 541 }, // Gold
        { 554, 542, 543, 544, 545 }, // Diamond
        { 524, 546, 547, 548, 549 }  // Aetherium
    };

    for (int i = 0; i < 5; ++i) {
        // Helmet (3x2: Mat Mat Mat / Mat 0 Mat)
        {
            GridRecipe gr;
            gr.id = 500 + i * 4;
            gr.name = "Helmet";
            gr.patternW = 3;
            gr.patternH = 2;
            gr.pattern = { armorTiers[i].mat, armorTiers[i].mat, armorTiers[i].mat,
                           armorTiers[i].mat,                 0, armorTiers[i].mat };
            gr.result = { armorTiers[i].helm, 1, 1 };
            registerGridRecipe(gr);
        }
        // Chestplate (3x3: Mat 0 Mat / Mat Mat Mat / Mat Mat Mat)
        {
            GridRecipe gr;
            gr.id = 501 + i * 4;
            gr.name = "Chestplate";
            gr.patternW = 3;
            gr.patternH = 3;
            gr.pattern = { armorTiers[i].mat,                 0, armorTiers[i].mat,
                           armorTiers[i].mat, armorTiers[i].mat, armorTiers[i].mat,
                           armorTiers[i].mat, armorTiers[i].mat, armorTiers[i].mat };
            gr.result = { armorTiers[i].chest, 1, 1 };
            registerGridRecipe(gr);
        }
        // Leggings (3x3: Mat Mat Mat / Mat 0 Mat / Mat 0 Mat)
        {
            GridRecipe gr;
            gr.id = 502 + i * 4;
            gr.name = "Leggings";
            gr.patternW = 3;
            gr.patternH = 3;
            gr.pattern = { armorTiers[i].mat, armorTiers[i].mat, armorTiers[i].mat,
                           armorTiers[i].mat,                 0, armorTiers[i].mat,
                           armorTiers[i].mat,                 0, armorTiers[i].mat };
            gr.result = { armorTiers[i].legs, 1, 1 };
            registerGridRecipe(gr);
        }
        // Boots (3x2: Mat 0 Mat / Mat 0 Mat)
        {
            GridRecipe gr;
            gr.id = 503 + i * 4;
            gr.name = "Boots";
            gr.patternW = 3;
            gr.patternH = 2;
            gr.pattern = { armorTiers[i].mat, 0, armorTiers[i].mat,
                           armorTiers[i].mat, 0, armorTiers[i].mat };
            gr.result = { armorTiers[i].boots, 1, 1 };
            registerGridRecipe(gr);
        }
    }

    // Golden Apple: 8 Gold Ingots (503) around Apple (561)
    {
        GridRecipe gr;
        gr.id = 600;
        gr.name = "Golden Apple";
        gr.patternW = 3;
        gr.patternH = 3;
        gr.pattern = { 503, 503, 503,
                       503, 561, 503,
                       503, 503, 503 };
        gr.result = { 562, 1, 16 };
        registerGridRecipe(gr);
    }

    // Also populate standard quick-craft recipes in `recipes` for the book catalog!
    registerRecipe({ 1, "Oak Wood Planks", "Materials", { 72, 4, 64 }, { { 71, 1 } }, "4 Planks from Oak Log." });
    registerRecipe({ 2, "Wooden Sticks", "Materials", { 550, 4, 64 }, { { 72, 2 } }, "4 Sticks from 2 Planks." });
    registerRecipe({ 3, "Torches", "Survival", { 551, 4, 64 }, { { 552, 1 }, { 550, 1 } }, "4 bright Torches." });
    registerRecipe({ 4, "Crafting Table", "Survival", { 331, 1, 64 }, { { 72, 4 } }, "Workstation for 3x3 recipes." });
    registerRecipe({ 5, "Furnace", "Survival", { 332, 1, 64 }, { { 37, 8 } }, "Smelts ores into ingots and cooks meat." });
    registerRecipe({ 6, "Chest", "Survival", { 337, 1, 64 }, { { 72, 8 } }, "Storage chest for items." });
    registerRecipe({ 7, "Iron Broadsword", "Weapons", { 513, 1, 1 }, { { 501, 2 }, { 550, 1 } }, "Keen steel sword [Attack: 38]." });
    registerRecipe({ 8, "Iron Pickaxe", "Tools", { 514, 1, 1 }, { { 501, 3 }, { 550, 2 } }, "Steel pickaxe [Attack: 24]." });
    registerRecipe({ 9, "Iron Chestplate", "Armor", { 535, 1, 1 }, { { 501, 8 } }, "Solid breastplate [Defense: +6]." });
    registerRecipe({ 10, "Diamond Sword", "Weapons", { 572, 1, 1 }, { { 554, 2 }, { 550, 1 } }, "Lethal blade [Attack: 52]." });
    registerRecipe({ 11, "Diamond Pickaxe", "Tools", { 575, 1, 1 }, { { 554, 3 }, { 550, 2 } }, "Faceted pickaxe [Attack: 36]." });
    registerRecipe({ 12, "Diamond Chestplate", "Armor", { 543, 1, 1 }, { { 554, 8 } }, "Crystalline cuirass [Defense: +8]." });
    registerRecipe({ 13, "Baked Bread", "Survival", { 560, 1, 32 }, { { 559, 3 } }, "Hearty loaf restores 25 HP & 40 STA." });
    registerRecipe({ 14, "Hunter Bow", "Weapons", { 515, 1, 1 }, { { 550, 3 }, { 556, 3 } }, "Recurve bow for range." });
    registerRecipe({ 15, "Wooden Arrows", "Weapons", { 516, 4, 128 }, { { 558, 1 }, { 550, 1 }, { 557, 1 } }, "4 fletched arrows." });
    registerRecipe({ 16, "Health Potion", "Survival", { 520, 2, 16 }, { { 504, 2 }, { 508, 1 } }, "2 Restorative healing elixirs." });
    registerRecipe({ 17, "Pal Sphere", "Survival", { 523, 2, 16 }, { { 161, 1 }, { 501, 1 }, { 524, 1 } }, "Taming spheres for creatures." });
    registerRecipe({ 18, "Bone Meal", "Materials", { 525, 3, 64 }, { { 509, 1 } }, "Instantly matures planted saplings." });
    registerRecipe({ 19, "Mega Sphere", "Survival", { 526, 2, 16 }, { { 523, 2 }, { 501, 2 }, { 554, 1 } }, "2.0x power high-grade capture spheres." });
    registerRecipe({ 20, "Giga Sphere", "Survival", { 527, 2, 16 }, { { 526, 2 }, { 503, 2 }, { 507, 1 } }, "3.5x power masterwork capture spheres." });

    std::cout << "CraftingRegistry initialized with " << gridRecipes.size() << " Minecraft grid recipes + " << recipes.size() << " quick recipes!" << std::endl;
}

} // namespace Aetheria
