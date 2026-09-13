#pragma once

#include "UIRenderer.hpp"
#include "../network/Client.hpp"
#include "../network/Server.hpp"
#include <string>
#include <vector>
#include <functional>

namespace Aetheria {

enum class MultiplayerTab {
    MyWorlds,      // Kendi Dunyalarim (Host, Local, Create World)
    OtherWorlds,   // Diger Dunyalar (Servers, LAN, Direct Connect)
    PlayerProfile  // Oyuncu Profili
};

struct LocalWorld {
    std::string name;
    uint32_t seed = 1337;
    uint16_t port = 25565;
    std::string gameMode = "Survival";
    std::string description = "Yerel Dunya";
};

struct RemoteServer {
    std::string name;
    std::string address;
    int pingMs = 15;
    bool isOnline = true;
    std::string motd = "Aetheria RPG Co-op Sunucusu";
};

class MultiplayerUI {
public:
    MultiplayerUI(Net::NetworkClient* client, Net::AetheriaServer* localServer);

    void render(UIRenderer* ui, int screenWidth, int screenHeight,
                int mouseX, int mouseY, bool mouseLeftClicked,
                float totalTime);

    void onCharInput(char c);
    void onKeyDown(int key);

    bool isOpen() const { return open; }
    void setOpen(bool o) { open = o; }

    void setOnStartGame(std::function<void()> cb) { onStartGame = cb; }
    void setOnStartGameConfig(std::function<void(uint32_t seed, bool isHost, uint16_t port, const std::string& connectIp)> cb) {
        onStartGameConfig = cb;
    }
    void setOnBackToMenu(std::function<void()> cb) { onBackToMenu = cb; }

    const std::string& getPlayerName() const { return playerName; }
    void setPlayerName(const std::string& n) { playerName = n; }
    const std::string& getServerAddress() const { return serverAddress; }

    MultiplayerTab getActiveTab() const { return activeTab; }
    void setActiveTab(MultiplayerTab tab) { activeTab = tab; }

private:
    Net::NetworkClient* client = nullptr;
    Net::AetheriaServer* localServer = nullptr;

    bool open = false;
    MultiplayerTab activeTab = MultiplayerTab::MyWorlds;

    // Player Profile
    std::string playerName = "Aetherian_Hero";
    std::string statusText = "Baglantiya Hazir";

    // Tab 1: Kendi Dunyalarim
    std::vector<LocalWorld> myWorlds;
    int selectedWorldIndex = 0;
    bool isCreatingWorld = false;
    std::string newWorldName = "Yeni Krallik";
    std::string newWorldSeed = "1337";
    std::string newWorldPort = "25565";

    // Tab 2: Diger Dunyalar
    std::vector<RemoteServer> otherServers;
    int selectedServerIndex = 0;
    std::string serverAddress = "127.0.0.1:25565";
    bool isAddingServer = false;
    std::string newServerName = "Yeni Sunucu";
    std::string newServerAddress = "192.168.1.100:25565";

    enum class ActiveField {
        None,
        PlayerName,
        ServerAddress,
        NewWorldName,
        NewWorldSeed,
        NewWorldPort,
        NewServerName,
        NewServerAddress
    };
    ActiveField activeField = ActiveField::None;

    std::function<void()> onStartGame = nullptr;
    std::function<void(uint32_t seed, bool isHost, uint16_t port, const std::string& connectIp)> onStartGameConfig = nullptr;
    std::function<void()> onBackToMenu = nullptr;

    bool drawButton(UIRenderer* ui, float x, float y, float w, float h,
                    const std::string& text, bool primary,
                    int mouseX, int mouseY, bool clicked);

    bool drawTabButton(UIRenderer* ui, float x, float y, float w, float h,
                       const std::string& text, bool isActive,
                       int mouseX, int mouseY, bool clicked);

    void drawInputField(UIRenderer* ui, float x, float y, float w, float h,
                        const std::string& label, const std::string& value,
                        bool isFocused, int mouseX, int mouseY, bool clicked,
                        ActiveField fieldType, float totalTime);
};

} // namespace Aetheria
