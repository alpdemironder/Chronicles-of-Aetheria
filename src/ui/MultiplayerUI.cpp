#include "MultiplayerUI.hpp"
#include <cmath>
#include <iostream>
#include <cstdlib>

namespace Aetheria {

MultiplayerUI::MultiplayerUI(Net::NetworkClient* client, Net::AetheriaServer* localServer)
    : client(client), localServer(localServer) {
    // 1. Initialize default local worlds
    myWorlds.push_back({"Aetheria Kingdom (Ana Krallik)", 1337, 25565, "Survival", "Varsayilan RPG Dunyasi"});
    myWorlds.push_back({"Pal & Creature Sanctuary", 4242, 25566, "Creative", "Dost Canlilar Vadisi"});
    myWorlds.push_back({"Mystic Caldera Realm", 7777, 25567, "Hardcore", "Volkanik Magma Vadisi"});

    // 2. Initialize default remote servers
    otherServers.push_back({"Yerel Ag Dunyasi (LAN Discovery)", "127.0.0.1:25565", 1, true, "Yerel Agdaki Arkadas Dunyasi"});
    otherServers.push_back({"Aetheria Resmi Sunucusu (Official)", "play.aetheria-rpg.net:25565", 18, true, "7/24 Kesintisiz Macera RPG"});
    otherServers.push_back({"Arkadasimin Dunyasi (Co-op)", "192.168.1.100:25565", 14, true, "Ozel Co-op Arkadas Dunyasi"});
}

void MultiplayerUI::onCharInput(char c) {
    if (!open) return;

    if (activeField == ActiveField::PlayerName) {
        if (c >= 32 && c <= 126 && playerName.length() < 24) playerName += c;
    } else if (activeField == ActiveField::ServerAddress) {
        if (c >= 32 && c <= 126 && serverAddress.length() < 40) serverAddress += c;
    } else if (activeField == ActiveField::NewWorldName) {
        if (c >= 32 && c <= 126 && newWorldName.length() < 30) newWorldName += c;
    } else if (activeField == ActiveField::NewWorldSeed) {
        if (c >= '0' && c <= '9' && newWorldSeed.length() < 10) newWorldSeed += c;
    } else if (activeField == ActiveField::NewWorldPort) {
        if (c >= '0' && c <= '9' && newWorldPort.length() < 5) newWorldPort += c;
    } else if (activeField == ActiveField::NewServerName) {
        if (c >= 32 && c <= 126 && newServerName.length() < 30) newServerName += c;
    } else if (activeField == ActiveField::NewServerAddress) {
        if (c >= 32 && c <= 126 && newServerAddress.length() < 40) newServerAddress += c;
    }
}

void MultiplayerUI::onKeyDown(int key) {
    if (!open) return;

    if (key == 8) { // Backspace
        if (activeField == ActiveField::PlayerName && !playerName.empty()) playerName.pop_back();
        else if (activeField == ActiveField::ServerAddress && !serverAddress.empty()) serverAddress.pop_back();
        else if (activeField == ActiveField::NewWorldName && !newWorldName.empty()) newWorldName.pop_back();
        else if (activeField == ActiveField::NewWorldSeed && !newWorldSeed.empty()) newWorldSeed.pop_back();
        else if (activeField == ActiveField::NewWorldPort && !newWorldPort.empty()) newWorldPort.pop_back();
        else if (activeField == ActiveField::NewServerName && !newServerName.empty()) newServerName.pop_back();
        else if (activeField == ActiveField::NewServerAddress && !newServerAddress.empty()) newServerAddress.pop_back();
    } else if (key == 9) { // Tab key switches active fields
        if (activeField == ActiveField::NewWorldName) activeField = ActiveField::NewWorldSeed;
        else if (activeField == ActiveField::NewWorldSeed) activeField = ActiveField::NewWorldPort;
        else if (activeField == ActiveField::NewWorldPort) activeField = ActiveField::NewWorldName;
        else if (activeField == ActiveField::NewServerName) activeField = ActiveField::NewServerAddress;
        else if (activeField == ActiveField::NewServerAddress) activeField = ActiveField::NewServerName;
        else if (activeField == ActiveField::PlayerName) activeField = ActiveField::ServerAddress;
        else activeField = ActiveField::PlayerName;
    } else if (key == 13) { // Enter key closes input
        activeField = ActiveField::None;
    }
}

bool MultiplayerUI::drawButton(UIRenderer* ui, float x, float y, float w, float h,
                               const std::string& text, bool primary,
                               int mouseX, int mouseY, bool clicked) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = primary ? (hovered ? Vec4(0.20f, 0.65f, 0.95f, 0.95f) : Vec4(0.12f, 0.42f, 0.72f, 0.92f))
                      : (hovered ? Vec4(0.24f, 0.32f, 0.44f, 0.92f) : Vec4(0.11f, 0.16f, 0.24f, 0.88f));

