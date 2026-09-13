#include "UpdateCalendarUI.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace Aetheria {

UpdateCalendarUI::UpdateCalendarUI(UIRenderer* ui) : ui(ui) {
    initEvents();
}

void UpdateCalendarUI::initEvents() {
    events.clear();

    // 0. v1.0
    {
        CalendarEvent ev;
        ev.version = "v1.0";
        ev.title = "Genesis of Aetheria";
        ev.targetDate = "August 15, 2026";
        ev.status = "[RELEASED]";
        ev.statusColor = {0.35f, 0.78f, 0.45f, 1.0f};
        ev.description = "Initial foundational release of Chronicles of Aetheria with 60 FPS modern voxel meshing and procedural generation.";
        ev.highlights = {
            "60 FPS dynamic voxel chunk greedy-meshing engine",
            "Real-time procedural solar illumination & day/night cycle",
            "First-person character physics & smooth voxel collision",
            "Procedural cave networks with deep mineral ore distribution"
        };
        ev.techDetails = "Modern OpenGL 3.3 Core Profile, Win32 High-Resolution Performance Timers, FastNoise Lite SIMD terrain.";
        ev.progressPercent = 100;
        ev.category = UpdateCategory::MajorRelease;
        ev.year = 2026;
        ev.month = 8;
        ev.day = 15;
        events.push_back(ev);
    }

    // 1. v2.0
    {
        CalendarEvent ev;
        ev.version = "v2.0";
        ev.title = "The Grand Overhaul & 365 Blocks";
        ev.targetDate = "September 1, 2026";
        ev.status = "[RELEASED]";
        ev.statusColor = {0.35f, 0.78f, 0.45f, 1.0f};
        ev.description = "Colossal content expansion introducing 365 unique block textures, 35 biomes, 20 mobs & bosses, and Palworld building.";
        ev.highlights = {
            "365 distinct block textures with crisp RPG voxel styling",
            "35 procedural climate biomes with custom atmospheric fog",
            "Palworld-style building system with 16 structural blueprints",
            "20 diverse creatures, tameable companions & Grand Boss encounters"
        };
        ev.techDetails = "Unified BlockRegistry & BiomeRegistry, Modular StructurePiece blueprints, AABB entity physics & dynamic MobSpawner.";
        ev.progressPercent = 100;
        ev.category = UpdateCategory::MajorRelease;
        ev.year = 2026;
        ev.month = 9;
        ev.day = 1;
        events.push_back(ev);
    }

    // 2. v2.4 (CURRENT RELEASE)
    {
        CalendarEvent ev;
        ev.version = "v2.4";
        ev.title = "The Iris Horizon & Motion Blur";
        ev.targetDate = "September 13, 2026";
        ev.status = "[LIVE NOW]";
        ev.statusColor = {0.0f, 0.95f, 0.65f, 1.0f};
        ev.description = "The current live release! Iris Minecraft shaderpack support, cinematic motion blur, double-tap sprint, dash, and sneak edge protection.";
        ev.highlights = {
            "Iris Shader Architecture: OptiFine pipeline with HDR G-Buffer FBOs",
            "Cinematic Motion Blur: 8-tap velocity reconstruction via inverse VP matrix",
            "Enhanced Movement: Double-tap 'W' sprint, 'R' impulse dash & 'C' zoom",
            "Minecraft Sneak Edge Protection: Prevents falling off heights while crouching",
            "0.5-Block Step Climbing: Seamless traversal over slabs, stairs, and micro-terrain"
        };
        ev.techDetails = "Multipass HDR framebuffers (colortex0, colortex1, depthtex0), ACES Filmic Tone Mapping, 4x4 Matrix Inversion.";
        ev.progressPercent = 100;
        ev.category = UpdateCategory::MajorRelease;
        ev.year = 2026;
        ev.month = 9;
        ev.day = 13;
        events.push_back(ev);
    }

    // 3. Harvest Festival
    {
        CalendarEvent ev;
        ev.version = "EVENT";
        ev.title = "Harvest Equinox Festival";
        ev.targetDate = "September 22 - 25, 2026";
        ev.status = "[UPCOMING EVENT]";
        ev.statusColor = {1.0f, 0.82f, 0.25f, 1.0f};
        ev.description = "Seasonal celebration honoring the turning of the seasons, golden wheat yields, and wandering merchants.";
        ev.highlights = {
            "Golden crops yield 2x harvest rates across all farm plots",
            "Vibrant autumn foliage tinting in deciduous forest biomes",
            "Wandering merchant visits surface settlements with rare saplings",
            "Exclusive crafting recipes for Festive Autumn Lanterns"
        };
        ev.techDetails = "Seasonal flora color blending shaders, dynamic trading merchant state machine, world calendar event hooks.";
        ev.progressPercent = 90;
        ev.category = UpdateCategory::SeasonalEvent;
        ev.year = 2026;
        ev.month = 9;
        ev.day = 22;
        events.push_back(ev);
    }

    // 4. v2.5
    {
        CalendarEvent ev;
        ev.version = "v2.5";
        ev.title = "The Abyssal Underworld";
        ev.targetDate = "October 16, 2026";
        ev.status = "[NEXT MAJOR UPDATE]";
        ev.statusColor = {0.0f, 0.85f, 1.0f, 1.0f};
        ev.description = "Upcoming dimension expansion unveiling the burning magma depths, Netherite tier gear, and obsidian gate portals.";
        ev.highlights = {
            "Obsidian portal ignition and inter-dimensional chunk streaming",
            "Nether realm featuring Basalt Deltas, Soul Sand Valleys & Crimson Forests",
            "Netherite alloy forging table for Tier 5 weapons and armor",
            "Magma Colossus world boss with volcanic shockwave attacks"
        };
        ev.techDetails = "Multi-dimension world chunk caching, volumetric lava fog, heated particle emitters, dimensional transition shaders.";
        ev.progressPercent = 75;
        ev.category = UpdateCategory::Roadmap;
        ev.year = 2026;
        ev.month = 10;
        ev.day = 16;
        events.push_back(ev);
    }

    // 5. Blood Moon
    {
        CalendarEvent ev;
        ev.version = "EVENT";
        ev.title = "Blood Moon & Eclipse Invasions";
        ev.targetDate = "October 31, 2026";
        ev.status = "[HALLOWEEN EVENT]";
        ev.statusColor = {0.95f, 0.25f, 0.35f, 1.0f};
        ev.description = "Special night-time world event where the celestial moon turns blood red and dangerous nocturnal swarms awaken.";
        ev.highlights = {
            "Crimson skybox illumination with eerie atmospheric synth drones",
            "Triple monster spawn density with rare Elite shadow variants",
            "Guaranteed world boss drop multipliers and Cursed Relic treasures",
            "Carved Jack-o'-Lantern blocks and Witch's Cauldron alchemy"
        };
        ev.techDetails = "Custom skybox atmosphere uniforms, entity buff system, seasonal loot injection tables.";
        ev.progressPercent = 60;
        ev.category = UpdateCategory::SeasonalEvent;
        ev.year = 2026;
        ev.month = 10;
        ev.day = 31;
        events.push_back(ev);
    }

    // 6. v2.6
    {
        CalendarEvent ev;
        ev.version = "v2.6";
        ev.title = "Companions & Flying Mounts";
        ev.targetDate = "November 20, 2026";
        ev.status = "[IN DEVELOPMENT]";
        ev.statusColor = {0.75f, 0.45f, 1.0f, 1.0f};
        ev.description = "Rideable aerial mounts, creature breeding, command wheel orders, and forged pet armor.";
        ev.highlights = {
            "Saddle crafting and dynamic aerial flight across high mountain ranges",
            "Griffin, Pegasus, and Drake tameable flying mounts",
            "Whistle command wheel (Follow, Guard, Attack, Patrol)",
            "Pet armor forgeable with Iron, Gold, and Diamond plating"
        };
        ev.techDetails = "Flight aerodynamics physics, multi-entity collision parenting, companion AI behavior tree expansion.";
        ev.progressPercent = 40;
        ev.category = UpdateCategory::Roadmap;
        ev.year = 2026;
        ev.month = 11;
        ev.day = 20;
        events.push_back(ev);
    }

    // 7. Winter Solstice
    {
        CalendarEvent ev;
        ev.version = "EVENT";
        ev.title = "Winter Solstice & Frost Wyrm Raid";
        ev.targetDate = "December 21, 2026";
        ev.status = "[WINTER FESTIVAL]";
        ev.statusColor = {0.35f, 0.65f, 1.0f, 1.0f};
        ev.description = "Annual winter festival bringing blizzards, ice sailing gliders, and the Ancient Frost Wyrm apex raid boss.";
        ev.highlights = {
            "Dynamic snow accumulation on exposed blocks during blizzards",
            "Frost Wyrm apex raid boss encountered in glacial mountain peaks",
            "Ice skates and aerodynamic gliding sleds for frozen lake racing",
            "Holiday gift tree boxes containing festive cosmetic items"
        };
        ev.techDetails = "Dynamic voxel snow layering, blizzard particle wind vectors, Frost Wyrm multi-segment skeletal animation.";
        ev.progressPercent = 25;
        ev.category = UpdateCategory::SeasonalEvent;
        ev.year = 2026;
        ev.month = 12;
        ev.day = 21;
        events.push_back(ev);
    }

    // 8. v3.0
    {
        CalendarEvent ev;
        ev.version = "v3.0";
        ev.title = "Arcane Sorcery & Runecrafting";
        ev.targetDate = "Q1 2027";
        ev.status = "[2027 ROADMAP]";
        ev.statusColor = {0.85f, 0.40f, 0.95f, 1.0f};
        ev.description = "Complete spellcasting framework introducing mana pools, spell staves, rune inscribing, and elemental dungeons.";
        ev.highlights = {
            "Mana regeneration & spellcasting (Fireball, Frost Nova, Blink, Heal)",
            "Spellbook crafting and elemental rune gem socketing",
            "Ancient Runestone obelisks hidden throughout all 35 biomes",
            "Towering Wizard spires and floating mystical sky isles"
        };
        ev.techDetails = "Arcane projectile physics engine, dynamic volumetric lighting for spells, rune enchantment pipeline.";
        ev.progressPercent = 15;
        ev.category = UpdateCategory::Roadmap;
        ev.year = 2027;
        ev.month = 2;
        ev.day = 15;
        events.push_back(ev);
    }

    // 9. v3.5
    {
        CalendarEvent ev;
        ev.version = "v3.5";
        ev.title = "Multiplayer Co-op & Dedicated Realms";
        ev.targetDate = "Q2 2027";
        ev.status = "[FUTURE VISION]";
        ev.statusColor = {0.25f, 0.65f, 1.0f, 1.0f};
        ev.description = "Peer-to-peer multiplayer co-op, shared world building, and dedicated community servers.";
        ev.highlights = {
            "Seamless LAN and direct-IP 4-player co-op world hosting",
            "Shared building permissions and chest lock privacy protection",
            "Party leveling system with dynamic boss health scaling",
            "Dedicated headless server console binary for Windows and Linux"
        };
        ev.techDetails = "UDP state synchronization, client-side movement prediction, delta voxel compression.";
        ev.progressPercent = 5;
        ev.category = UpdateCategory::Roadmap;
        ev.year = 2027;
        ev.month = 5;
        ev.day = 1;
        events.push_back(ev);
    }
}

