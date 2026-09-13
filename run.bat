@echo off
if exist bin\AetheriaRPG.exe (
    cd /d "%~dp0"
    start "" bin\AetheriaRPG.exe
) else (
    echo bin\AetheriaRPG.exe not found! Running build first...
    call build.bat
    if exist bin\AetheriaRPG.exe (
        start "" bin\AetheriaRPG.exe
    )
)