    Vec4 border = primary ? (hovered ? Vec4(0.85f, 0.96f, 1.0f, 1.0f) : Vec4(0.40f, 0.75f, 1.0f, 0.9f))
                          : (hovered ? Vec4(0.70f, 0.85f, 1.0f, 0.95f) : Vec4(0.28f, 0.38f, 0.52f, 0.75f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, primary ? 2.0f : 1.5f, border);

    if (hovered) {
        ui->drawRect(x, y, 4.0f, 4.0f, {1.0f, 1.0f, 1.0f, 0.9f});
        ui->drawRect(x + w - 4.0f, y, 4.0f, 4.0f, {1.0f, 1.0f, 1.0f, 0.9f});
        ui->drawRect(x, y + h - 4.0f, 4.0f, 4.0f, {1.0f, 1.0f, 1.0f, 0.9f});
        ui->drawRect(x + w - 4.0f, y + h - 4.0f, 4.0f, 4.0f, {1.0f, 1.0f, 1.0f, 0.9f});
    }

    float textScale = primary ? 1.70f : 1.50f;
    float tx = x + (w - text.length() * 6.0f * textScale) * 0.5f;
    float ty = y + (h - 7.0f * textScale) * 0.5f;

    Vec4 textColor = primary ? (hovered ? Vec4(1.0f, 1.0f, 1.0f, 1.0f) : Vec4(0.92f, 0.96f, 1.0f, 1.0f))
                             : (hovered ? Vec4(1.0f, 0.95f, 0.70f, 1.0f) : Vec4(0.85f, 0.90f, 0.95f, 0.95f));

    ui->drawText(text, tx, ty, textScale, textColor);
    return hovered && clicked;
}

bool MultiplayerUI::drawTabButton(UIRenderer* ui, float x, float y, float w, float h,
                                  const std::string& text, bool isActive,
                                  int mouseX, int mouseY, bool clicked) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = isActive ? Vec4(0.12f, 0.35f, 0.58f, 0.95f)
                       : (hovered ? Vec4(0.18f, 0.24f, 0.34f, 0.85f) : Vec4(0.08f, 0.11f, 0.16f, 0.75f));

    Vec4 border = isActive ? Vec4(0.35f, 0.85f, 1.0f, 1.0f)
                           : (hovered ? Vec4(0.50f, 0.65f, 0.85f, 0.8f) : Vec4(0.20f, 0.28f, 0.38f, 0.6f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, isActive ? 2.0f : 1.0f, border);

    // Bottom luminous underline for active tab
    if (isActive) {
        ui->drawRect(x, y + h - 3.0f, w, 3.0f, {0.35f, 0.90f, 1.0f, 1.0f});
    }

    float textScale = 1.6f;
    float tx = x + (w - text.length() * 6.0f * textScale) * 0.5f;
    float ty = y + (h - 7.0f * textScale) * 0.5f;

    Vec4 textColor = isActive ? Vec4(1.0f, 0.92f, 0.45f, 1.0f)
                              : (hovered ? Vec4(1.0f, 1.0f, 1.0f, 1.0f) : Vec4(0.75f, 0.82f, 0.90f, 0.85f));

    ui->drawText(text, tx, ty, textScale, textColor);
    return hovered && clicked;
}

void MultiplayerUI::drawInputField(UIRenderer* ui, float x, float y, float w, float h,
                                   const std::string& label, const std::string& value,
                                   bool isFocused, int mouseX, int mouseY, bool clicked,
                                   ActiveField fieldType, float totalTime) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);
    if (hovered && clicked) {
        activeField = fieldType;
    }

    if (!label.empty()) {
        ui->drawText(label, x + 2.0f, y - 18.0f, 1.35f, {0.85f, 0.90f, 0.95f, 0.95f});
    }

    Vec4 bg = isFocused ? Vec4(0.08f, 0.16f, 0.26f, 0.95f) : (hovered ? Vec4(0.06f, 0.12f, 0.18f, 0.90f) : Vec4(0.04f, 0.08f, 0.13f, 0.85f));
    Vec4 border = isFocused ? Vec4(0.35f, 0.88f, 1.0f, 1.0f) : (hovered ? Vec4(0.40f, 0.65f, 0.85f, 0.8f) : Vec4(0.20f, 0.32f, 0.45f, 0.6f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, isFocused ? 2.0f : 1.2f, border);

    std::string displayText = value;
    if (isFocused && std::fmod(totalTime, 0.8f) < 0.4f) {
        displayText += "_";
    }

    ui->drawText(displayText, x + 10.0f, y + (h - 13.0f) * 0.5f, 1.65f, {1.0f, 1.0f, 1.0f, 1.0f});
}