std::vector<int> UpdateCalendarUI::getFilteredIndices() const {
    std::vector<int> res;
    for (size_t i = 0; i < events.size(); ++i) {
        if (currentCategory == UpdateCategory::All || events[i].category == currentCategory) {
            res.push_back(static_cast<int>(i));
        }
    }
    return res;
}

void UpdateCalendarUI::nextMonth() {
    calendarMonth++;
    if (calendarMonth > 12) {
        calendarMonth = 1;
        calendarYear++;
    }
}

void UpdateCalendarUI::prevMonth() {
    calendarMonth--;
    if (calendarMonth < 1) {
        calendarMonth = 12;
        calendarYear--;
    }
}

bool UpdateCalendarUI::drawButton(float x, float y, float w, float h,
                                  const std::string& text, bool active,
                                  int mouseX, int mouseY, bool clicked,
                                  const Vec4& accent) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = active ? Vec4(accent.x * 0.35f, accent.y * 0.35f, accent.z * 0.35f, 0.92f)
                     : (hovered ? Vec4(0.18f, 0.23f, 0.32f, 0.90f) : Vec4(0.09f, 0.12f, 0.17f, 0.85f));

    Vec4 border = active ? accent
                         : (hovered ? Vec4(0.6f, 0.75f, 0.95f, 0.9f) : Vec4(0.20f, 0.28f, 0.38f, 0.65f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, active ? 1.8f : 1.2f, border);

    if (active) {
        ui->drawRect(x, y, w, 2.5f, accent);
    }

    float textScale = 1.35f;
    float tx = x + (w - text.length() * 6.0f * textScale) * 0.5f;
    float ty = y + (h - 7.0f * textScale) * 0.5f;

    Vec4 textColor = active ? Vec4(1.0f, 1.0f, 1.0f, 1.0f)
                            : (hovered ? Vec4(0.95f, 0.98f, 1.0f, 1.0f) : Vec4(0.70f, 0.78f, 0.88f, 0.9f));

    ui->drawText(text, tx, ty, textScale, textColor);

    return hovered && clicked;
}

