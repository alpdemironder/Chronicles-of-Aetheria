#pragma once

#include "UIRenderer.hpp"
#include <string>
#include <vector>
#include <functional>

namespace Aetheria {

struct AccountProfile {
    std::string name;
    std::string characterClass;
    std::string race = "Insan";
    int level = 1;
};

class LoginUI {
public:
    LoginUI();

    void render(UIRenderer* ui, int screenWidth, int screenHeight,
                int mouseX, int mouseY, bool mouseLeftDown, bool mouseLeftClicked,
                float totalTime);

    void onCharInput(char c);
    void onKeyDown(int key);

    bool isOpen() const { return open; }
    void setOpen(bool o) { open = o; }

    const std::string& getUsername() const { return username; }
    void setUsername(const std::string& name) { username = name; }

    const std::string& getCharacterClass() const { return selectedClass; }
    void setCharacterClass(const std::string& cls) { selectedClass = cls; }

    const std::string& getRace() const { return selectedRace; }
    void setRace(const std::string& r) { selectedRace = r; }

    bool getRememberMe() const { return rememberMe; }
    void setRememberMe(bool rem) { rememberMe = rem; }

    void setOnLoginSuccess(std::function<void(const std::string& user, const std::string& charClass, const std::string& race)> cb) {
        onLoginSuccess = cb;
    }
    void setOnCancel(std::function<void()> cb) { onCancel = cb; }

private:
    bool open = false;
    bool isInputFocused = true;

    std::string username = "Alp";
    std::string selectedClass = "Savasci";
    std::string selectedRace = "Insan";
    bool rememberMe = true;

    std::string statusMessage = "Lutfen oyuncu isminizi girin ve Giris Yap'a basin.";
    Vec4 statusColor{0.75f, 0.88f, 1.0f, 0.9f};
    float statusTimer = 0.0f;

    std::vector<AccountProfile> savedProfiles;

    std::function<void(const std::string&, const std::string&, const std::string&)> onLoginSuccess = nullptr;
    std::function<void()> onCancel = nullptr;

    bool drawButton(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& text, bool primary,
                    int mouseX, int mouseY, bool clicked);

    bool drawClassCard(UIRenderer* ui, float x, float y, float w, float h,
                       const std::string& className, const std::string& roleDesc,
                       const std::string& perks, bool isSelected,
                       int mouseX, int mouseY, bool clicked);

    bool drawRaceCard(UIRenderer* ui, float x, float y, float w, float h,
                      const std::string& raceName, const std::string& roleDesc,
                      const std::string& perks, bool isSelected,
                      int mouseX, int mouseY, bool clicked);

    void generateRandomName();
    void tryLogin();
};

} // namespace Aetheria