void MultiplayerUI::render(UIRenderer* ui, int screenWidth, int screenHeight,
                           int mouseX, int mouseY, bool mouseLeftClicked,
                           float totalTime) {
    if (!open) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);
    float cx = sw * 0.5f;

    // Dark cinematic backdrop
    ui->drawRect(0, 0, sw, sh, {0.02f, 0.04f, 0.07f, 0.85f});

    // Main Dialog Chassis (Expanded 740x550)
    float panelW = 740.0f;
    float panelH = 550.0f;
    float px = cx - panelW * 0.5f;
    float py = sh * 0.5f - panelH * 0.5f;

    ui->drawRect(px, py, panelW, panelH, {0.05f, 0.08f, 0.13f, 0.96f});
    ui->drawRectOutline(px, py, panelW, panelH, 2.0f, {0.25f, 0.55f, 0.85f, 0.85f});

    // Header Bar
    ui->drawRect(px, py, panelW, 52.0f, {0.07f, 0.13f, 0.22f, 0.95f});
    ui->drawRect(px, py + 50.0f, panelW, 2.0f, {0.35f, 0.70f, 1.0f, 0.9f});

    ui->drawTextCentered("CO-OP MULTIPLAYER // COK OYUNCULU DUNYALAR", cx, py + 12.0f, 2.2f, {1.0f, 0.88f, 0.35f, 1.0f});
    ui->drawTextCentered("Kendi dunyalarini yerel agda ac veya arkadaslarinin sunucularina baglan!", cx, py + 34.0f, 1.25f, {0.6f, 0.85f, 1.0f, 0.9f});

    // =========================================================================
    // TOP TAB NAVIGATION BAR (3 TABS)
    // =========================================================================
    float tabY = py + 58.0f;
    float tabW = (panelW - 40.0f) / 3.0f;
    float tabH = 36.0f;

    if (drawTabButton(ui, px + 20.0f + 0 * tabW, tabY, tabW - 4.0f, tabH,
                      "[ 1. KENDI DUNYALARIM ]", activeTab == MultiplayerTab::MyWorlds,
                      mouseX, mouseY, mouseLeftClicked)) {
        activeTab = MultiplayerTab::MyWorlds;
        isCreatingWorld = false;
        activeField = ActiveField::None;
    }

    if (drawTabButton(ui, px + 20.0f + 1 * tabW, tabY, tabW - 4.0f, tabH,
                      "[ 2. DIGER DUNYALAR ]", activeTab == MultiplayerTab::OtherWorlds,
                      mouseX, mouseY, mouseLeftClicked)) {
        activeTab = MultiplayerTab::OtherWorlds;
        isAddingServer = false;
        activeField = ActiveField::None;
    }

    if (drawTabButton(ui, px + 20.0f + 2 * tabW, tabY, tabW - 4.0f, tabH,
                      "[ 3. OYUNCU PROFILI ]", activeTab == MultiplayerTab::PlayerProfile,
                      mouseX, mouseY, mouseLeftClicked)) {
        activeTab = MultiplayerTab::PlayerProfile;
        activeField = ActiveField::None;
    }

    // Tab content area background
    float contentY = tabY + tabH + 8.0f;
    float contentH = 370.0f;
    float contentW = panelW - 40.0f;
    float contentX = px + 20.0f;

    ui->drawRect(contentX, contentY, contentW, contentH, {0.03f, 0.06f, 0.10f, 0.70f});
    ui->drawRectOutline(contentX, contentY, contentW, contentH, 1.0f, {0.18f, 0.30f, 0.45f, 0.5f});

    // =========================================================================
    // TAB 1: KENDI DUNYALARIM (HOST / PLAY SOLO / CREATE NEW WORLD)
    // =========================================================================
    if (activeTab == MultiplayerTab::MyWorlds) {
        if (isCreatingWorld) {
            // --- World Creation Form ---
            ui->drawText("YENI DUNYA OLUSTURMA PANELI", contentX + 20.0f, contentY + 20.0f, 1.9f, {1.0f, 0.88f, 0.35f, 1.0f});
            ui->drawText("Dunya adi, tohum kodu (seed) ve yerel port numarasini belirleyin:", contentX + 20.0f, contentY + 44.0f, 1.3f, {0.75f, 0.85f, 0.95f, 0.9f});

            drawInputField(ui, contentX + 20.0f, contentY + 85.0f, 320.0f, 34.0f,
                           "DUNYA ADI:", newWorldName, activeField == ActiveField::NewWorldName,
                           mouseX, mouseY, mouseLeftClicked, ActiveField::NewWorldName, totalTime);

            drawInputField(ui, contentX + 20.0f, contentY + 145.0f, 200.0f, 34.0f,
                           "TOHUM (SEED):", newWorldSeed, activeField == ActiveField::NewWorldSeed,
                           mouseX, mouseY, mouseLeftClicked, ActiveField::NewWorldSeed, totalTime);

            drawInputField(ui, contentX + 240.0f, contentY + 145.0f, 100.0f, 34.0f,
                           "PORT:", newWorldPort, activeField == ActiveField::NewWorldPort,
                           mouseX, mouseY, mouseLeftClicked, ActiveField::NewWorldPort, totalTime);

            // Random seed button
            if (drawButton(ui, contentX + 360.0f, contentY + 145.0f, 130.0f, 34.0f, "RASTGELE SEED", false, mouseX, mouseY, mouseLeftClicked)) {
                uint32_t r1 = static_cast<uint32_t>(std::rand()) ^ (static_cast<uint32_t>(std::rand()) << 15);
                uint32_t r2 = static_cast<uint32_t>(std::rand()) << 30;
                uint32_t r = (r1 ^ r2) ^ static_cast<uint32_t>(std::rand() * 1664525u + 1013904223u);
                if (r == 0) r = 133742u;
                newWorldSeed = std::to_string(r);
            }

            // Create submit button
            if (drawButton(ui, contentX + 20.0f, contentY + 220.0f, 180.0f, 38.0f, "> DUNYAYI OLUSTUR <", true, mouseX, mouseY, mouseLeftClicked)) {
                uint32_t s = 1337;
                try { s = static_cast<uint32_t>(std::stoul(newWorldSeed)); } catch (...) {}
                uint16_t p = 25565;
                try { p = static_cast<uint16_t>(std::stoul(newWorldPort)); } catch (...) {}

                myWorlds.push_back({newWorldName, s, p, "Survival", "Ozel Olusturulan Dunya"});
                selectedWorldIndex = static_cast<int>(myWorlds.size()) - 1;
                isCreatingWorld = false;
                activeField = ActiveField::None;
            }

            // Cancel button
            if (drawButton(ui, contentX + 220.0f, contentY + 220.0f, 120.0f, 38.0f, "IPTAL", false, mouseX, mouseY, mouseLeftClicked)) {
                isCreatingWorld = false;
                activeField = ActiveField::None;
            }
        } else {
            // --- List of Local Worlds ---
            ui->drawText("KENDI YEREL DUNYALARINIZ (Secip Baslatin veya Yerel Agda Paylasin):",
                         contentX + 16.0f, contentY + 14.0f, 1.4f, {0.85f, 0.90f, 0.98f, 0.95f});

            float cardY = contentY + 36.0f;
            float cardW = contentW - 32.0f;
            float cardH = 58.0f;
            float cardGap = 8.0f;

            for (size_t i = 0; i < myWorlds.size() && i < 4; ++i) {
                bool isSelected = (selectedWorldIndex == static_cast<int>(i));
                float cy = cardY + i * (cardH + cardGap);
                bool cardHovered = (mouseX >= contentX + 16.0f && mouseX <= contentX + 16.0f + cardW &&
                                    mouseY >= cy && mouseY <= cy + cardH);

                if (cardHovered && mouseLeftClicked) {
                    selectedWorldIndex = static_cast<int>(i);
                }

                Vec4 cBg = isSelected ? Vec4(0.12f, 0.32f, 0.52f, 0.95f)
                                      : (cardHovered ? Vec4(0.08f, 0.16f, 0.25f, 0.85f) : Vec4(0.05f, 0.09f, 0.15f, 0.80f));
                Vec4 cBorder = isSelected ? Vec4(0.40f, 0.88f, 1.0f, 1.0f)
                                          : (cardHovered ? Vec4(0.35f, 0.60f, 0.85f, 0.8f) : Vec4(0.18f, 0.28f, 0.40f, 0.6f));

                ui->drawRect(contentX + 16.0f, cy, cardW, cardH, cBg);
                ui->drawRectOutline(contentX + 16.0f, cy, cardW, cardH, isSelected ? 2.0f : 1.0f, cBorder);

                // World icon box
                ui->drawRect(contentX + 22.0f, cy + 8.0f, 42.0f, 42.0f, {0.18f, 0.40f, 0.22f, 0.9f});
                ui->drawRectOutline(contentX + 22.0f, cy + 8.0f, 42.0f, 42.0f, 1.0f, {0.4f, 0.85f, 0.45f, 0.8f});
                ui->drawText("RPG", contentX + 31.0f, cy + 22.0f, 1.3f, {1.0f, 1.0f, 1.0f, 1.0f});

                // Title & Details
                ui->drawText(myWorlds[i].name, contentX + 74.0f, cy + 12.0f, 1.7f, isSelected ? Vec4(1.0f, 0.92f, 0.45f, 1.0f) : Vec4(1.0f, 1.0f, 1.0f, 1.0f));

                std::string meta = "Seed: " + std::to_string(myWorlds[i].seed) + "  |  Mod: " + myWorlds[i].gameMode +
                                   "  |  Port: " + std::to_string(myWorlds[i].port) + "  |  " + myWorlds[i].description;
                ui->drawText(meta, contentX + 74.0f, cy + 34.0f, 1.25f, {0.65f, 0.80f, 0.92f, 0.85f});

                // Status Tag
                std::string tag = isSelected ? "[SECILI]" : "[HAZIR]";
                Vec4 tagCol = isSelected ? Vec4(0.35f, 0.95f, 0.45f, 1.0f) : Vec4(0.6f, 0.7f, 0.8f, 0.7f);
                ui->drawText(tag, contentX + cardW - 70.0f, cy + 22.0f, 1.35f, tagCol);
            }

            // Action Buttons for My Worlds
            float actY = contentY + 285.0f;
            float bW = 210.0f;
            float bH = 38.0f;

            // 1. Host & Play (Start Server & Connect as Host)
            if (drawButton(ui, contentX + 16.0f, actY, bW, bH, "> DUNYAYI BASLAT (HOST) <", true, mouseX, mouseY, mouseLeftClicked)) {
                if (selectedWorldIndex >= 0 && selectedWorldIndex < static_cast<int>(myWorlds.size())) {
                    const auto& w = myWorlds[selectedWorldIndex];
                    if (localServer && client) {
                        if (!localServer->isRunning()) {
                            localServer->startAsync(w.port);
                        }
                        statusText = "Dunya baslatildi! Port: " + std::to_string(w.port);
                        client->connect("127.0.0.1", w.port, playerName);
                    }
                    if (onStartGameConfig) {
                        onStartGameConfig(w.seed, true, w.port, "127.0.0.1");
                    } else if (onStartGame) {
                        onStartGame();
                    }
                    open = false;
                }
            }

            // 2. Play Solo
            if (drawButton(ui, contentX + 236.0f, actY, 150.0f, bH, "TEK BASINA OYNA", false, mouseX, mouseY, mouseLeftClicked)) {
                if (selectedWorldIndex >= 0 && selectedWorldIndex < static_cast<int>(myWorlds.size())) {
                    const auto& w = myWorlds[selectedWorldIndex];
                    if (onStartGameConfig) {
                        onStartGameConfig(w.seed, false, 0, "");
                    } else if (onStartGame) {
                        onStartGame();
                    }
                    open = false;
                }
            }

            // 3. Create New World
            if (drawButton(ui, contentX + 396.0f, actY, 160.0f, bH, "+ YENI DUNYA", false, mouseX, mouseY, mouseLeftClicked)) {
                isCreatingWorld = true;
                newWorldName = "Yeni Dunya " + std::to_string(myWorlds.size() + 1);
                newWorldSeed = std::to_string(1000 + (std::rand() % 9000));
                newWorldPort = "25565";
            }

            // 4. Delete World
            if (myWorlds.size() > 1) {
                if (drawButton(ui, contentX + 566.0f, actY, 118.0f, bH, "SIL", false, mouseX, mouseY, mouseLeftClicked)) {
                    myWorlds.erase(myWorlds.begin() + selectedWorldIndex);
                    if (selectedWorldIndex >= static_cast<int>(myWorlds.size())) {
                        selectedWorldIndex = static_cast<int>(myWorlds.size()) - 1;
                    }
                }
            }
        }
    }

    // =========================================================================
    // TAB 2: DIGER DUNYALAR & SUNUCULAR (SERVER LIST & DIRECT CONNECT)
    // =========================================================================
    else if (activeTab == MultiplayerTab::OtherWorlds) {
        if (isAddingServer) {
            ui->drawText("YENI SUNUCU EKLEME PANELI", contentX + 20.0f, contentY + 20.0f, 1.9f, {1.0f, 0.88f, 0.35f, 1.0f});
            ui->drawText("Arkadasinizin veya topluluk sunucusunun bilgilerini girin:", contentX + 20.0f, contentY + 44.0f, 1.3f, {0.75f, 0.85f, 0.95f, 0.9f});

            drawInputField(ui, contentX + 20.0f, contentY + 85.0f, 320.0f, 34.0f,
                           "SUNUCU ADI:", newServerName, activeField == ActiveField::NewServerName,
                           mouseX, mouseY, mouseLeftClicked, ActiveField::NewServerName, totalTime);

            drawInputField(ui, contentX + 20.0f, contentY + 145.0f, 320.0f, 34.0f,
                           "SUNUCU ADRESI (IP:PORT):", newServerAddress, activeField == ActiveField::NewServerAddress,
                           mouseX, mouseY, mouseLeftClicked, ActiveField::NewServerAddress, totalTime);

            if (drawButton(ui, contentX + 20.0f, contentY + 220.0f, 160.0f, 38.0f, "> SUNUCUYU EKLE <", true, mouseX, mouseY, mouseLeftClicked)) {
                otherServers.push_back({newServerName, newServerAddress, 15, true, "Ozel Eklenen Sunucu"});
                selectedServerIndex = static_cast<int>(otherServers.size()) - 1;
                serverAddress = newServerAddress;
                isAddingServer = false;
                activeField = ActiveField::None;
            }

            if (drawButton(ui, contentX + 200.0f, contentY + 220.0f, 120.0f, 38.0f, "IPTAL", false, mouseX, mouseY, mouseLeftClicked)) {
                isAddingServer = false;
                activeField = ActiveField::None;
            }
        } else {
            ui->drawText("DIGER DUNYALAR & SUNUCU LISTESI (Katilmak icin secin veya IP girin):",
                         contentX + 16.0f, contentY + 14.0f, 1.4f, {0.85f, 0.90f, 0.98f, 0.95f});

            float cardY = contentY + 36.0f;
            float cardW = contentW - 32.0f;
            float cardH = 54.0f;
            float cardGap = 8.0f;

            for (size_t i = 0; i < otherServers.size() && i < 3; ++i) {
                bool isSelected = (selectedServerIndex == static_cast<int>(i));
                float cy = cardY + i * (cardH + cardGap);
                bool cardHovered = (mouseX >= contentX + 16.0f && mouseX <= contentX + 16.0f + cardW &&
                                    mouseY >= cy && mouseY <= cy + cardH);

                if (cardHovered && mouseLeftClicked) {
                    selectedServerIndex = static_cast<int>(i);
                    serverAddress = otherServers[i].address;
                }

                Vec4 cBg = isSelected ? Vec4(0.12f, 0.32f, 0.52f, 0.95f)
                                      : (cardHovered ? Vec4(0.08f, 0.16f, 0.25f, 0.85f) : Vec4(0.05f, 0.09f, 0.15f, 0.80f));
                Vec4 cBorder = isSelected ? Vec4(0.40f, 0.88f, 1.0f, 1.0f)
                                          : (cardHovered ? Vec4(0.35f, 0.60f, 0.85f, 0.8f) : Vec4(0.18f, 0.28f, 0.40f, 0.6f));

                ui->drawRect(contentX + 16.0f, cy, cardW, cardH, cBg);
                ui->drawRectOutline(contentX + 16.0f, cy, cardW, cardH, isSelected ? 2.0f : 1.0f, cBorder);

                // Online indicator beacon dot
                ui->drawRect(contentX + 24.0f, cy + 16.0f, 8.0f, 8.0f, otherServers[i].isOnline ? Vec4(0.2f, 0.95f, 0.35f, 1.0f) : Vec4(0.9f, 0.25f, 0.25f, 1.0f));

                // Server title & IP
                ui->drawText(otherServers[i].name, contentX + 40.0f, cy + 10.0f, 1.6f, isSelected ? Vec4(1.0f, 0.92f, 0.45f, 1.0f) : Vec4(1.0f, 1.0f, 1.0f, 1.0f));
                std::string meta = otherServers[i].address + "  |  " + otherServers[i].motd;
                ui->drawText(meta, contentX + 40.0f, cy + 30.0f, 1.25f, {0.65f, 0.80f, 0.92f, 0.85f});

                // Ping & Status
                std::string pingStr = std::to_string(otherServers[i].pingMs) + "ms";
                ui->drawText(pingStr, contentX + cardW - 75.0f, cy + 12.0f, 1.35f, {0.35f, 0.95f, 0.45f, 1.0f});
                ui->drawText("CEVRIMICI", contentX + cardW - 75.0f, cy + 30.0f, 1.15f, {0.4f, 0.85f, 1.0f, 0.8f});
            }

            // Direct Connect Address Input Field
            float dirY = contentY + 230.0f;
            drawInputField(ui, contentX + 16.0f, dirY, cardW, 36.0f,
                           "DOGRUDAN BAGLANTI ADRESI (IP:PORT):", serverAddress,
                           activeField == ActiveField::ServerAddress,
                           mouseX, mouseY, mouseLeftClicked, ActiveField::ServerAddress, totalTime);

            // Action Buttons for Other Worlds
            float actY = contentY + 288.0f;

            // 1. Join World / Connect
            if (drawButton(ui, contentX + 16.0f, actY, 240.0f, 38.0f, "> DUNYAYA KATIL (JOIN) <", true, mouseX, mouseY, mouseLeftClicked)) {
                if (client) {
                    std::string ip = serverAddress;
                    uint16_t port = Net::DEFAULT_PORT;
                    size_t colon = serverAddress.find(':');
                    if (colon != std::string::npos) {
                        ip = serverAddress.substr(0, colon);
                        port = static_cast<uint16_t>(std::atoi(serverAddress.substr(colon + 1).c_str()));
                    }
                    statusText = "Dunyaya baglaniliyor: " + ip + ":" + std::to_string(port);
                    client->connect(ip, port, playerName);
                }
                if (onStartGameConfig) {
                    uint16_t port = 25565;
                    size_t colon = serverAddress.find(':');
                    std::string ip = serverAddress;
                    if (colon != std::string::npos) {
                        ip = serverAddress.substr(0, colon);
                        port = static_cast<uint16_t>(std::atoi(serverAddress.substr(colon + 1).c_str()));
                    }
                    onStartGameConfig(0, false, port, ip);
                } else if (onStartGame) {
                    onStartGame();
                }
                open = false;
            }

            // 2. Add New Server
            if (drawButton(ui, contentX + 266.0f, actY, 170.0f, 38.0f, "+ SUNUCU EKLE", false, mouseX, mouseY, mouseLeftClicked)) {
                isAddingServer = true;
                newServerName = "Arkadas Sunucusu";
                newServerAddress = "192.168.1.100:25565";
            }

            // 3. Refresh List
            if (drawButton(ui, contentX + 446.0f, actY, 130.0f, 38.0f, "YENILE", false, mouseX, mouseY, mouseLeftClicked)) {
                statusText = "Sunucu listesi yenilendi.";
            }
        }
    }

    // =========================================================================
    // TAB 3: OYUNCU PROFILI & AYARLAR
    // =========================================================================
    else if (activeTab == MultiplayerTab::PlayerProfile) {
        ui->drawText("OYUNCU PROFILI & COK OYUNCULU AYARLARI", contentX + 20.0f, contentY + 20.0f, 1.9f, {1.0f, 0.88f, 0.35f, 1.0f});
        ui->drawText("Diger oyuncularin gorecegi takma adinizi belirleyin:", contentX + 20.0f, contentY + 44.0f, 1.3f, {0.75f, 0.85f, 0.95f, 0.9f});

        drawInputField(ui, contentX + 20.0f, contentY + 85.0f, 320.0f, 36.0f,
                       "OYUNCU ADI (NICKNAME):", playerName,
                       activeField == ActiveField::PlayerName,
                       mouseX, mouseY, mouseLeftClicked, ActiveField::PlayerName, totalTime);

        // Network diagnostic stats box
        float statBoxY = contentY + 145.0f;
        ui->drawRect(contentX + 20.0f, statBoxY, contentW - 40.0f, 110.0f, {0.04f, 0.07f, 0.12f, 0.90f});
        ui->drawRectOutline(contentX + 20.0f, statBoxY, contentW - 40.0f, 110.0f, 1.0f, {0.20f, 0.35f, 0.50f, 0.6f});

        ui->drawText("AG VE BAGLANTI BILGILERI:", contentX + 32.0f, statBoxY + 12.0f, 1.45f, {0.45f, 0.85f, 1.0f, 1.0f});
        ui->drawText("Protokol: Aetheria NetProtocol v2.4 (TCP/UDP)", contentX + 32.0f, statBoxY + 34.0f, 1.25f, {0.75f, 0.82f, 0.90f, 0.85f});
        ui->drawText("Varsayilan Port: 25565 (Yerel Ag ve LAN kesfi aktif)", contentX + 32.0f, statBoxY + 54.0f, 1.25f, {0.75f, 0.82f, 0.90f, 0.85f});
        ui->drawText("Dunya Senkronizasyonu: Voksel blok degisiklikleri, oyuncu konumu, sohbet ve esyalar", contentX + 32.0f, statBoxY + 74.0f, 1.25f, {0.75f, 0.82f, 0.90f, 0.85f});

        // Quick profile save button
        if (drawButton(ui, contentX + 20.0f, contentY + 280.0f, 180.0f, 38.0f, "PROFILI KAYDET", true, mouseX, mouseY, mouseLeftClicked)) {
            activeField = ActiveField::None;
            statusText = "Profil guncellendi: " + playerName;
        }
    }

    // =========================================================================
    // BOTTOM STATUS & EXIT CONTROLS
    // =========================================================================
    float botY = contentY + contentH + 10.0f;

    // Status Banner Box
    float statW = panelW - 220.0f;
    ui->drawRect(contentX, botY, statW, 36.0f, {0.04f, 0.06f, 0.10f, 0.90f});
    ui->drawRectOutline(contentX, botY, statW, 36.0f, 1.0f, {0.20f, 0.35f, 0.50f, 0.6f});

    Vec4 statColor{0.8f, 0.85f, 0.9f, 1.0f};
    if (client) {
        switch (client->getStatus()) {
            case Net::ClientStatus::Connected:
                statColor = {0.35f, 0.95f, 0.45f, 1.0f};
                statusText = client->getStatusMessage();
                break;
            case Net::ClientStatus::Connecting:
                statColor = {1.0f, 0.85f, 0.35f, 1.0f};
                statusText = client->getStatusMessage();
                break;
            case Net::ClientStatus::Failed:
                statColor = {1.0f, 0.40f, 0.35f, 1.0f};
                statusText = client->getStatusMessage();
                break;
            default:
                break;
        }
    }
    ui->drawText("DURUM: " + statusText, contentX + 12.0f, botY + 11.0f, 1.35f, statColor);

    // Back to Menu Button
    if (drawButton(ui, contentX + statW + 10.0f, botY, panelW - statW - 50.0f, 36.0f,
                   "ANA MENU", false, mouseX, mouseY, mouseLeftClicked)) {
        open = false;
        activeField = ActiveField::None;
        if (onBackToMenu) {
            onBackToMenu();
        }
    }
}

} // namespace Aetheria