void UpdateCalendarUI::render(int screenWidth, int screenHeight,
                             int mouseX, int mouseY, bool mouseLeftDown, bool mouseLeftClicked,
                             float totalTime) {
    if (!isOpen) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);

    // 1. Semi-transparent backdrop overlay
    ui->drawRect(0, 0, sw, sh, {0.02f, 0.03f, 0.06f, 0.82f});

    // 2. Main Dialog Frame
    float panelW = 940.0f;
    float panelH = 620.0f;
    float panelX = (sw - panelW) * 0.5f;
    float panelY = (sh - panelH) * 0.5f;

    // Panel background & double border
    ui->drawRect(panelX, panelY, panelW, panelH, {0.08f, 0.10f, 0.14f, 0.98f});
    ui->drawRectOutline(panelX, panelY, panelW, panelH, 2.0f, {0.20f, 0.65f, 0.95f, 0.85f});
    ui->drawRectOutline(panelX + 4, panelY + 4, panelW - 8, panelH - 8, 1.0f, {0.15f, 0.28f, 0.42f, 0.45f});

    // Header Glow Bar
    ui->drawRect(panelX + 2, panelY + 2, panelW - 4, 4.0f, {0.25f, 0.75f, 1.0f, 0.9f});

    // Title Header
    std::string title = "CHRONICLES OF AETHERIA // UPDATE CALENDAR & ROADMAP";
    float titleX = panelX + 26.0f;
    ui->drawText(title, titleX, panelY + 16.0f, 1.9f, {1.0f, 0.85f, 0.25f, 1.0f});

    std::string subTitle = "Release History, Development Milestones & Seasonal World Event Schedule";
    ui->drawText(subTitle, titleX, panelY + 38.0f, 1.25f, {0.60f, 0.72f, 0.85f, 0.95f});

    // =========================================================================
    // VIEW MODE SWITCHER TABS (Top-Right)
    // =========================================================================
    float modeW = 160.0f;
    float modeH = 28.0f;
    float modeX1 = panelX + panelW - 355.0f;
    float modeX2 = modeX1 + modeW + 8.0f;
    float modeY = panelY + 20.0f;

    if (drawButton(modeX1, modeY, modeW, modeH, "1. ROADMAP TIMELINE", viewMode == 0, mouseX, mouseY, mouseLeftClicked, {0.2f, 0.65f, 1.0f, 1.0f})) {
        viewMode = 0;
    }
    if (drawButton(modeX2, modeY, modeW, modeH, "2. MONTHLY CALENDAR", viewMode == 1, mouseX, mouseY, mouseLeftClicked, {1.0f, 0.75f, 0.2f, 1.0f})) {
        viewMode = 1;
    }

    // Divider
    ui->drawRect(panelX + 20.0f, panelY + 58.0f, panelW - 40.0f, 1.5f, {0.20f, 0.30f, 0.45f, 0.7f});

    // =========================================================================
    // CATEGORY FILTER BAR
    // =========================================================================
    float filterY = panelY + 68.0f;
    float fBtnW = 125.0f;
    float fBtnH = 24.0f;
    float fGap = 8.0f;
    float fStartX = panelX + 25.0f;

    ui->drawText("Filter:", fStartX, filterY + 5.0f, 1.35f, {0.7f, 0.75f, 0.85f, 1.0f});
    fStartX += 60.0f;

    if (drawButton(fStartX + 0 * (fBtnW + fGap), filterY, fBtnW, fBtnH, "ALL ENTRIES", currentCategory == UpdateCategory::All, mouseX, mouseY, mouseLeftClicked)) {
        currentCategory = UpdateCategory::All;
    }
    if (drawButton(fStartX + 1 * (fBtnW + fGap), filterY, fBtnW, fBtnH, "MAJOR RELEASES", currentCategory == UpdateCategory::MajorRelease, mouseX, mouseY, mouseLeftClicked)) {
        currentCategory = UpdateCategory::MajorRelease;
    }
    if (drawButton(fStartX + 2 * (fBtnW + fGap), filterY, fBtnW, fBtnH, "DEV ROADMAP", currentCategory == UpdateCategory::Roadmap, mouseX, mouseY, mouseLeftClicked)) {
        currentCategory = UpdateCategory::Roadmap;
    }
    if (drawButton(fStartX + 3 * (fBtnW + fGap), filterY, fBtnW, fBtnH, "WORLD EVENTS", currentCategory == UpdateCategory::SeasonalEvent, mouseX, mouseY, mouseLeftClicked)) {
        currentCategory = UpdateCategory::SeasonalEvent;
    }

    // =========================================================================
    // CONTENT AREA: MODE 0 (TIMELINE) OR MODE 1 (CALENDAR GRID)
    // =========================================================================
    float contentY = filterY + fBtnH + 12.0f;
    float contentH = panelH - (contentY - panelY) - 52.0f;

    if (viewMode == 0) {
        renderRoadmapTimeline(panelX, contentY, panelW, contentH, mouseX, mouseY, mouseLeftClicked, totalTime);
    } else {
        renderCalendarGrid(panelX, contentY, panelW, contentH, mouseX, mouseY, mouseLeftClicked, totalTime);
    }

    // =========================================================================
    // BOTTOM BAR: KEY HINTS & CLOSE BUTTON
    // =========================================================================
    float bottomY = panelY + panelH - 44.0f;
    ui->drawRect(panelX + 20.0f, bottomY - 6.0f, panelW - 40.0f, 1.2f, {0.18f, 0.25f, 0.35f, 0.6f});

    std::string navHint = "[1/2] View Mode  |  [K] Quick Toggle  |  [ESC] Close";
    ui->drawText(navHint, panelX + 30.0f, bottomY + 6.0f, 1.30f, {0.55f, 0.68f, 0.82f, 0.85f});

    float closeW = 200.0f;
    float closeH = 30.0f;
    float closeX = panelX + panelW - closeW - 25.0f;

    if (drawButton(closeX, bottomY - 2.0f, closeW, closeH, "CLOSE (Esc / K)", false, mouseX, mouseY, mouseLeftClicked, {0.9f, 0.3f, 0.3f, 1.0f})) {
        close();
    }
}

