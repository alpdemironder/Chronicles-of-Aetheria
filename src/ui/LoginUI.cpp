#include "LoginUI.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace Aetheria {

LoginUI::LoginUI() {
    savedProfiles.push_back({"Alp", "Savasci", 12});
    savedProfiles.push_back({"DemirBilek", "Paladin", 8});
    savedProfiles.push_back({"GolgeAvcisi", "Okcu", 15});
    savedProfiles.push_back({"AuraMage", "Buyucu", 20});
}

void LoginUI::generateRandomName() {
    static const std::vector<std::string> randomNames = {
        "Alp_Gezgin", "GokTurk", "DemirBilek", "Valkyrie", "Aether_Hero",
        "EjderAvcisi", "GolgeAdim", "Firtina", "KuzeyYildizi", "YalnizKurt",
        "Atlas", "Kaya", "Bora", "Barbaros", "Turan", "Ruzgar_Ece"
    };
    int idx = std::rand() % randomNames.size();
    username = randomNames[idx];
    statusMessage = "Rastgele isim atandi: " + username;
    statusColor = {0.4f, 0.95f, 1.0f, 0.9f};
}

void LoginUI::tryLogin() {
    if (username.length() < 3) {
        statusMessage = "HATA: Oyuncu ismi en az 3 karakter olmalidir!";
        statusColor = {1.0f, 0.35f, 0.35f, 1.0f};
        return;
    }

    statusMessage = "Giris basarili! Hos geldin, " + username + "!";
    statusColor = {0.35f, 1.0f, 0.45f, 1.0f};

    if (onLoginSuccess) {
        onLoginSuccess(username, selectedClass);
    }
    open = false;
}

void LoginUI::onCharInput(char c) {
    if (!open || !isInputFocused) return;

    if (c >= 32 && c <= 126) {
        if (username.length() < 16) {
            username += c;
            statusMessage = "Isim: " + username;
            statusColor = {0.8f, 0.9f, 1.0f, 0.9f};
        }
    }
}

void LoginUI::onKeyDown(int key) {
    if (!open) return;

    // VK_BACK = 0x08
    if (key == 0x08) {
        if (!username.empty()) {
            username.pop_back();
            statusMessage = "Isim: " + username;
            statusColor = {0.8f, 0.9f, 1.0f, 0.9f};
        }
    }
    // VK_RETURN = 0x0D
    else if (key == 0x0D) {
        tryLogin();
    }
    // VK_ESCAPE = 0x1B
    else if (key == 0x1B) {
        if (onCancel) {
            onCancel();
        } else {
            open = false;
        }
    }
}

bool LoginUI::drawButton(UIRenderer* ui, float x, float y, float w, float h,
                         const std::string& text, bool primary,
                         int mouseX, int mouseY, bool clicked) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bgColor = primary ?
        (hovered ? Vec4(0.20f, 0.65f, 0.95f, 0.95f) : Vec4(0.12f, 0.45f, 0.75f, 0.90f)) :
        (hovered ? Vec4(0.15f, 0.22f, 0.32f, 0.90f) : Vec4(0.08f, 0.12f, 0.18f, 0.85f));

    Vec4 borderColor = primary ?
        (hovered ? Vec4(0.60f, 0.95f, 1.0f, 1.0f) : Vec4(0.35f, 0.75f, 0.95f, 0.85f)) :
        (hovered ? Vec4(0.40f, 0.60f, 0.85f, 0.85f) : Vec4(0.20f, 0.32f, 0.45f, 0.65f));

    ui->drawRect(x, y, w, h, bgColor);
    ui->drawRectOutline(x, y, w, h, hovered ? 1.5f : 1.0f, borderColor);

    Vec4 textColor = primary ?
        (hovered ? Vec4(1.0f, 1.0f, 1.0f, 1.0f) : Vec4(0.92f, 0.96f, 1.0f, 0.95f)) :
        (hovered ? Vec4(1.0f, 0.92f, 0.45f, 1.0f) : Vec4(0.80f, 0.88f, 0.95f, 0.90f));

    ui->drawTextCentered(text, x + w * 0.5f, y + (h - 14.0f) * 0.5f, 1.35f, textColor);

    return hovered && clicked;
}

