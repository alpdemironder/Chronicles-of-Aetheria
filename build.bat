@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo   BUILDING CHRONICLES OF AETHERIA (OpenGL RPG Sandbox)
echo ========================================================

set PATH=C:\Users\alpde\.gemini\antigravity\scratch\tools\w64devkit\bin;%PATH%
set GXX=g++.exe

if not exist bin mkdir bin

echo Compiling C++ Source Files with Modern OpenGL and Winsock2...
set SRCS=src\main.cpp ^
src\core\Window.cpp ^
src\core\Audio.cpp ^
src\core\Camera.cpp ^
src\core\Settings.cpp ^
src\render\GLHeaders.cpp ^
src\render\GLContext.cpp ^
src\render\GLBuffer.cpp ^
src\render\GLPipeline.cpp ^
src\render\TextureAtlas.cpp ^
src\world\BlockRegistry.cpp ^
src\world\BiomeRegistry.cpp ^
src\world\Chunk.cpp ^
src\world\World.cpp ^
src\world\WorldGenerator.cpp ^
src\world\StructureGenerator.cpp ^
src\building\StructurePiece.cpp ^
src\building\BuildingManager.cpp ^
src\entities\Entity.cpp ^
src\entities\Player.cpp ^
src\entities\RemotePlayer.cpp ^
src\entities\CreatureRegistry.cpp ^
src\entities\Creature.cpp ^
src\entities\CaptureSphere.cpp ^
src\entities\ItemEntity.cpp ^
src\entities\MobSpawner.cpp ^
src\inventory\ItemRegistry.cpp ^
src\inventory\Inventory.cpp ^
src\inventory\CraftingRegistry.cpp ^
src\inventory\FurnaceManager.cpp ^
src\network\NetworkSocket.cpp ^
src\network\Server.cpp ^
src\network\Client.cpp ^
src\render\IrisShaderManager.cpp ^
src\ui\UIRenderer.cpp ^
src\ui\ItemIconRenderer.cpp ^
src\ui\HUD.cpp ^
src\ui\InventoryUI.cpp ^
src\ui\BuildMenuUI.cpp ^
src\ui\BlockCatalogUI.cpp ^
src\ui\BestiaryUI.cpp ^
src\ui\BiomeCodexUI.cpp ^
src\ui\SettingsUI.cpp ^
src\ui\IrisShaderUI.cpp ^
src\ui\UpdateCalendarUI.cpp ^
src\ui\MainMenuUI.cpp ^
src\ui\MultiplayerUI.cpp ^
src\ui\LoginUI.cpp ^
src\ui\SkillTreeUI.cpp ^
src\ui\ChatUI.cpp

"%GXX%" -std=c++17 -O2 -I src %SRCS% -o bin\AetheriaRPG.exe -lopengl32 -lgdi32 -luser32 -lwinmm -lshell32 -lws2_32

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Game Client compilation failed!
    exit /b %ERRORLEVEL%
)

echo.
echo Compiling Chronicles of Aetheria Dedicated Server (Headless Console)...
set SERVER_SRCS=src\server_main.cpp src\network\NetworkSocket.cpp src\network\Server.cpp
"%GXX%" -std=c++17 -O2 -I src %SERVER_SRCS% -o bin\AetheriaServer.exe -lws2_32

if %ERRORLEVEL% equ 0 (
    echo ========================================================
    echo   BUILD SUCCESSFUL!
    echo   Client: bin\AetheriaRPG.exe
    echo   Server: bin\AetheriaServer.exe
    echo ========================================================
) else (
    echo ========================================================
    echo   DEDICATED SERVER BUILD FAILED with error code %ERRORLEVEL%
    echo ========================================================
)