void UpdateCalendarUI::renderRoadmapTimeline(float panelX, float panelY, float panelW, float panelH,
                                            int mouseX, int mouseY, bool clicked, float totalTime) {
    std::vector<int> filtered = getFilteredIndices();
    if (filtered.empty()) return;

    // Left Column: Scrollable Cards Stream
    float col1X = panelX + 25.0f;
    float col1W = 380.0f;

    float cardH = 58.0f;
    float cardGap = 8.0f;

    int visibleCount = std::min(static_cast<int>(filtered.size()), 6);
    int startIdx = 0;
    // Keep selected item within view
    auto itSel = std::find(filtered.begin(), filtered.end(), selectedEventIdx);
    if (itSel != filtered.end()) {
        int pos = static_cast<int>(std::distance(filtered.begin(), itSel));
        if (pos >= visibleCount) {
            startIdx = pos - visibleCount + 1;
        }
    }

    for (int i = 0; i < visibleCount && (startIdx + i) < static_cast<int>(filtered.size()); ++i) {
        int evIdx = filtered[startIdx + i];
        const auto& ev = events[evIdx];

        float cy = panelY + i * (cardH + cardGap);
        bool isSel = (evIdx == selectedEventIdx);
        bool hovered = (mouseX >= col1X && mouseX <= col1X + col1W && mouseY >= cy && mouseY <= cy + cardH);

        Vec4 bg = isSel ? Vec4(0.12f, 0.22f, 0.34f, 0.95f)
                        : (hovered ? Vec4(0.13f, 0.17f, 0.24f, 0.90f) : Vec4(0.08f, 0.10f, 0.15f, 0.85f));

        Vec4 border = isSel ? ev.statusColor
                            : (hovered ? Vec4(0.45f, 0.65f, 0.85f, 0.8f) : Vec4(0.18f, 0.24f, 0.34f, 0.6f));

        ui->drawRect(col1X, cy, col1W, cardH, bg);
        ui->drawRectOutline(col1X, cy, col1W, cardH, isSel ? 1.8f : 1.0f, border);

        if (isSel) {
            ui->drawRect(col1X, cy, 4.0f, cardH, ev.statusColor);
        }

        // Version Badge
        float badgeW = ev.version.length() * 6.0f * 1.25f + 14.0f;
        ui->drawRect(col1X + 12.0f, cy + 9.0f, badgeW, 17.0f, {0.05f, 0.08f, 0.12f, 0.85f});
        ui->drawRectOutline(col1X + 12.0f, cy + 9.0f, badgeW, 17.0f, 1.0f, ev.statusColor);
        ui->drawText(ev.version, col1X + 18.0f, cy + 12.0f, 1.20f, ev.statusColor);

        // Status Pill
        float statusX = col1X + badgeW + 20.0f;
        ui->drawText(ev.status, statusX, cy + 12.0f, 1.15f, ev.statusColor);

        // Title
        ui->drawText(ev.title, col1X + 14.0f, cy + 32.0f, 1.45f, isSel ? Vec4(1.0f, 1.0f, 1.0f, 1.0f) : Vec4(0.85f, 0.90f, 0.95f, 0.95f));

        // Date
        float dateW = ev.targetDate.length() * 6.0f * 1.10f;
        ui->drawText(ev.targetDate, col1X + col1W - dateW - 12.0f, cy + 12.0f, 1.10f, {0.55f, 0.65f, 0.78f, 0.85f});

        if (hovered && clicked) {
            selectedEventIdx = evIdx;
        }
    }

    // Vertical Separator
    float divX = col1X + col1W + 18.0f;
    ui->drawRect(divX, panelY, 1.5f, panelH, {0.18f, 0.25f, 0.35f, 0.65f});

    // Right Column: Detailed Showcase Inspector
    float col2X = divX + 18.0f;
    float col2W = panelW - (col2X - panelX) - 25.0f;

    if (selectedEventIdx >= 0 && selectedEventIdx < static_cast<int>(events.size())) {
        const auto& sel = events[selectedEventIdx];

        // Inspector Card Background
        ui->drawRect(col2X, panelY, col2W, panelH, {0.06f, 0.08f, 0.12f, 0.92f});
        ui->drawRectOutline(col2X, panelY, col2W, panelH, 1.2f, {0.20f, 0.32f, 0.45f, 0.65f});

        // Accent top bar
        ui->drawRect(col2X, panelY, col2W, 3.0f, sel.statusColor);

        float curY = panelY + 16.0f;

        // Header: Version + Status
        float bW = sel.version.length() * 6.0f * 1.4f + 16.0f;
        ui->drawRect(col2X + 20.0f, curY, bW, 22.0f, {0.08f, 0.14f, 0.20f, 0.9f});
        ui->drawRectOutline(col2X + 20.0f, curY, bW, 22.0f, 1.2f, sel.statusColor);
        ui->drawText(sel.version, col2X + 26.0f, curY + 4.0f, 1.4f, sel.statusColor);

        ui->drawText(sel.status, col2X + bW + 30.0f, curY + 4.0f, 1.35f, sel.statusColor);

        // Date right-aligned
        std::string dateLabel = "Target Date: " + sel.targetDate;
        float dlW = dateLabel.length() * 6.0f * 1.25f;
        ui->drawText(dateLabel, col2X + col2W - dlW - 20.0f, curY + 4.0f, 1.25f, {0.65f, 0.75f, 0.88f, 0.9f});

        curY += 34.0f;

        // Big Title
        ui->drawText(sel.title, col2X + 20.0f, curY, 2.1f, {1.0f, 0.95f, 0.75f, 1.0f});
        curY += 30.0f;

        // Progress Bar
        float pbW = col2W - 40.0f;
        float pbH = 14.0f;
        float progFrac = static_cast<float>(sel.progressPercent) / 100.0f;

        ui->drawRect(col2X + 20.0f, curY, pbW, pbH, {0.04f, 0.06f, 0.09f, 0.95f});
        ui->drawRectOutline(col2X + 20.0f, curY, pbW, pbH, 1.0f, {0.20f, 0.32f, 0.45f, 0.7f});
        ui->drawRect(col2X + 20.0f, curY, pbW * progFrac, pbH, sel.statusColor);

        std::string progText = "Development Progress: " + std::to_string(sel.progressPercent) + "%";
        ui->drawText(progText, col2X + 20.0f, curY + pbH + 6.0f, 1.15f, {0.55f, 0.70f, 0.85f, 0.9f});

        curY += pbH + 24.0f;

        // Thematic Description
        ui->drawText("OVERVIEW & THEME", col2X + 20.0f, curY, 1.4f, {0.35f, 0.85f, 1.0f, 1.0f});
        curY += 20.0f;

        ui->drawText(sel.description, col2X + 20.0f, curY, 1.25f, {0.80f, 0.85f, 0.92f, 0.95f});
        curY += 34.0f;

        // Key Features Bullet Points
        ui->drawText("KEY FEATURES & ADDITIONS", col2X + 20.0f, curY, 1.4f, {0.35f, 0.85f, 1.0f, 1.0f});
        curY += 22.0f;

        for (size_t k = 0; k < sel.highlights.size(); ++k) {
            // Bullet symbol
            ui->drawText(">", col2X + 24.0f, curY, 1.35f, sel.statusColor);
            ui->drawText(sel.highlights[k], col2X + 38.0f, curY, 1.25f, {0.92f, 0.94f, 0.98f, 1.0f});
            curY += 22.0f;
        }

        curY += 8.0f;

        // Tech Details Box
        float techBoxH = 50.0f;
        ui->drawRect(col2X + 20.0f, curY, pbW, techBoxH, {0.04f, 0.06f, 0.08f, 0.85f});
        ui->drawRectOutline(col2X + 20.0f, curY, pbW, techBoxH, 1.0f, {0.18f, 0.28f, 0.40f, 0.6f});

        ui->drawText("ENGINE & ARCHITECTURE:", col2X + 28.0f, curY + 8.0f, 1.15f, {0.95f, 0.78f, 0.25f, 1.0f});
        ui->drawText(sel.techDetails, col2X + 28.0f, curY + 26.0f, 1.15f, {0.65f, 0.78f, 0.88f, 0.9f});
    }
}