bool LoginUI::drawClassCard(UIRenderer* ui, float x, float y, float w, float h,
                            const std::string& className, const std::string& roleDesc,
                            const std::string& perks, bool isSelected,
                            int mouseX, int mouseY, bool clicked) {
    bool hovered = (mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h);

    Vec4 bg = isSelected ? Vec4(0.14f, 0.32f, 0.50f, 0.95f) :
              (hovered ? Vec4(0.09f, 0.18f, 0.28f, 0.88f) : Vec4(0.05f, 0.09f, 0.15f, 0.82f));

    Vec4 border = isSelected ? Vec4(1.0f, 0.85f, 0.30f, 1.0f) :
                  (hovered ? Vec4(0.40f, 0.70f, 0.95f, 0.85f) : Vec4(0.20f, 0.30f, 0.42f, 0.60f));

    ui->drawRect(x, y, w, h, bg);
    ui->drawRectOutline(x, y, w, h, isSelected ? 2.0f : 1.0f, border);

    Vec4 titleCol = isSelected ? Vec4(1.0f, 0.92f, 0.40f, 1.0f) : Vec4(0.95f, 0.95f, 0.95f, 1.0f);
    ui->drawTextCentered(className, x + w * 0.5f, y + 8.0f, 1.45f, titleCol);
    ui->drawTextCentered(roleDesc, x + w * 0.5f, y + 26.0f, 1.10f, {0.70f, 0.85f, 0.95f, 0.85f});
    ui->drawTextCentered(perks, x + w * 0.5f, y + 42.0f, 1.05f, isSelected ? Vec4(0.4f, 1.0f, 0.6f, 1.0f) : Vec4(0.55f, 0.75f, 0.85f, 0.75f));

    return hovered && clicked;
}

