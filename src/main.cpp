#include <windows.h>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

#include "core/Math.hpp"
#include "core/Window.hpp"
#include "core/Timer.hpp"
#include "core/Audio.hpp"
#include "core/Camera.hpp"
#include "core/Settings.hpp"

#include "render/GLHeaders.hpp"
#include "render/GLContext.hpp"
#include "render/GLPipeline.hpp"
#include "render/GLBuffer.hpp"
#include "render/TextureAtlas.hpp"

#include "world/BlockRegistry.hpp"
#include "world/BiomeRegistry.hpp"
#include "world/World.hpp"

#include "building/StructurePiece.hpp"
#include "building/BuildingManager.hpp"

#include "entities/Player.hpp"
#include "entities/Creature.hpp"
#include "entities/CreatureRegistry.hpp"
#include "entities/ItemEntity.hpp"
#include "entities/CaptureSphere.hpp"
#include "entities/MobSpawner.hpp"
#include "inventory/ItemRegistry.hpp"
#include "inventory/CraftingRegistry.hpp"

#include "ui/UIRenderer.hpp"
#include "ui/HUD.hpp"
#include "ui/InventoryUI.hpp"
#include "ui/BuildMenuUI.hpp"
#include "ui/BlockCatalogUI.hpp"
#include "ui/BestiaryUI.hpp"
#include "ui/BiomeCodexUI.hpp"
#include "ui/SettingsUI.hpp"
#include "render/IrisShaderManager.hpp"
#include "ui/IrisShaderUI.hpp"
#include "ui/UpdateCalendarUI.hpp"
#include "ui/MainMenuUI.hpp"

using namespace Aetheria;