// Days in month helper
static int getDaysInMonth(int year, int month) {
    if (month == 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return leap ? 29 : 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    return 31;
}

// Day of week for 1st of month (0 = Monday, 6 = Sunday)
static int getFirstDayOfWeek(int year, int month) {
    int y = year;
    int m = month;
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    int dow = (y + y/4 - y/100 + y/400 + t[m-1] + 1) % 7; // 0 = Sunday
    return (dow + 6) % 7; // Convert to 0 = Monday
}

static std::string getMonthName(int month) {
    static const char* names[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
    if (month >= 1 && month <= 12) return names[month - 1];
    return "Month";
}

void UpdateCalendarUI::renderCalendarGrid(float panelX, float panelY, float panelW, float panelH,
                                         int mouseX, int mouseY, bool clicked, float totalTime) {
    float col1X = panelX + 25.0f;
    float col1W = 540.0f;

    // Month & Year Header Controller
    float navY = panelY + 6.0f;
    float navBtnW = 32.0f;
    float navBtnH = 26.0f;

    if (drawButton(col1X, navY, navBtnW, navBtnH, "<", false, mouseX, mouseY, clicked, {0.3f, 0.7f, 1.0f, 1.0f})) {
        prevMonth();
    }

    std::string monthHeader = getMonthName(calendarMonth) + " " + std::to_string(calendarYear);
    float mhW = monthHeader.length() * 6.0f * 1.8f;
    ui->drawText(monthHeader, col1X + (col1W - mhW) * 0.5f, navY + 4.0f, 1.8f, {1.0f, 0.85f, 0.25f, 1.0f});

    if (drawButton(col1X + col1W - navBtnW, navY, navBtnW, navBtnH, ">", false, mouseX, mouseY, clicked, {0.3f, 0.7f, 1.0f, 1.0f})) {
        nextMonth();
    }

    // Days of the week header
    float dowY = navY + 36.0f;
    float cellW = (col1W - 6 * 4.0f) / 7.0f;
    float cellH = 48.0f;
    static const char* dowNames[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};

    for (int i = 0; i < 7; ++i) {
        float dx = col1X + i * (cellW + 4.0f);
        ui->drawRect(dx, dowY, cellW, 20.0f, {0.06f, 0.09f, 0.13f, 0.8f});
        ui->drawTextCentered(dowNames[i], dx + cellW * 0.5f, dowY + 4.0f, 1.15f, {0.45f, 0.75f, 0.95f, 1.0f});
    }

    // Calendar Grid Days
    int firstDow = getFirstDayOfWeek(calendarYear, calendarMonth);
    int totalDays = getDaysInMonth(calendarYear, calendarMonth);

    float gridStartY = dowY + 26.0f;
    int currentDayNum = 1;

    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            float cx = col1X + col * (cellW + 4.0f);
            float cy = gridStartY + row * (cellH + 4.0f);

            int cellIndex = row * 7 + col;
            if (cellIndex < firstDow || currentDayNum > totalDays) {
                // Empty padding slot
                ui->drawRect(cx, cy, cellW, cellH, {0.05f, 0.06f, 0.09f, 0.35f});
                ui->drawRectOutline(cx, cy, cellW, cellH, 1.0f, {0.12f, 0.15f, 0.22f, 0.25f});
            } else {
                int day = currentDayNum++;

                // Check if any event falls on this day
                const CalendarEvent* dayEv = nullptr;
                int dayEvIdx = -1;
                for (size_t e = 0; e < events.size(); ++e) {
                    if (events[e].year == calendarYear && events[e].month == calendarMonth && events[e].day == day) {
                        dayEv = &events[e];
                        dayEvIdx = static_cast<int>(e);
                        break;
                    }
                }

                bool isToday = (calendarYear == 2026 && calendarMonth == 9 && day == 13);
                bool isEvent = (dayEv != nullptr);
                bool isHovered = (mouseX >= cx && mouseX <= cx + cellW && mouseY >= cy && mouseY <= cy + cellH);
                bool isSelected = (isEvent && selectedEventIdx == dayEvIdx);

                Vec4 bg = isSelected ? Vec4(0.15f, 0.28f, 0.42f, 0.95f)
                                     : (isEvent ? Vec4(0.12f, 0.18f, 0.26f, 0.92f)
                                                : (isHovered ? Vec4(0.10f, 0.14f, 0.20f, 0.85f) : Vec4(0.06f, 0.08f, 0.12f, 0.75f)));

                ui->drawRect(cx, cy, cellW, cellH, bg);

                if (isSelected) {
                    ui->drawRectOutline(cx, cy, cellW, cellH, 2.0f, dayEv->statusColor);
                } else if (isEvent) {
                    ui->drawRectOutline(cx, cy, cellW, cellH, 1.5f, dayEv->statusColor);
                } else if (isToday) {
                    ui->drawRectOutline(cx, cy, cellW, cellH, 1.5f, {0.2f, 0.85f, 1.0f, 0.9f});
                } else {
                    ui->drawRectOutline(cx, cy, cellW, cellH, 1.0f, {0.15f, 0.20f, 0.30f, 0.45f});
                }

                // Day number
                std::string dayStr = std::to_string(day);
                ui->drawText(dayStr, cx + 5.0f, cy + 4.0f, 1.2f, isToday ? Vec4(0.3f, 0.95f, 1.0f, 1.0f) : Vec4(0.85f, 0.90f, 0.95f, 0.9f));

                // Event marker badge on day
                if (isEvent) {
                    float pulse = 0.85f + 0.15f * std::sin(totalTime * 3.5f);
                    ui->drawRect(cx + 4.0f, cy + cellH - 16.0f, cellW - 8.0f, 12.0f, {dayEv->statusColor.x * 0.3f, dayEv->statusColor.y * 0.3f, dayEv->statusColor.z * 0.3f, 0.9f});
                    ui->drawRectOutline(cx + 4.0f, cy + cellH - 16.0f, cellW - 8.0f, 12.0f, 1.0f, {dayEv->statusColor.x * pulse, dayEv->statusColor.y * pulse, dayEv->statusColor.z * pulse, 1.0f});

                    std::string badgeText = dayEv->version;
                    ui->drawTextCentered(badgeText, cx + cellW * 0.5f, cy + cellH - 14.0f, 0.95f, {1.0f, 1.0f, 1.0f, 1.0f});
                } else if (isToday) {
                    ui->drawTextCentered("TODAY", cx + cellW * 0.5f, cy + cellH - 14.0f, 0.85f, {0.3f, 0.85f, 1.0f, 0.85f});
                }

                if (isHovered && clicked && isEvent) {
                    selectedEventIdx = dayEvIdx;
                }
            }
        }
        if (currentDayNum > totalDays && row >= 4) break;
    }

    // Vertical Divider
    float divX = col1X + col1W + 18.0f;
    ui->drawRect(divX, panelY, 1.5f, panelH, {0.18f, 0.25f, 0.35f, 0.65f});

    // Right Column: Event Detail Inspector for Selected Event
    float col2X = divX + 18.0f;
    float col2W = panelW - (col2X - panelX) - 25.0f;

    if (selectedEventIdx >= 0 && selectedEventIdx < static_cast<int>(events.size())) {
        const auto& sel = events[selectedEventIdx];

        ui->drawRect(col2X, panelY, col2W, panelH, {0.06f, 0.08f, 0.12f, 0.92f});
        ui->drawRectOutline(col2X, panelY, col2W, panelH, 1.2f, {0.20f, 0.32f, 0.45f, 0.65f});
        ui->drawRect(col2X, panelY, col2W, 3.0f, sel.statusColor);

        float curY = panelY + 16.0f;

        // Version + Status
        float bW = sel.version.length() * 6.0f * 1.4f + 16.0f;
        ui->drawRect(col2X + 18.0f, curY, bW, 22.0f, {0.08f, 0.14f, 0.20f, 0.9f});
        ui->drawRectOutline(col2X + 18.0f, curY, bW, 22.0f, 1.2f, sel.statusColor);
        ui->drawText(sel.version, col2X + 24.0f, curY + 4.0f, 1.4f, sel.statusColor);

        ui->drawText(sel.status, col2X + bW + 28.0f, curY + 4.0f, 1.35f, sel.statusColor);

        curY += 34.0f;
        ui->drawText(sel.title, col2X + 18.0f, curY, 2.0f, {1.0f, 0.95f, 0.75f, 1.0f});
        curY += 28.0f;

        std::string dateLabel = "Scheduled Date: " + sel.targetDate;
        ui->drawText(dateLabel, col2X + 18.0f, curY, 1.25f, {0.4f, 0.85f, 1.0f, 1.0f});
        curY += 26.0f;

        // Overview
        ui->drawText(sel.description, col2X + 18.0f, curY, 1.20f, {0.80f, 0.86f, 0.92f, 0.95f});
        curY += 36.0f;

        // Features list
        ui->drawText("FEATURE HIGHLIGHTS", col2X + 18.0f, curY, 1.35f, {0.35f, 0.85f, 1.0f, 1.0f});
        curY += 20.0f;

        for (size_t k = 0; k < sel.highlights.size() && k < 4; ++k) {
            ui->drawText(">", col2X + 22.0f, curY, 1.30f, sel.statusColor);
            ui->drawText(sel.highlights[k], col2X + 36.0f, curY, 1.20f, {0.92f, 0.94f, 0.98f, 1.0f});
            curY += 22.0f;
        }

        curY += 10.0f;
        float pbW = col2W - 36.0f;
        float techBoxH = 46.0f;
        ui->drawRect(col2X + 18.0f, curY, pbW, techBoxH, {0.04f, 0.06f, 0.08f, 0.85f});
        ui->drawRectOutline(col2X + 18.0f, curY, pbW, techBoxH, 1.0f, {0.18f, 0.28f, 0.40f, 0.6f});

        ui->drawText("ENGINE NOTES:", col2X + 24.0f, curY + 6.0f, 1.10f, {0.95f, 0.78f, 0.25f, 1.0f});
        ui->drawText(sel.techDetails, col2X + 24.0f, curY + 24.0f, 1.10f, {0.65f, 0.78f, 0.88f, 0.9f});
    }
}

} // namespace Aetheria