void LoginUI::render(UIRenderer* ui, int screenWidth, int screenHeight,
                     int mouseX, int mouseY, bool mouseLeftDown, bool mouseLeftClicked,
                     float totalTime) {
    if (!open) return;

    float sw = static_cast<float>(screenWidth);
    float sh = static_cast<float>(screenHeight);
    float cx = sw * 0.5f;
    float cy = sh * 0.5f;

    // 1. Semi-transparent backdrop blur vignette
    ui->drawRect(0, 0, sw, sh, {0.02f, 0.03f, 0.06f, 0.75f});

    // 2. Main Login Modal Dimensions
    float panelW = 620.0f;
    float panelH = 500.0f;
    float panelX = cx - panelW * 0.5f;
    float panelY = cy - panelH * 0.5f;

    // Panel background & double border
    ui->drawRect(panelX, panelY, panelW, panelH, {0.04f, 0.07f, 0.12f, 0.96f});
    ui->drawRectOutline(panelX, panelY, panelW, panelH, 2.0f, {0.25f, 0.65f, 0.95f, 0.90f});
    ui->drawRectOutline(panelX + 4.0f, panelY + 4.0f, panelW - 8.0f, panelH - 8.0f, 1.0f, {0.15f, 0.35f, 0.55f, 0.50f});

    // Golden Corner Accents
    float cSize = 6.0f;
    Vec4 goldAccent(1.0f, 0.85f, 0.35f, 1.0f);
    ui->drawRect(panelX, panelY, cSize, cSize, goldAccent);
    ui->drawRect(panelX + panelW - cSize, panelY, cSize, cSize, goldAccent);
    ui->drawRect(panelX, panelY + panelH - cSize, cSize, cSize, goldAccent);
    ui->drawRect(panelX + panelW - cSize, panelY + panelH - cSize, cSize, cSize, goldAccent);

    // 3. Header Title & Subtitle
    float headerY = panelY + 16.0f;
    ui->drawTextCentered("CHRONICLES OF AETHERIA", cx, headerY, 2.4f, {1.0f, 0.88f, 0.35f, 1.0f});
    ui->drawTextCentered("HESAP GIRISI & KARAKTER PROFILI", cx, headerY + 28.0f, 1.35f, {0.45f, 0.85f, 1.0f, 0.95f});

    // Horizontal divider
    float divY = headerY + 48.0f;
    ui->drawLine(panelX + 24.0f, divY, panelX + panelW - 24.0f, divY, 1.0f, {0.20f, 0.40f, 0.65f, 0.60f});

    // =========================================================================
    // SECTION 1: OYUNCU ISMI / NICKNAME INPUT
    // =========================================================================
    float sec1Y = divY + 14.0f;
    ui->drawText("OYUNCU ISMI (KULLANICI ADI):", panelX + 30.0f, sec1Y, 1.35f, {0.85f, 0.92f, 1.0f, 1.0f});

    float inputX = panelX + 30.0f;
    float inputY = sec1Y + 20.0f;
    float inputW = 380.0f;
    float inputH = 38.0f;

    bool inputHovered = (mouseX >= inputX && mouseX <= inputX + inputW && mouseY >= inputY && mouseY <= inputY + inputH);
    if (inputHovered && mouseLeftClicked) {
        isInputFocused = true;
    } else if (mouseLeftClicked && !inputHovered && mouseX >= panelX && mouseX <= panelX + panelW && mouseY >= panelY && mouseY <= panelY + panelH) {
        // clicked elsewhere on panel
    }

    Vec4 inputBg = isInputFocused ? Vec4(0.08f, 0.14f, 0.22f, 0.95f) : Vec4(0.05f, 0.09f, 0.15f, 0.85f);
    Vec4 inputBorder = isInputFocused ? Vec4(0.35f, 0.85f, 1.0f, 1.0f) : Vec4(0.20f, 0.35f, 0.50f, 0.70f);

    ui->drawRect(inputX, inputY, inputW, inputH, inputBg);
    ui->drawRectOutline(inputX, inputY, inputW, inputH, isInputFocused ? 2.0f : 1.0f, inputBorder);

    // Render current typed name + blinking cursor
    bool showCursor = isInputFocused && (std::sin(totalTime * 6.0f) > 0.0f);
    std::string displayText = username + (showCursor ? "|" : "");
    if (username.empty() && !isInputFocused) {
        ui->drawText("Isminizi buraya yazin...", inputX + 12.0f, inputY + 10.0f, 1.30f, {0.45f, 0.55f, 0.65f, 0.7f});
    } else {
        ui->drawText(displayText, inputX + 12.0f, inputY + 10.0f, 1.45f, {1.0f, 1.0f, 1.0f, 1.0f});
    }

    // Random Name Button
    float randBtnX = inputX + inputW + 12.0f;
    float randBtnW = panelW - 60.0f - inputW - 12.0f;
    if (drawButton(ui, randBtnX, inputY, randBtnW, inputH, "RASTGELE", false, mouseX, mouseY, mouseLeftClicked)) {
        generateRandomName();
    }

    ui->drawText("Diger oyuncularin ve dunyanin gorecegi takma ad (3-16 karakter)",
                 inputX, inputY + inputH + 5.0f, 1.10f, {0.60f, 0.75f, 0.88f, 0.80f});

    // =========================================================================
    // SECTION 2: KARAKTER SINIFI / ROL SECIMI (CHARACTER CLASS)
    // =========================================================================
    float sec2Y = inputY + inputH + 24.0f;
    ui->drawText("KARAKTER SINIFI & ROL:", panelX + 30.0f, sec2Y, 1.35f, {0.85f, 0.92f, 1.0f, 1.0f});

    float cardW = (panelW - 60.0f - 3 * 10.0f) / 4.0f;
    float cardH = 64.0f;
    float cardY = sec2Y + 20.0f;

    if (drawClassCard(ui, panelX + 30.0f + 0 * (cardW + 10.0f), cardY, cardW, cardH,
                      "SAVASCI", "Warrior", "+20 HP  |  Kilic", selectedClass == "Savasci",
                      mouseX, mouseY, mouseLeftClicked)) {
        selectedClass = "Savasci";
    }

    if (drawClassCard(ui, panelX + 30.0f + 1 * (cardW + 10.0f), cardY, cardW, cardH,
                      "BUYUCU", "Mage", "+50 Mana  |  Buyu", selectedClass == "Buyucu",
                      mouseX, mouseY, mouseLeftClicked)) {
        selectedClass = "Buyucu";
    }

    if (drawClassCard(ui, panelX + 30.0f + 2 * (cardW + 10.0f), cardY, cardW, cardH,
                      "OKCU", "Ranger", "+15% Hiz  |  Yay", selectedClass == "Okcu",
                      mouseX, mouseY, mouseLeftClicked)) {
        selectedClass = "Okcu";
    }

    if (drawClassCard(ui, panelX + 30.0f + 3 * (cardW + 10.0f), cardY, cardW, cardH,
                      "PALADIN", "Paladin", "+10 Zirh  |  Sifa", selectedClass == "Paladin",
                      mouseX, mouseY, mouseLeftClicked)) {
        selectedClass = "Paladin";
    }

    // =========================================================================
    // SECTION 3: KAYITLI HESAPLAR / HIZLI PROFIL GECISI
    // =========================================================================
    float sec3Y = cardY + cardH + 16.0f;
    ui->drawText("KAYITLI PROFILLER (HIZLI SECIM):", panelX + 30.0f, sec3Y, 1.25f, {0.80f, 0.88f, 0.95f, 0.9f});

    float chipW = 132.0f;
    float chipH = 28.0f;
    float chipGap = 10.0f;
    float chipY = sec3Y + 18.0f;

    for (size_t i = 0; i < savedProfiles.size() && i < 4; ++i) {
        float chipX = panelX + 30.0f + i * (chipW + chipGap);
        bool isCurrent = (username == savedProfiles[i].name);
        std::string label = savedProfiles[i].name;

        if (drawButton(ui, chipX, chipY, chipW, chipH, label, isCurrent, mouseX, mouseY, mouseLeftClicked)) {
            username = savedProfiles[i].name;
            selectedClass = savedProfiles[i].characterClass;
            statusMessage = "Profil secildi: " + username + " (" + selectedClass + ")";
            statusColor = {0.35f, 0.95f, 0.45f, 1.0f};
        }
    }

    // =========================================================================
    // SECTION 4: BENI HATIRLA TOGGLE & STATUS MESSAGE
    // =========================================================================
    float sec4Y = chipY + chipH + 16.0f;
    float chkX = panelX + 30.0f;
    float chkW = 20.0f;
    float chkH = 20.0f;

    bool chkHovered = (mouseX >= chkX && mouseX <= chkX + 220.0f && mouseY >= sec4Y && mouseY <= sec4Y + chkH);
    if (chkHovered && mouseLeftClicked) {
        rememberMe = !rememberMe;
    }

    ui->drawRect(chkX, sec4Y, chkW, chkH, {0.08f, 0.14f, 0.22f, 0.95f});
    ui->drawRectOutline(chkX, sec4Y, chkW, chkH, 1.0f, {0.30f, 0.60f, 0.85f, 0.9f});
    if (rememberMe) {
        ui->drawRect(chkX + 4.0f, sec4Y + 4.0f, chkW - 8.0f, chkH - 8.0f, {0.25f, 0.90f, 0.40f, 1.0f});
    }
    ui->drawText("Beni Hatirla (Otomatik Giris)", chkX + 28.0f, sec4Y + 3.0f, 1.20f, {0.85f, 0.90f, 0.98f, 0.95f});

    // Status / Notification Line
    float statusY = sec4Y + 30.0f;
    ui->drawTextCentered(statusMessage, cx, statusY, 1.25f, statusColor);

    // =========================================================================
    // SECTION 5: ACTION BUTTONS (LOGIN, GUEST, CANCEL)
    // =========================================================================
    float actY = statusY + 24.0f;
    float bLoginW = 240.0f;
    float bGuestW = 160.0f;
    float bCancelW = 120.0f;
    float bH = 40.0f;

    // Login & Play (Primary)
    if (drawButton(ui, panelX + 30.0f, actY, bLoginW, bH, "> GIRIS YAP VE OYNA <", true, mouseX, mouseY, mouseLeftClicked)) {
        tryLogin();
    }

    // Guest login
    if (drawButton(ui, panelX + 30.0f + bLoginW + 12.0f, actY, bGuestW, bH, "MISAFIR GIRISI", false, mouseX, mouseY, mouseLeftClicked)) {
        username = "Misafir_" + std::to_string(1000 + (std::rand() % 9000));
        tryLogin();
    }

    // Cancel / Close
    if (drawButton(ui, panelX + 30.0f + bLoginW + 12.0f + bGuestW + 12.0f, actY, bCancelW, bH, "GERI / IPTAL", false, mouseX, mouseY, mouseLeftClicked)) {
        if (onCancel) {
            onCancel();
        } else {
            open = false;
        }
    }
}

} // namespace Aetheria