int main(int argc, char* argv[]) {
    std::cout << "========================================================\n"
              << "       CHRONICLES OF AETHERIA (OpenGL RPG Sandbox)      \n"
              << "  Featuring: 365 Blocks | 35 Biomes | Palworld Building  \n"
              << "         Interactive Settings Menu & Full Customization  \n"
              << "========================================================\n"
              << std::endl;

    // 1. Initialize Registries & Settings
    BlockRegistry::init();
    BiomeRegistry::init();
    CreatureRegistry::init();
    StructureRegistry::init();
    ItemRegistry::init();
    CraftingRegistry::init();

    Settings& config = Settings::instance();
    config.load();

    // 2. Initialize Platform Window & Audio
    auto window = std::make_unique<Window>("Chronicles of Aetheria (OpenGL 3.3 3D RPG)", 1280, 720);
    auto audio = std::make_unique<AudioEngine>();
    Timer timer;
    Camera camera;

    // 3. Initialize Modern OpenGL Context & Pipelines
    auto glContext = std::make_unique<GLContext>(window.get());
    if (!glContext->isInitialized()) {
        std::cerr << "CRITICAL: OpenGL context creation failed!" << std::endl;
        MessageBoxA(window->getHandle(), "Failed to initialize OpenGL! Please check your GPU drivers.", "OpenGL Error", MB_ICONERROR);
        return 1;
    }

    auto glPipeline = std::make_unique<GLPipeline>();
    if (!glPipeline->init()) {
        std::cerr << "CRITICAL: Failed to compile OpenGL GLSL shaders!" << std::endl;
        MessageBoxA(window->getHandle(), "Failed to compile OpenGL shaders!", "Shader Error", MB_ICONERROR);
        return 1;
    }

    auto textureAtlas = std::make_unique<TextureAtlas>();
    if (!textureAtlas->init()) {
        std::cerr << "CRITICAL: Failed to initialize TextureAtlas!" << std::endl;
        MessageBoxA(window->getHandle(), "Failed to initialize 16x16 Texture Atlas!", "Texture Error", MB_ICONERROR);
        return 1;
    }

    window->setResizeCallback([&](int w, int h) {
        glContext->resize(w, h);
    });

    // 4. World & Palworld Building System
    auto world = std::make_unique<World>(1337);
    auto buildingMgr = std::make_unique<BuildingManager>(world.get(), audio.get());

    // 5. Player Setup on Ground Surface & Initial RPG Starter Kit
    int spawnGroundY = world->getHighestBlock(0, 0);
    Player player(Vec3(0.0f, static_cast<float>(spawnGroundY) + 1.8f, 0.0f));
    camera.setPosition(player.getPosition());

    // Populate initial starter kit so hotbar and inventory are immediately usable & visible
    Inventory& pInv = player.getInventory();
    pInv.setSlot(0, ItemStack(513, 1));  // Iron Broadsword (38 Atk)
    pInv.setSlot(1, ItemStack(514, 1));  // Iron Pickaxe (Tier 3)
    pInv.setSlot(2, ItemStack(578, 1));  // Iron Axe (32 Atk)
    pInv.setSlot(3, ItemStack(72, 64));  // Oak Wood Planks (Block #72)
    pInv.setSlot(4, ItemStack(37, 64));  // Cobblestone (Block #37)
    pInv.setSlot(5, ItemStack(551, 32)); // Torches
    pInv.setSlot(6, ItemStack(522, 16)); // Cooked Feast (Restores 40 HP & 100 Stamina)
    pInv.setSlot(7, ItemStack(331, 1));  // Crafting Table Workstation
    pInv.setSlot(8, ItemStack(332, 1));  // Furnace (Ocak)

    // Equipped Starting Armor
    pInv.setSlot(Inventory::SLOT_HELMET, ItemStack(530, 1)); // Leather Cap
    pInv.setSlot(Inventory::SLOT_CHEST,  ItemStack(531, 1)); // Leather Tunic
    pInv.setSlot(Inventory::SLOT_LEGS,   ItemStack(532, 1)); // Leather Pants
    pInv.setSlot(Inventory::SLOT_BOOTS,  ItemStack(533, 1)); // Leather Boots

    // Backpack Supplies
    pInv.setSlot(9,  ItemStack(501, 32)); // Iron Ingots
    pInv.setSlot(10, ItemStack(503, 16)); // Gold Ingots
    pInv.setSlot(11, ItemStack(552, 32)); // Coal
    pInv.setSlot(12, ItemStack(520, 8));  // Health Potions
    pInv.setSlot(13, ItemStack(523, 10)); // Pal Spheres
    pInv.setSlot(14, ItemStack(515, 1));  // Hunter Bow
    pInv.setSlot(15, ItemStack(516, 64)); // Arrows
    pInv.setSlot(16, ItemStack(550, 32)); // Wooden Sticks

    // 6. Dynamic Mob Spawner & Safe Surface Initial Population
    MobSpawner mobSpawner;
    std::vector<std::unique_ptr<Creature>> creatures;
    mobSpawner.spawnInitial(world.get(), player.getPosition(), creatures);

    // 7. UI & Iris Shader Subsystems
    auto irisManager = std::make_unique<IrisShaderManager>();
    irisManager->init(window->getWidth(), window->getHeight());
    irisManager->setMotionBlur(config.graphics.motionBlur);
    irisManager->setMotionBlurStrength(config.graphics.motionBlurStrength);
    IrisShaderUI irisShaderUI(irisManager.get());

    auto uiRenderer = std::make_unique<UIRenderer>();
    HUD hud(uiRenderer.get());
    InventoryUI inventoryMenu;
    BuildMenuUI buildMenu(uiRenderer.get());
    BlockCatalogUI blockCatalog(uiRenderer.get());
    BestiaryUI bestiary(uiRenderer.get());
    BiomeCodexUI biomeCodex(uiRenderer.get());
    SettingsUI settingsMenu;
    UpdateCalendarUI updateCalendar(uiRenderer.get());

    settingsMenu.setOpenIrisCallback([&]() {
        settingsMenu.setOpen(false);
        irisShaderUI.setOpen(true);
    });

    enum class GameState { MainMenu, Playing };
    GameState gameState = GameState::MainMenu;

    MainMenuUI mainMenu;
    mainMenu.setOnPlay([&]() {
        gameState = GameState::Playing;
        window->setCursorLocked(true);
    });
    mainMenu.setOnOpenCalendar([&]() {
        updateCalendar.open();
    });
    mainMenu.setOnOpenShaders([&]() {
        irisShaderUI.setOpen(true);
    });
    mainMenu.setOnOpenSettings([&]() {
        settingsMenu.setOpen(true);
    });
    mainMenu.setOnQuit([&]() {
        PostQuitMessage(0);
    });

    settingsMenu.setQuitToTitleCallback([&]() {
        gameState = GameState::MainMenu;
        window->setCursorLocked(false);
    });

    GLBuffer creatureBuffer;
    GLBuffer itemBuffer;
    std::vector<std::unique_ptr<ItemEntity>> droppedItems;
    std::vector<std::unique_ptr<CaptureSphere>> captureSpheres;

    // Minecraft-Style Continuous Block Mining State
    float miningProgress = 0.0f;
    IVec3 currentMiningPos{0, -999, 0};
    uint16_t currentMiningId = 0;
    float miningSwingTimer = 0.0f;

    window->setCursorLocked(false);

    std::cout << "\nGame started on Main Menu! WASD to move, [C] Zoom, [O] Iris Shaders, [E] Inventory, [ESC] Pause!\n" << std::endl;

    // ----------------------------------------------------
    // MAIN ENGINE LOOP
    // ----------------------------------------------------
    while (!window->shouldClose()) {
        if (!window->processMessages()) break;

        timer.update();
        float dt = timer.getDeltaTime();
        float totalTime = timer.getTotalTime();

        // ----------------------------------------------------
        // MENU TOGGLE & ESC HIERARCHY
        // ----------------------------------------------------
        // ESC key: Priority close for any active menu, or open Settings if in gameplay
        if (window->isKeyPressed(VK_ESCAPE)) {
            if (updateCalendar.getIsOpen()) {
                updateCalendar.close();
            } else if (irisShaderUI.isOpen()) {
                irisShaderUI.setOpen(false);
            } else if (settingsMenu.isOpen()) {
                settingsMenu.setOpen(false);
            } else if (gameState == GameState::MainMenu) {
                // At main menu, nothing else to close
            } else if (inventoryMenu.getIsOpen()) {
                inventoryMenu.returnGridItemsToPlayer(player);
                inventoryMenu.close();
            } else if (buildMenu.getIsOpen()) {
                buildMenu.close();
            } else if (blockCatalog.getIsOpen()) {
                blockCatalog.close();
            } else if (bestiary.getIsOpen()) {
                bestiary.close();
            } else if (biomeCodex.getIsOpen()) {
                biomeCodex.close();
            } else if (buildingMgr->getIsBuilding() || buildingMgr->getIsDismantling()) {
                buildingMgr->setBuildMode(false);
                buildingMgr->setDismantleMode(false);
            } else {
                settingsMenu.setOpen(true);
            }
        }

        // Gameplay hotkeys (only active when playing, not in Main Menu)
        if (gameState == GameState::Playing) {
            // 'E', 'TAB', 'I' keys: Toggle Inventory & Crafting UI
            if (window->isKeyPressed('E') || window->isKeyPressed('I') || window->isKeyPressed(VK_TAB)) {
                if (irisShaderUI.isOpen()) {
                    irisShaderUI.setOpen(false);
                    inventoryMenu.openPlayerInventory();
                } else if (settingsMenu.isOpen()) {
                    settingsMenu.setOpen(false);
                    inventoryMenu.openPlayerInventory();
                } else if (inventoryMenu.getIsOpen()) {
                    inventoryMenu.returnGridItemsToPlayer(player);
                    inventoryMenu.close();
                } else {
                    buildMenu.close();
                    blockCatalog.close();
                    bestiary.close();
                    biomeCodex.close();
                    buildingMgr->setBuildMode(false);
                    buildingMgr->setDismantleMode(false);
                    inventoryMenu.openPlayerInventory();
                }
            }

            // 'O' key quick toggle for Iris Shaderpacks Menu
            if (window->isKeyPressed('O')) {
                irisShaderUI.toggle();
                if (irisShaderUI.isOpen()) {
                    settingsMenu.setOpen(false);
                    inventoryMenu.returnGridItemsToPlayer(player);
                    inventoryMenu.close();
                    buildMenu.close();
                    blockCatalog.close();
                    bestiary.close();
                    biomeCodex.close();
                    buildingMgr->setBuildMode(false);
                    buildingMgr->setDismantleMode(false);
                }
            }

            // Sub-menu keys (B = Build, N = 365 Blocks, M = Bestiary, J = Biomes, K = Calendar, X = Dismantle)
            if (!updateCalendar.getIsOpen() && !irisShaderUI.isOpen() && !settingsMenu.isOpen() && !inventoryMenu.getIsOpen()) {
                if (window->isKeyPressed('B')) {
                    buildMenu.toggle();
                    if (buildMenu.getIsOpen()) {
                        blockCatalog.close();
                        bestiary.close();
                        biomeCodex.close();
                        buildingMgr->setBuildMode(false);
                        buildingMgr->setDismantleMode(false);
                    }
                }
                if (window->isKeyPressed('N')) {
                    blockCatalog.toggle();
                    if (blockCatalog.getIsOpen()) {
                        buildMenu.close();
                        bestiary.close();
                        biomeCodex.close();
                        buildingMgr->setBuildMode(false);
                        buildingMgr->setDismantleMode(false);
                    }
                }
                if (window->isKeyPressed('M')) {
                    bestiary.toggle();
                    if (bestiary.getIsOpen()) {
                        buildMenu.close();
                        blockCatalog.close();
                        biomeCodex.close();
                        buildingMgr->setBuildMode(false);
                        buildingMgr->setDismantleMode(false);
                    }
                }
                if (window->isKeyPressed('J')) {
                    biomeCodex.toggle();
                    if (biomeCodex.getIsOpen()) {
                        buildMenu.close();
                        blockCatalog.close();
                        bestiary.close();
                        buildingMgr->setBuildMode(false);
                        buildingMgr->setDismantleMode(false);
                    }
                }
                if (window->isKeyPressed('K')) {
                    updateCalendar.toggle();
                    if (updateCalendar.getIsOpen()) {
                        buildMenu.close();
                        blockCatalog.close();
                        bestiary.close();
                        biomeCodex.close();
                        buildingMgr->setBuildMode(false);
                        buildingMgr->setDismantleMode(false);
                    }
                }
                if (window->isKeyPressed('X')) {
                    buildingMgr->setDismantleMode(!buildingMgr->getIsDismantling());
                    if (buildingMgr->getIsDismantling()) {
                        buildingMgr->setBuildMode(false);
                        buildMenu.close();
                    }
                }
                if (window->isKeyPressed(VK_F5)) {
                    config.gameplay.thirdPerson = !config.gameplay.thirdPerson;
                }
            }
        }

        bool anyMenuOpen = (gameState == GameState::MainMenu) || updateCalendar.getIsOpen() || irisShaderUI.isOpen() || settingsMenu.isOpen() ||
                           inventoryMenu.getIsOpen() || buildMenu.getIsOpen() ||
                           blockCatalog.getIsOpen() || bestiary.getIsOpen() || biomeCodex.getIsOpen();

        window->setCursorLocked(!anyMenuOpen);

        // Sync Dynamic Settings (only apply when changed to prevent driver stutter)
        static bool lastVSync = true;
        if (config.graphics.vsync != lastVSync) {
            glContext->setVSync(config.graphics.vsync);
            lastVSync = config.graphics.vsync;
        }
        static bool lastWireframe = false;
        if (config.graphics.wireframe != lastWireframe) {
            glContext->setWireframe(config.graphics.wireframe);
            lastWireframe = config.graphics.wireframe;
        }
        camera.setThirdPerson(config.gameplay.thirdPerson);
        player.setStepHeight(config.controls.autoStepUp ? config.controls.stepHeight : 0.0f);

        // Dynamic Motion Blur synchronization
        static bool lastIrisMB = true;
        if (irisManager->getMotionBlur() != lastIrisMB) {
            config.graphics.motionBlur = irisManager->getMotionBlur();
            lastIrisMB = irisManager->getMotionBlur();
        } else if (config.graphics.motionBlur != lastIrisMB) {
            irisManager->setMotionBlur(config.graphics.motionBlur);
            lastIrisMB = config.graphics.motionBlur;
        }
        irisManager->setMotionBlurStrength(config.graphics.motionBlurStrength);

        // Update Camera Zoom or Title Panorama
        if (gameState == GameState::MainMenu) {
            camera.addYawPitch(dt * 3.5f, 0.0f);
            camera.setZooming(false);
            camera.updateZoom(config.graphics.fov, dt, false);
        } else {
            bool zoomDown = window->isKeyDown('C') && !anyMenuOpen;
            camera.setZooming(zoomDown);
            camera.updateZoom(config.graphics.fov, dt, player.getIsSprinting());
        }

        // Mouse coordinates & keyboard modifiers
        int mx = window->getMouseX();
        int my = window->getMouseY();
        bool mDown = window->isMouseButtonDown(0);
        bool mClicked = window->isMouseButtonPressed(0);
        bool rClicked = window->isMouseButtonPressed(1);
        bool shiftDown = window->isKeyDown(VK_SHIFT);

        if (anyMenuOpen || gameState == GameState::MainMenu) {
            miningProgress = 0.0f;
            currentMiningPos = {0, -999, 0};
            currentMiningId = 0;
        }

        // Handle Active Menu Input or Gameplay Input
        if (updateCalendar.getIsOpen()) {
            if (window->isKeyPressed('1')) updateCalendar.setViewMode(0);
            if (window->isKeyPressed('2')) updateCalendar.setViewMode(1);
            if (window->isKeyPressed('A') || window->isKeyPressed(VK_LEFT)) updateCalendar.prevMonth();
            if (window->isKeyPressed('D') || window->isKeyPressed(VK_RIGHT)) updateCalendar.nextMonth();
        } else if (gameState == GameState::MainMenu) {
            // Handled during UI rendering
        } else if (irisShaderUI.isOpen()) {
            // Handled during UI rendering
        } else if (settingsMenu.isOpen()) {
            // Handled during UI rendering
        } else if (inventoryMenu.getIsOpen()) {
            inventoryMenu.handleInput(*window, player, audio.get());
        } else if (buildMenu.getIsOpen()) {
            buildMenu.handleInput(*window, *buildingMgr);
        } else if (blockCatalog.getIsOpen()) {
            blockCatalog.handleInput(*window, player);
        } else if (bestiary.getIsOpen()) {
            bestiary.handleInput(*window);
        } else if (biomeCodex.getIsOpen()) {
            biomeCodex.handleInput(*window);
        } else {
            // Gameplay Controls (WASD, Double-tap W Sprint, Shift Crouch, R Dash)
            player.handleInput(*window, camera, audio.get(), dt, buildingMgr->getIsBuilding());

            Ray aimRay(camera.getRenderPosition(), camera.getForward());

            // -----------------------------------------------------------------
            // COMBAT & MINECRAFT-STYLE CONTINUOUS BLOCK MINING
            // -----------------------------------------------------------------
            if (buildingMgr->getIsBuilding()) {
                miningProgress = 0.0f;
                currentMiningPos = {0, -999, 0};
                currentMiningId = 0;
                if (window->isMouseButtonPressed(0)) {
                    buildingMgr->tryPlaceStructure(&player.getInventory());
                }
            } else if (buildingMgr->getIsDismantling()) {
                miningProgress = 0.0f;
                currentMiningPos = {0, -999, 0};
                currentMiningId = 0;
                if (window->isMouseButtonPressed(0)) {
                    buildingMgr->tryDismantle(aimRay, &player.getInventory());
                }
            } else {
                // Check if aiming at any creature in front of player
                Creature* bestTarget = nullptr;
                float closestDist = 999.0f;

                for (auto& c : creatures) {
                    if (!c->getIsDead()) {
                        Vec3 toMob = (c->getPosition() + Vec3(0, c->getDef().size.y * 0.5f, 0)) - camera.getRenderPosition();
                        float dist = toMob.length();
                        if (dist < 3.8f) {
                            Vec3 dirToMob = toMob.normalized();
                            float dot = dirToMob.dot(camera.getForward());
                            if (dot > 0.40f && dist < closestDist) {
                                closestDist = dist;
                                bestTarget = c.get();
                            }
                        }
                    }
                }

                // Immediate Creature Attack on Click (Minecraft 1.9+ Attack Cooldown & Scaling)
                if (bestTarget && window->isMouseButtonPressed(0)) {
                    float charge = player.getAttackRechargeProgress();
                    float rawDmg = player.getAttackPower(); // Pre-scaled by (0.2 + 0.8 * charge^2)

                    // Minecraft 1.9 Critical Hit check:
                    // Player is falling (velocity.y < -0.4), not on ground, not in water, and charge >= 0.85f
                    bool isCrit = (player.getVelocity().y < -0.4f && !player.isGrounded() && !player.getIsInWater() && charge >= 0.85f);
                    if (isCrit) {
                        rawDmg *= 1.50f; // +50% critical damage!
                    }

                    // Reset attack cooldown after computing damage
                    player.triggerAttack();

                    // Knockback scaled by charge (suppressed on rapid spam clicks)
                    Vec3 knockback = camera.getForward();
                    float knockMult = (charge >= 0.85f) ? 1.0f : (0.20f + 0.80f * charge);
                    knockback = knockback * knockMult;

                    // Audio feedback: sharp heavy strike if charged/crit, lighter swish if spamming
                    if (audio) {
                        if (isCrit) {
                            audio->playSound(SoundID::SwordSwing, 1.25f, 1.25f);
                        } else if (charge >= 0.85f) {
                            audio->playSound(SoundID::SwordSwing, 1.0f, 1.0f);
                        } else {
                            audio->playSound(SoundID::SwordSwing, 0.45f, 1.5f);
                        }
                    }

                    bestTarget->takeDamage(rawDmg, knockback, audio.get());
                    miningProgress = 0.0f;
                    currentMiningPos = {0, -999, 0};
                    currentMiningId = 0;

                    // Alert nearby tamed companions to assist master!
                    for (auto& comp : creatures) {
                        if (!comp->getIsDead() && comp->getIsTamed() && comp->getStance() == CompanionStance::Follow) {
                            float d = (comp->getPosition() - player.getPosition()).length();
                            if (d <= 25.0f) {
                                comp->notifyMasterAttacked(bestTarget);
                            }
                        }
                    }

                    if (bestTarget->getIsDead()) {
                        player.gainXP(bestTarget->getDef().xpReward, audio.get());

                        // Award companion XP to nearby tamed companions
                        for (auto& comp : creatures) {
                            if (!comp->getIsDead() && comp->getIsTamed()) {
                                float d = (comp->getPosition() - player.getPosition()).length();
                                if (d <= 25.0f) {
                                    comp->gainCompanionXP(bestTarget->getDef().xpReward * 0.9f, audio.get());
                                }
                            }
                        }

                        // Drop physical 3D creature loot in the world!
                        uint16_t lootId = bestTarget->getDef().dropItemId;
                        uint32_t lootCount = bestTarget->getDef().dropCount;
                        if (lootId == 0) {
                            lootId = 508; // Raw Meat
                            lootCount = 2;
                        }
                        Vec3 mCenter = bestTarget->getPosition() + Vec3(0, 0.5f, 0);
                        for (uint32_t li = 0; li < lootCount; ++li) {
                            Vec3 popVel(
                                (std::rand() % 100 - 50) * 0.04f,
                                3.5f + (std::rand() % 10) * 0.2f,
                                (std::rand() % 100 - 50) * 0.04f
                            );
                            droppedItems.push_back(std::make_unique<ItemEntity>(ItemStack{lootId, 1, 64}, mCenter, popVel));
                        }
                    }
                } else if (!bestTarget && window->isMouseButtonDown(0)) {
                    // Holding LMB to Mine Targeted Voxel Block
                    RaycastResult rHit = world->raycast(aimRay, 5.5f);
                    if (rHit.hit && rHit.blockId != 0 && rHit.blockId != 52) { // 52 = Obsidian Bedrock (Indestructible)
                        if (rHit.hitBlockPos.x != currentMiningPos.x ||
                            rHit.hitBlockPos.y != currentMiningPos.y ||
                            rHit.hitBlockPos.z != currentMiningPos.z) {
                            currentMiningPos = rHit.hitBlockPos;
                            currentMiningId = rHit.blockId;
                            miningProgress = 0.0f;
                            miningSwingTimer = 0.0f;
                        }

                        const BlockDef& bDef = BlockRegistry::get(currentMiningId);
                        const ItemStack& heldItem = player.getHeldItem();
                        const ItemDef& heldDef = ItemRegistry::get(heldItem.id);

                        // Tool affinity speed multiplier
                        float speedMult = 1.0f;
                        bool isEffective = false;
                        if (bDef.toolRequired == 0) {
                            if (heldDef.toolType == ToolType::Shovel && bDef.category == BlockCategory::Terrain) isEffective = true;
                            else if (heldDef.toolType == ToolType::Axe && bDef.category == BlockCategory::Wood) isEffective = true;
                            else if (heldDef.toolType == ToolType::Pickaxe && (bDef.category == BlockCategory::Stone || bDef.category == BlockCategory::Ores)) isEffective = true;
                        } else if (bDef.toolRequired == 1 && heldDef.toolType == ToolType::Pickaxe) {
                            isEffective = true;
                        } else if (bDef.toolRequired == 2 && heldDef.toolType == ToolType::Axe) {
                            isEffective = true;
                        } else if (bDef.toolRequired == 3 && heldDef.toolType == ToolType::Shovel) {
                            isEffective = true;
                        }

                        if (isEffective) {
                            // Wood: 3.5x, Stone: 5.5x, Iron: 7.5x, Gold: 9.5x, Diamond: 12.5x, Aetherium: 18.0x
                            speedMult = 1.5f + heldDef.toolTier * 2.0f;
                            if (heldDef.toolTier == 4) speedMult = 9.5f;
                            if (heldDef.toolTier >= 5) speedMult = 12.5f + (heldDef.toolTier - 5) * 5.0f;
                        } else {
                            // Wrong tool penalty for tough stone and ores
                            if (bDef.toolRequired == 1) speedMult = 0.30f;
                            else speedMult = 1.0f;
                        }

                        float breakDuration = std::max(0.12f, (bDef.hardness * 1.25f) / speedMult);
                        if (bDef.hardness <= 0.06f) breakDuration = 0.10f; // Instant foliage & sapling break

                        // Discrete hit impact sound and arm swing animation
                        miningSwingTimer -= dt;
                        if (miningSwingTimer <= 0.0f) {
                            miningSwingTimer = 0.22f;
                            player.triggerAttack();
                            audio->playSound(SoundID::BlockBreak, 0.45f, 0.75f + miningProgress * 0.4f);
                        }

                        miningProgress += dt / breakDuration;

                        if (miningProgress >= 1.0f) {
                            // Complete fracture: block broken!
                            world->setBlock(currentMiningPos.x, currentMiningPos.y, currentMiningPos.z, 0);
                            audio->playSound(SoundID::BlockBreak, 1.0f, 1.0f);
                            player.gainXP(5, audio.get());

                            uint16_t dropId = (bDef.dropItemId > 0) ? bDef.dropItemId : currentMiningId;
                            uint32_t dropCount = (bDef.dropCount > 0) ? bDef.dropCount : 1;

                            // Check if leaf block
                            bool isLeaf = (currentMiningId == 73 || currentMiningId == 76 || currentMiningId == 79 ||
                                           currentMiningId == 82 || currentMiningId == 85 || currentMiningId == 88 ||
                                           currentMiningId == 91 || currentMiningId == 94 || currentMiningId == 97 ||
                                           (currentMiningId >= 100 && currentMiningId <= 118 && (currentMiningId - 73) % 3 == 0));

                            Vec3 bCenter(currentMiningPos.x + 0.5f, currentMiningPos.y + 0.4f, currentMiningPos.z + 0.5f);

                            if (isLeaf) {
                                // Tree leaf drop tables: Saplings (~18%), Sticks (~15%), Apples (~8%)
                                uint16_t saplingDrop = 366; // Oak default
                                if (currentMiningId == 76) saplingDrop = 367; // Birch
                                else if (currentMiningId == 79 || currentMiningId == 82) saplingDrop = 368; // Spruce / Pine
                                else if (currentMiningId == 97) saplingDrop = 369; // Cherry

                                if ((std::rand() % 100) < 18) {
                                    Vec3 popVel((std::rand() % 100 - 50) * 0.035f, 3.8f + (std::rand() % 10) * 0.15f, (std::rand() % 100 - 50) * 0.035f);
                                    droppedItems.push_back(std::make_unique<ItemEntity>(ItemStack{saplingDrop, 1, 64}, bCenter, popVel));
                                }
                                if ((std::rand() % 100) < 15) {
                                    Vec3 popVel((std::rand() % 100 - 50) * 0.035f, 3.8f + (std::rand() % 10) * 0.15f, (std::rand() % 100 - 50) * 0.035f);
                                    droppedItems.push_back(std::make_unique<ItemEntity>(ItemStack{550, 1, 64}, bCenter, popVel));
                                }
                                if (currentMiningId == 73 && (std::rand() % 100) < 8) {
                                    Vec3 popVel((std::rand() % 100 - 50) * 0.035f, 3.8f + (std::rand() % 10) * 0.15f, (std::rand() % 100 - 50) * 0.035f);
                                    droppedItems.push_back(std::make_unique<ItemEntity>(ItemStack{561, 1, 32}, bCenter, popVel));
                                }
                            } else {
                                for (uint32_t di = 0; di < dropCount; ++di) {
                                    Vec3 popVel((std::rand() % 100 - 50) * 0.035f, 3.8f + (std::rand() % 10) * 0.15f, (std::rand() % 100 - 50) * 0.035f);
                                    droppedItems.push_back(std::make_unique<ItemEntity>(ItemStack{dropId, 1, 64}, bCenter, popVel));
                                }
                            }

                            miningProgress = 0.0f;
                            currentMiningPos = {0, -999, 0};
                            currentMiningId = 0;
                        }
                    } else {
                        // Looking at air, bedrock, or out of reach
                        if (window->isMouseButtonPressed(0)) {
                            player.triggerAttack();
                            if (audio) audio->playSound(SoundID::SwordSwing, 0.45f, 1.35f);
                        }
                        miningProgress = 0.0f;
                        currentMiningPos = {0, -999, 0};
                        currentMiningId = 0;
                    }
                } else {
                    // Not holding LMB
                    miningProgress = 0.0f;
                    currentMiningPos = {0, -999, 0};
                    currentMiningId = 0;
                }
            }

            // Right Click Action (Workstations, Consumables, Pal Spheres, Saplings & Bone Meal)
            if (window->isMouseButtonPressed(1)) {
                if (buildingMgr->getIsBuilding()) {
                    buildingMgr->rotatePreview();
                } else {
                    // Check if clicking functional workstation block in the world!
                    RaycastResult rHit = world->raycast(aimRay, 5.5f);
                    if (rHit.hit && rHit.blockId == 331) { // Crafting Table
                        inventoryMenu.openCraftingTable();
                        audio->playSound(SoundID::BlockPlace, 1.2f, 0.9f);
                    } else if (rHit.hit && rHit.blockId == 332) { // Furnace (Ocak)
                        inventoryMenu.openFurnace();
                        audio->playSound(SoundID::BlockPlace, 0.9f, 0.9f);
                    } else {
                        ItemStack& held = player.getInventory().getSlot(player.getSelectedHotbarIndex());
                        if (!held.isEmpty() && (held.id == 520 || held.id == 521 || held.id == 522)) {
                            // Consumable potion/food
                            std::string iName = ItemRegistry::get(held.id).name;
                            if (player.tryUseHeldItem(audio.get())) {
                                hud.addNotification("Used " + iName + "!", {0.4f, 0.95f, 0.5f, 1.0f});
                            }
                        } else if (!held.isEmpty() && (held.id == 523 || held.id == 526 || held.id == 527)) {
                            // Throw physical 3D Capture Sphere projectile!
                            Vec3 throwOrigin = camera.getRenderPosition() + camera.getForward() * 0.65f + Vec3(0, -0.1f, 0);
                            Vec3 throwVel = camera.getForward() * 19.5f + Vec3(0, 3.8f, 0);
                            captureSpheres.push_back(std::make_unique<CaptureSphere>(held.id, throwOrigin, throwVel));

                            uint16_t sphereId = held.id;
                            player.getInventory().removeItem(sphereId, 1);
                            if (audio) audio->playSound(SoundID::SwordSwing, 1.25f, 1.35f);
                            std::string sName = ItemRegistry::get(sphereId).name;
                            hud.addNotification("Threw " + sName + "!", {0.30f, 0.90f, 1.0f, 1.0f});
                        } else if (!held.isEmpty() && held.id == 525 && rHit.hit && World::isSapling(rHit.blockId)) {
                            // Bone Meal applied to planted sapling: instantly mature into procedural tree!
                            uint16_t saplingId = rHit.blockId;
                            world->growTree(rHit.hitBlockPos.x, rHit.hitBlockPos.y, rHit.hitBlockPos.z, saplingId);
                            world->removeSapling(rHit.hitBlockPos.x, rHit.hitBlockPos.y, rHit.hitBlockPos.z);
                            held.count--;
                            if (held.count == 0) held.clear();
                            audio->playSound(SoundID::LevelUp, 1.2f, 1.1f);
                            hud.addNotification("★ Tree matured instantly with Bone Meal! ★", {0.35f, 0.95f, 0.55f, 1.0f});
                        } else if (rHit.hit) {
                            // Place held block
                            if (!held.isEmpty() && held.id >= 1 && held.id <= 369) {
                                bool canPlace = true;
                                if (World::isSapling(held.id)) {
                                    // Saplings must be planted on dirt, grass, podzol, turf, or moss
                                    uint16_t ground = rHit.blockId;
                                    bool isSoil = (ground == 1 || ground == 2 || ground == 3 || ground == 4 ||
                                                   ground == 5 || ground == 7 || ground == 9 || ground == 25 || ground == 26);
                                    if (!isSoil || rHit.adjacentPos.y <= rHit.hitBlockPos.y) {
                                        canPlace = false;
                                        hud.addNotification("Saplings must be planted on dirt or grass!", {0.95f, 0.40f, 0.35f, 1.0f});
                                    }
                                }
                                if (canPlace) {
                                    world->setBlock(rHit.adjacentPos.x, rHit.adjacentPos.y, rHit.adjacentPos.z, held.id);
                                    audio->playSound(SoundID::BlockPlace, 1.0f, 1.0f);
                                    if (World::isSapling(held.id)) {
                                        hud.addNotification("Planted " + ItemRegistry::get(held.id).name + "! (Growing...)", {0.4f, 0.95f, 0.5f, 1.0f});
                                    }
                                    held.count--;
                                    if (held.count == 0) held.clear();
                                }
                            }
                        }
                    }
                }
            }

            // 'Q' Key: Drop held item into the world as a 3D ItemEntity
            if (window->isKeyPressed('Q')) {
                ItemStack& held = player.getInventory().getSlot(player.getSelectedHotbarIndex());
                if (!held.isEmpty()) {
                    Vec3 dropPos = player.getPosition() + Vec3(0, 1.2f, 0) + camera.getForward() * 0.7f;
                    Vec3 dropVel = camera.getForward() * 5.0f + Vec3(0, 2.2f, 0);
                    droppedItems.push_back(std::make_unique<ItemEntity>(ItemStack{held.id, 1, held.maxStack}, dropPos, dropVel));
                    held.count--;
                    if (held.count == 0) held.clear();
                    audio->playSound(SoundID::ItemPickup, 0.8f, 0.7f);
                }
            }

            // 'F' Key: Hammer blueprint scaffold
            if (window->isKeyDown('F')) {
                buildingMgr->hammerTargetBlueprint(aimRay, dt * 3.0f);
            }

            // 'R' Key: Rotate building piece when in building mode
            if (window->isKeyPressed('R') && buildingMgr->getIsBuilding()) {
                buildingMgr->rotatePreview();
            }

            // 'V' Key: Command Companion Stance (Follow -> Stay -> Work at Base)
            if (window->isKeyPressed('V')) {
                Creature* cmdTarget = nullptr;
                float cDist = 16.0f;
                for (auto& c : creatures) {
                    if (!c->getIsDead() && c->getIsTamed()) {
                        Vec3 toMob = c->getPosition() - camera.getRenderPosition();
                        float d = toMob.length();
                        if (d < cDist) {
                            float dot = toMob.normalized().dot(camera.getForward());
                            if (dot > 0.65f) {
                                cDist = d;
                                cmdTarget = c.get();
                            }
                        }
                    }
                }
                if (!cmdTarget) {
                    // Fallback to closest tamed companion
                    float closestD = 35.0f;
                    for (auto& c : creatures) {
                        if (!c->getIsDead() && c->getIsTamed()) {
                            float d = (c->getPosition() - player.getPosition()).length();
                            if (d < closestD) {
                                closestD = d;
                                cmdTarget = c.get();
                            }
                        }
                    }
                }

                if (cmdTarget) {
                    cmdTarget->cycleStance();
                    if (audio) audio->playSound(SoundID::ItemPickup, 1.30f, 1.25f);
                    hud.addNotification("★ " + cmdTarget->getDef().name + " set to [" + cmdTarget->getStanceName() + "] ★",
                                       {0.25f, 0.95f, 1.0f, 1.0f});
                } else {
                    hud.addNotification("No tamed companion nearby to command!", {0.95f, 0.45f, 0.35f, 1.0f});
                }
            }

            // Update ghost preview placement
            if (buildingMgr->getIsBuilding()) {
                buildingMgr->updateGhostPlacement(aimRay);
            }
        }

        // Entity & Systems Updates
        if (gameState == GameState::Playing) {
            camera.setPosition(player.getPosition());
            player.update(world.get(), audio.get(), dt);
            inventoryMenu.update(dt); // Smelts items in furnace (Ocak)

            // Spawn pending dropped items from Inventory Action Deck
            if (!inventoryMenu.getPendingDrops().empty()) {
                for (const auto& item : inventoryMenu.getPendingDrops()) {
                    Vec3 dropPos = player.getPosition() + Vec3(0, 1.2f, 0) + camera.getForward() * 0.7f;
                    Vec3 dropVel = camera.getForward() * 4.0f + Vec3(0, 2.0f, 0);
                    droppedItems.push_back(std::make_unique<ItemEntity>(item, dropPos, dropVel));
                }
                inventoryMenu.clearPendingDrops();
            }

            buildingMgr->update(dt);
        } else {
            camera.setPosition(player.getPosition());
        }

        world->update(player.getPosition(), dt);
        audio->update(dt);

        // Update physical dropped items (physics, terrain bounce, player magnetism, pickup)
        for (auto it = droppedItems.begin(); it != droppedItems.end(); ) {
            (*it)->update(world.get(), player.getPosition(), dt);
            if ((*it)->canPickup()) {
                float pDist = ((*it)->getPosition() - (player.getPosition() + Vec3(0, 0.5f, 0))).length();
                if (pDist < 1.35f) {
                    const ItemStack& is = (*it)->getItem();
                    uint32_t rem = player.getInventory().addItem(is.id, is.count);
                    if (rem < is.count) {
                        uint32_t picked = is.count - rem;
                        const auto& iDef = ItemRegistry::get(is.id);
                        hud.addNotification("+" + std::to_string(picked) + " " + iDef.name, iDef.color);
                        audio->playSound(SoundID::ItemPickup, 1.15f, 0.85f);
                        (*it)->markPickedUp();
                    }
                }
            }
            if ((*it)->isDead()) {
                it = droppedItems.erase(it);
            } else {
                ++it;
            }
        }

        // Dynamic mob spawner (culls far/dead mobs, populates biomes)
        mobSpawner.update(world.get(), player.getPosition(), creatures, dt);

        // Update creature AI, companion combat, and hostile attack damage
        for (auto& c : creatures) {
            c->updateAI(world.get(), player.getPosition(), buildingMgr.get(), audio.get(), dt);

            // Hostile creature melee attack against player
            if (gameState == GameState::Playing && !c->getIsDead() && c->getDef().isHostile && c->isAttacking()) {
                float dist = (c->getPosition() - player.getPosition()).length();
                if (dist <= 2.5f) {
                    player.takeDamage(c->getDef().attackDamage * dt * 0.75f, audio.get());

                    // Alert nearby tamed companions to defend master
                    for (auto& comp : creatures) {
                        if (!comp->getIsDead() && comp->getIsTamed() && comp->getStance() == CompanionStance::Follow) {
                            float cd = (comp->getPosition() - player.getPosition()).length();
                            if (cd <= 25.0f) {
                                comp->notifyMasterDamaged(c.get());
                            }
                        }
                    }
                }
            }

            // Tamed companion melee attack against hostile combat target
            if (gameState == GameState::Playing && !c->getIsDead() && c->getIsTamed() && c->isCompanionAttacking() && c->getCompanionTarget()) {
                Creature* tgt = c->getCompanionTarget();
                if (!tgt->getIsDead()) {
                    float distToTgt = (c->getPosition() - tgt->getPosition()).length();
                    if (distToTgt <= 2.6f) {
                        Vec3 kb = (tgt->getPosition() - c->getPosition()).normalized() * 0.8f;
                        tgt->takeDamage(c->getCompanionAttackDamage() * dt * 0.85f, kb, audio.get());
                        if (tgt->getIsDead()) {
                            c->gainCompanionXP(tgt->getDef().xpReward * 1.2f, audio.get());
                            c->setCompanionTarget(nullptr);
                        }
                    }
                }
            }
        }

        // Update physical capture spheres (ballistics, wobble stage checks, missed retrieval)
        for (auto it = captureSpheres.begin(); it != captureSpheres.end();) {
            (*it)->update(world.get(), creatures, audio.get(), dt, [&](const std::string& msg, const Vec4& col) {
                hud.addNotification(msg, col);
            });

            if ((*it)->canPickup()) {
                float pDist = ((*it)->getPosition() - (player.getPosition() + Vec3(0, 0.5f, 0))).length();
                if (pDist < 1.4f) {
                    uint16_t sId = (*it)->getSphereItemId();
                    uint32_t rem = player.getInventory().addItem(sId, 1);
                    if (rem == 0) {
                        hud.addNotification("+1 " + ItemRegistry::get(sId).name + " (Retrieved)", {0.3f, 0.9f, 1.0f, 1.0f});
                        if (audio) audio->playSound(SoundID::ItemPickup, 1.15f, 0.9f);
                        (*it)->markPickedUp();
                    }
                }
            }

            if ((*it)->isDead()) {
                it = captureSpheres.erase(it);
            } else {
                ++it;
            }
        }

        // ----------------------------------------------------
        // OPENGL FRAME RENDERING
        // ----------------------------------------------------
        uint8_t curBiomeId = world->getBiomeAt(player.getPosition().x, player.getPosition().z);
        const BiomeDef& curBiome = BiomeRegistry::get(curBiomeId);

        // Determine target sky, fog color, and fog distance based on underwater, underlava, or biome + day/night state
        Vec3 dynamicSky = world->getSkyColor(Vec3(curBiome.skyColor.x, curBiome.skyColor.y, curBiome.skyColor.z));
        Vec3 dynamicFog = world->getFogColor(Vec3(curBiome.fogColor.x, curBiome.fogColor.y, curBiome.fogColor.z));
        Vec4 targetSkyColor(dynamicSky.x, dynamicSky.y, dynamicSky.z, 1.0f);
        Vec4 targetFogColor(dynamicFog.x, dynamicFog.y, dynamicFog.z, 1.0f);

        float defaultMaxFogDist = static_cast<float>(config.graphics.renderDistance * CHUNK_X) * 0.95f;
        float defaultFogEnd = std::max(60.0f, defaultMaxFogDist);
        float targetFogEnd = defaultFogEnd;
        float targetFogStart = defaultFogEnd * 0.45f;

        if (player.getIsUnderLava()) {
            targetSkyColor = Vec4(0.85f, 0.20f, 0.05f, 1.0f);
            targetFogColor = Vec4(0.85f, 0.20f, 0.05f, 1.0f);
            targetFogStart = 0.5f;
            targetFogEnd = 8.0f;
        } else if (player.getIsUnderwater()) {
            targetSkyColor = Vec4(0.06f, 0.28f, 0.58f, 1.0f);
            targetFogColor = Vec4(0.06f, 0.28f, 0.58f, 1.0f);
            targetFogStart = 1.0f;
            targetFogEnd = 28.0f;
        }

        // Atmospheric smoothing across biomes, day/night cycles, and fluids
        static Vec4 smoothSkyColor(0.45f, 0.65f, 0.95f, 1.0f);
        static Vec4 smoothFogColor(0.70f, 0.82f, 0.95f, 1.0f);
        static float smoothFogStart = 30.0f;
        static float smoothFogEnd = 85.0f;
        static bool atmosphereInit = false;

        if (!atmosphereInit) {
            smoothSkyColor = targetSkyColor;
            smoothFogColor = targetFogColor;
            smoothFogStart = targetFogStart;
            smoothFogEnd = targetFogEnd;
            atmosphereInit = true;
        } else {
            // Responsive smooth transition when entering/leaving fluid or cycle changes
            float blendSpeed = std::clamp(dt * (player.getIsUnderwater() || player.getIsUnderLava() ? 6.0f : 2.5f), 0.0f, 1.0f);
            smoothSkyColor.x += (targetSkyColor.x - smoothSkyColor.x) * blendSpeed;
            smoothSkyColor.y += (targetSkyColor.y - smoothSkyColor.y) * blendSpeed;
            smoothSkyColor.z += (targetSkyColor.z - smoothSkyColor.z) * blendSpeed;

            smoothFogColor.x += (targetFogColor.x - smoothFogColor.x) * blendSpeed;
            smoothFogColor.y += (targetFogColor.y - smoothFogColor.y) * blendSpeed;
            smoothFogColor.z += (targetFogColor.z - smoothFogColor.z) * blendSpeed;

            smoothFogStart += (targetFogStart - smoothFogStart) * blendSpeed;
            smoothFogEnd += (targetFogEnd - smoothFogEnd) * blendSpeed;
        }

        float fogStart = smoothFogStart;
        float fogEnd = smoothFogEnd;


        // Pipe 3D scene into Iris G-Buffers (or screen FBO 0 if Vanilla)
        irisManager->resize(window->getWidth(), window->getHeight());
        irisManager->beginScene();

        // Clear screen with matching horizon fog color for 100% seamless transition
        glClearColor(smoothFogColor.x, smoothFogColor.y, smoothFogColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate Camera Matrices (OpenGL Projection)
        Mat4 viewMat = camera.getViewMatrix();
        float fovRad = camera.getCurrentFOV() * DEG2RAD;
        Mat4 projMat = Mat4::perspectiveGL(fovRad, window->getAspectRatio(), 0.1f, 1000.0f);
        Mat4 viewProj = projMat * viewMat;
        Mat4 modelIdentity = Mat4::identity();

        Vec3 camEye = camera.getRenderPosition();
        Vec3 sunDir = world->getSunDirection();

        // 1. VOXEL PIPELINE (Terrain, Creatures, Placed Structures)
        glPipeline->useVoxel();
        glPipeline->setVoxelUniforms(viewProj, modelIdentity, sunDir, camEye,
                                     config.graphics.ambientOcclusion,
                                     smoothFogColor, fogStart, fogEnd,
                                     config.graphics.fog,
                                     totalTime, irisManager->getWavingFoliage());
        textureAtlas->bind(0);

        // Render World Chunks
        world->render();

        // Batch & Render Physical Dropped 3D Items
        std::vector<VoxelVertex> itemVerts;
        for (const auto& itm : droppedItems) {
            itm->appendVertices(itemVerts, totalTime);
        }
        if (!itemVerts.empty()) {
            itemBuffer.uploadVoxelData(itemVerts.data(), itemVerts.size() * sizeof(VoxelVertex),
                                       static_cast<uint32_t>(itemVerts.size()), true);
            itemBuffer.draw();
        }

        // Batch & Render Creatures and Physical Capture Spheres
        std::vector<VoxelVertex> creatureVerts;
        for (const auto& c : creatures) {
            c->appendModelVertices(creatureVerts, totalTime);
        }
        for (const auto& sp : captureSpheres) {
            sp->appendModelVertices(creatureVerts, totalTime);
        }
        if (!creatureVerts.empty()) {
            creatureBuffer.uploadVoxelData(creatureVerts.data(), creatureVerts.size() * sizeof(VoxelVertex),
                                           static_cast<uint32_t>(creatureVerts.size()), true);
            creatureBuffer.draw();
        }

        // Render Placed Structures & Hologram Ghost Preview
        buildingMgr->render(glPipeline.get(), viewProj, camEye, totalTime);

        // 2. IRIS POST-PROCESSING PIPELINE (Composite God Rays & Final ACES Tone Mapping)
        irisManager->endScene();
        irisManager->renderCompositeAndFinal(sunDir, viewProj, totalTime);

        // 3. UI PIPELINE (HUD, Menus & Settings)
        uiRenderer->begin(window->getWidth(), window->getHeight());

        if (gameState == GameState::MainMenu) {
            mainMenu.render(uiRenderer.get(), window->getWidth(), window->getHeight(), mx, my, mClicked, totalTime);
        } else {
            // Render In-Game HUD & Mob Overhead Indicators only when gameplay is active (no menu open)
            if (!anyMenuOpen) {
                // Detect aimed wild creature for capture reticle and find active tamed companion
                const ItemStack& curHeld = player.getInventory().getSlot(player.getSelectedHotbarIndex());
                bool isHoldingSphere = (!curHeld.isEmpty() && (curHeld.id == 523 || curHeld.id == 526 || curHeld.id == 527));

                Creature* aimedWildCreature = nullptr;
                float aimCatchChance = 0.0f;
                if (isHoldingSphere) {
                    float closeTameDist = 22.0f;
                    for (auto& c : creatures) {
                        if (!c->getIsDead() && !c->getIsTamed() && !c->isInCapture()) {
                            Vec3 toMob = c->getPosition() + Vec3(0, c->getDef().size.y * 0.5f, 0) - camera.getRenderPosition();
                            float d = toMob.length();
                            if (d < closeTameDist) {
                                float dot = toMob.normalized().dot(camera.getForward());
                                if (dot > 0.70f) {
                                    closeTameDist = d;
                                    aimedWildCreature = c.get();
                                }
                            }
                        }
                    }
                    if (aimedWildCreature) {
                        aimCatchChance = CaptureSphere::calculateCaptureChance(curHeld.id, *aimedWildCreature);
                    }
                }

                // Active Companion detection (closest tamed creature within 35 blocks)
                Creature* activeCompanion = nullptr;
                float closestCompDist = 35.0f;
                for (auto& c : creatures) {
                    if (!c->getIsDead() && c->getIsTamed()) {
                        float d = (c->getPosition() - player.getPosition()).length();
                        if (d < closestCompDist) {
                            closestCompDist = d;
                            activeCompanion = c.get();
                        }
                    }
                }

                hud.render(window->getWidth(), window->getHeight(), player, *world, *buildingMgr, timer.getFPS(), totalTime, dt, camera.getIsZooming(), miningProgress, currentMiningId, aimedWildCreature, aimCatchChance, activeCompanion);

                // Render Floating Overhead Health Bars & Nameplates for Visible Nearby Creatures
                Creature* closestBoss = nullptr;
                float closestBossDist = 999.0f;

                for (const auto& c : creatures) {
                    if (c->getIsDead()) continue;

                    float dist = (c->getPosition() - player.getPosition()).length();

                    // Track nearby World Boss
                    if (c->getDef().isBoss && dist < 50.0f) {
                        if (dist < closestBossDist) {
                            closestBossDist = dist;
                            closestBoss = c.get();
                        }
                    }

                    // Only draw floating overhead bar if within 26 blocks
                    if (dist > 26.0f) continue;

                    Vec3 overheadWorld = c->getPosition() + Vec3(0.0f, c->getDef().size.y + 0.45f, 0.0f);
                    Vec4 clip = viewProj * Vec4(overheadWorld, 1.0f);
                    if (clip.w > 0.15f) {
                        float ndcX = clip.x / clip.w;
                        float ndcY = clip.y / clip.w;
                        float ndcZ = clip.z / clip.w;

                        if (ndcZ >= -1.0f && ndcZ <= 1.0f && ndcX >= -1.1f && ndcX <= 1.1f && ndcY >= -1.1f && ndcY <= 1.1f) {
                            float sx = (ndcX * 0.5f + 0.5f) * static_cast<float>(window->getWidth());
                            float sy = (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(window->getHeight());

                            // Bar dimensions scaled smoothly by distance
                            float scaleFactor = std::clamp(14.0f / (dist + 4.0f), 0.65f, 1.25f);
                            float barW = 56.0f * scaleFactor;
                            float barH = 5.0f * scaleFactor;
                            float hpFrac = c->getHealth() / c->getMaxHealth();

                            Vec4 fillColor = c->getDef().isHostile ? Vec4(0.9f, 0.22f, 0.22f, 0.92f) :
                                             (c->getIsTamed() ? Vec4(0.15f, 0.80f, 1.0f, 0.95f) : Vec4(0.25f, 0.88f, 0.32f, 0.92f));

                            uiRenderer->drawProgressBar(sx - barW * 0.5f, sy, barW, barH, hpFrac, fillColor, Vec4(0.1f, 0.1f, 0.1f, 0.75f));

                            int mobLevel = c->getIsTamed() ? c->getCompanionLevel() : std::max(1, static_cast<int>(c->getMaxHealth() / 15.0f));
                            std::string label = c->getIsTamed() ?
                                                ("★ " + c->getDef().name + " (Lv." + std::to_string(mobLevel) + ") [" + c->getStanceName() + "] ★") :
                                                (c->getDef().name + " [Lv." + std::to_string(mobLevel) + "]");
                            float textScale = (dist < 10.0f) ? 1.2f : 0.9f;
                            Vec4 nameCol = c->getIsTamed() ? Vec4(0.35f, 0.95f, 1.0f, 1.0f) : Vec4(1.0f, 1.0f, 1.0f, 0.95f);
                            uiRenderer->drawTextCentered(label, sx, sy - 8.0f * textScale - 2.0f, textScale, nameCol);
                        }
                    }
                }

                // Render Grand Boss Health Bar at top of screen if Boss is nearby
                if (closestBoss) {
                    float bW = 420.0f;
                    float bH = 16.0f;
                    float bX = (static_cast<float>(window->getWidth()) - bW) * 0.5f;
                    float bY = 40.0f;
                    float bossHpFrac = closestBoss->getHealth() / closestBoss->getMaxHealth();

                    uiRenderer->drawRect(bX - 4.0f, bY - 22.0f, bW + 8.0f, bH + 28.0f, Vec4(0.05f, 0.05f, 0.08f, 0.85f));
                    uiRenderer->drawRectOutline(bX - 4.0f, bY - 22.0f, bW + 8.0f, bH + 28.0f, 1.5f, Vec4(0.85f, 0.65f, 0.15f, 0.95f));

                    int bossLevel = std::max(1, static_cast<int>(closestBoss->getMaxHealth() / 15.0f));
                    std::string bossTitle = "[WORLD BOSS] " + closestBoss->getDef().name + " - Lv." + std::to_string(bossLevel);
                    uiRenderer->drawTextCentered(bossTitle, bX + bW * 0.5f, bY - 18.0f, 1.6f, Vec4(1.0f, 0.82f, 0.25f, 1.0f));

                    uiRenderer->drawProgressBar(bX, bY + 4.0f, bW, bH, bossHpFrac, Vec4(0.95f, 0.15f, 0.15f, 0.95f), Vec4(0.2f, 0.05f, 0.05f, 0.85f));

                    std::string hpStr = std::to_string(static_cast<int>(closestBoss->getHealth())) + " / " + std::to_string(static_cast<int>(closestBoss->getMaxHealth()));
                    uiRenderer->drawTextCentered(hpStr, bX + bW * 0.5f, bY + 6.0f, 1.2f, Vec4(1.0f, 1.0f, 1.0f, 0.95f));
                }
            }

            // Render active menus
            if (inventoryMenu.getIsOpen()) {
                inventoryMenu.render(uiRenderer.get(), window->getWidth(), window->getHeight(), player, audio.get(), mx, my, mDown, mClicked, rClicked, shiftDown);
            }
            if (buildMenu.getIsOpen()) buildMenu.render(window->getWidth(), window->getHeight(), *buildingMgr);
            if (blockCatalog.getIsOpen()) blockCatalog.render(window->getWidth(), window->getHeight(), player);
            if (bestiary.getIsOpen()) bestiary.render(window->getWidth(), window->getHeight());
            if (biomeCodex.getIsOpen()) biomeCodex.render(window->getWidth(), window->getHeight());
        }

        // Render Settings Menu
        if (settingsMenu.isOpen()) {
            settingsMenu.render(uiRenderer.get(), window->getWidth(), window->getHeight(), mx, my, mDown, mClicked);
        }

        // Render Iris Shaderpacks Menu on topmost layer
        if (irisShaderUI.isOpen()) {
            irisShaderUI.render(uiRenderer.get(), window->getWidth(), window->getHeight(), mx, my, mDown, mClicked);
        }

        // Render Update Calendar & Development Roadmap Menu
        if (updateCalendar.getIsOpen()) {
            updateCalendar.render(window->getWidth(), window->getHeight(), mx, my, mDown, mClicked, totalTime);
        }

        uiRenderer->end(glPipeline.get());

        // Present OpenGL frame
        glContext->swapBuffers();
    }

    std::cout << "Chronicles of Aetheria exited cleanly. Goodbye!" << std::endl;
    return 0;
}
