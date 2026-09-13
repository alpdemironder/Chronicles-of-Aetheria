# Chronicles of Aetheria

[![Release](https://img.shields.io/github/v/release/alpdemironder/Chronicles-of-Aetheria?color=00d2ff&label=Release)](https://github.com/alpdemironder/Chronicles-of-Aetheria/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![OpenGL 3.3](https://img.shields.io/badge/OpenGL-3.3%20Core-green.svg)](https://www.opengl.org/)

An advanced **C++17 OpenGL 3.3 Core Profile Voxel RPG Sandbox** that blends the infinite procedural exploration of Minecraft with the tactical companion taming and modular freeform base building of Palworld.

---

## 📸 Gameplay Gallery

![Procedural World Vista with Iris Shaders](docs/screenshots/world_vista.jpg)
*Lush procedural biomes featuring cherry blossom groves, reflective river systems, and Iris shader atmospheric lighting.*

![Palworld-Style Creature Taming](docs/screenshots/creature_taming.jpg)
*Dynamic projectile capture mechanics with ballistic arc physics, real-time catch probability reticle, and mythical creature encounters.*

![Modular Base Building & Companion Camp](docs/screenshots/base_building.jpg)
*Modular multi-tier architectural construction, crafting workshops, and loyal tamed companion wolves guarding the base at golden hour.*

---

## 🌟 Key Features

### 1. 🌍 Procedural Voxel Engine & Worldgen
- **365+ Block IDs** with distinct material properties, step heights, and mining speeds.
- **35 Unique Biomes**:
  - *Temperate*: Lush Meadows, Autumnal Woodlands, Birch Glades.
  - *Arid*: Dune Sea, Red Rock Mesas, Oasis.
  - *Cold*: Frosted Taiga, Glacial Spikes, Alpine Peaks.
  - *Tropical*: Dense Jungle, Bamboo Sanctuary, Mangrove Swamps.
  - *Nocturnal & Volcanic*: Magma Calderas, Obsidian Ridges.
  - *Celestial*: Void Peaks, Ethereal Spires.
- **3D Multi-Octave Caverns**: Subterranean cave systems, winding tunnels, aquifer chambers, and molten magma pools with fluid flow physics.

### 2. 🔮 Creature Ecosystem & Palworld-Style Taming System
- **Farm Animals First (Docile Wildlife & Livestock)**:
  - **Dairy Cow**: White with black patched coat, curved ivory horns, floppy ears, udder, drops Tanned Leather.
  - **Highland Sheep**: Fluffy wool fleece coat, distinct dark face and ears, drops Wool.
  - **Farm Pig**: Rounded pink body, 4 trotters, protruding 3D snout with nostrils, curly tail, drops Savory Feast Meat.
  - **Farm Chicken**: Plump feathered body, flapping wings, yellow beak, bright red comb & wattle, drops Feathers.
  - **Wild Steed (Horse)**: Muscular equine torso, arched neck, dark crest mane, tall hooved legs, flowing tail, rapid 6.5 m/s stride.
- **Monsters (Hostile Foes & Night Terrors)**:
  - **Draugr Zombie**: Decaying necrotic green skin, forward-extended reaching arms, dark sunken eyes, drops Iron Ingots.
  - **Skeleton Archer**: Ivory bone structure, exposed ribcage, hollow sockets, wields a recurve bow, drops Bone Meal.
  - **Cave Spider**: Dedicated 8-legged articulated arachnid model with ripple-wave crawling gait, 6 glowing ruby eyes, venomous pedipalp fangs, drops Silk Rope.
  - **Crypt Ghoul**: Gaunt ashen-purple predatory fiend, hunched feral posture, elongated arms with razor black talons, protruding spinal bone ridges, glowing amber eyes.
  - **Goblin Raider**: Short green skirmisher, large pointed bat ears, upward underbite tusks, wields a jagged iron dagger, drops Gold Ingots.
- **Physical 3D Capture Sphere Projectiles**:
  - Pal Sphere (1.0x capture power)
  - Mega Sphere (2.0x capture power)
  - Giga Sphere (3.5x capture power)
  - Arcing throw physics with parabolic gravity and drag. Missed spheres land safely and can be recovered by walking over them.
- **3-Stage Wobble Capture Sequence**:
  - Capturing weakened wild creatures pulls them into a physical 3D sphere.
  - 3 successive tension-filled wobble checks ($t = 0.8\text{s}$, $1.6\text{s}$, $2.4\text{s}$) with audio cues and rolling animations.
  - Capture probability dynamically scales with remaining health ($1.0 - 0.72 \times \text{HP}\%$).
- **Dynamic Capture Reticle**: Aiming at wild creatures displays real-time calculated catch chance (`[ CATCH CHANCE: XX% ]`) with color-coded feedback.
- **Companion Stance Management (`[V]` Key)**:
  - `[FOLLOW & DEFEND]`: Follows master, teleports if stranded, and attacks hostile enemies.
  - `[STAY / GUARD]`: Holds position and defends a 4-block perimeter.
  - `[WORK AT BASE]`: Patrols and works inside player base camp boundaries.
- **Companion Progression**: Earns XP from monster defeats, levels up, increases Max HP and Attack Damage, and regenerates health out of combat.
- **Companion HUD Deck**: Displays companion level, HP bar, active stance, and overhead crown badges.

### 3. 🏰 Palworld Freeform Modular Building & Furniture Catalog
- **128 Modular Pieces with Authentic Textures**:
  - Expanded catalog of 128 architectural and furniture pieces across all **16 Tree Species** (Oak, Birch, Spruce, Pine, Jungle, Acacia, Dark Oak, Mangrove, Cherry, Maple, Willow, Fungal, Petrified, Bamboo, Palm, Astral):
    - **Stairs (Merdiven)**: 2-step stepped voxel geometry matching each wood's grain and color.
    - **Trapdoors (Tuzak Kapısı)**: Horizontal slatted hatch plate with reinforced cross battens.
    - **Slabs (Basamak)**: Half-height bottom floor/walkway slabs.
    - **Side Slabs (Yan Basamak)**: Half-width vertical partition slabs for windows, pillars, and arches.
    - **Chairs (Sandalye)**: 4-legged dining/hall chairs with seat cushions and backrest posts & splats.
    - **Benches (Bank)**: 6-legged wide tavern/garden benches with full backrest panels and side armrests.
    - **Tables (Masa)**: Sturdy 4-corner legged tables with under-table support aprons and polished tabletop slabs.
  - Core base camp structures: Palbox Camp Core, Primitive & Sphere Workbenches, Smelting Furnace, Straw Creature Beds, Feed Boxes, and Storage Chests.
- **Interactive Hologram Ghost Preview & 3D Rotation**:
  - Real-time holographic ghost preview showing full composite 3D models (chairs, benches, tables, stairs, etc.) in translucent cyan/gold.
  - 90° Cardinal piece rotation (`[R]`) baked directly into placed structures.
  - 0.5-meter fine grid snapping for interior furniture layout.
  - Dismantle mode (`[X]`) with 100% material recycling refunded to player inventory.
- **Modern Build Menu Navigation (`[B]`)**:
  - Smooth mouse wheel scrolling and category tab clicking.
  - Quick wood species browsing (`[Left/Right]` or `[A/D]` jump by 4 items, PageUp/PageDown).
  - Clean wood species badges (`[Oak]`, `[Cherry]`, `[Astral]`) and real-time piece/page counters.

### 4. ⚔️ Minecraft 1.9+ PvP Combat Mechanics
- **Attack Speed Cooldowns & Recharge Meter**: Weapon-specific recovery speeds (Iron Broadsword, Battleaxes, Daggers) with visual HUD attack charge indicator beneath crosshair.
- **Quadratic Scaling**: Quick click spamming deals minimal damage; charged strikes deliver full damage and heavy knockback.
- **Critical Strikes**: Striking opponents during downward fall grants $+50\%$ critical bonus damage with crisp particle and audio feedback.

### 5. ☀️ 12-Minute Day/Night Cycle & Immersive Iris Shaders
- **Active Complementary Reimagined Shaders by Default**: Dual G-Buffer deferred pipeline running `Complementary_Reimagined` out of the box with cinematic post-processing.
- **24-Step Volumetric God Rays**: Dither-jittered crepuscular light shafts streaming through canopies, mountains, and architectural structures with zero banding.
- **Depth-Aware Screen-Space Ambient Occlusion (SSAO)**: 12-tap randomized golden spiral kernel providing rich contact shadows in corners and eaves.
- **Water Surface Screen-Space Reflections (SSR) & Caustics**: Dynamic normal perturbation with animated water ripples and Fresnel sky reflections.
- **Atmospheric Celestial Lighting**: Dynamic sunlight color modulation (warm amber sunrise/sunset vs crisp midday vs cool nocturnal moonlight) and Mie solar forward-scattering horizon glow.
- **Subsurface Foliage Scattering**: Backlit tree leaves glow with radiant green chlorophyll translucency when viewing canopies against the sun.
- **Cinematic ACES Filmic Tonemapping**: True highlight rolloff and subtle optical lens chromatic aberration.
- **Iris In-Game Shader Menu (`[O]`)**: Live hot-swapping between shaderpacks and toggleable shader features.

### 6. 🎨 Authentic Minecraft Textures & Sub-Texel Anti-Aliasing
- **Modern Minecraft (1.17+ Jappa) Signature Ore Patterns**: Tailored pixel-art shapes for Coal (chunky angular spots), Copper (teardrop nuggets with oxidized turquoise patina), Iron (iconic diagonal stepped streaks), Gold (scattered sparkling nuggets), Redstone (dense ruby crystal cluster), Lapis Lazuli (jagged ultramarine vein with gold pyrite flecks), Diamond (brilliant cyan crystal clusters with white sparkles), and Emerald (hexagonal cut gem).
- **Stone Bricks Suite (IDs 241 to 244)**: Classic 2x2 running bond ashlar stone bricks with upper/left highlight bevels and lower/right shadow seams, plus Mossy, Cracked, and Chiseled variants.
- **Geological & Dimensional Blocks**: Conchoidal fractured Obsidian and glowing Crying Obsidian with weeping magenta tears, craggy porous Netherrack, and cratered End Stone.
- **Luminaries & Utilities**: Clustered incandescent Glowstone, aquamarine framed Sea Lanterns, Nether Fortress Bricks, and incandescent Magma tiles.
- **Hardware Mipmapping & 16x Anisotropic Filtering**: Generates full trilinear mipmap chains on `GL_TEXTURE_2D_ARRAY` (`GL_LINEAR_MIPMAP_LINEAR`) with up to 16x anisotropic filtering, completely eliminating distant pixel swimming and moiré patterns.
- **Analytic Sub-Texel Anti-Aliasing (`gl_voxel.frag`)**: Custom GPU shader algorithm computes continuous screen-space UV derivatives (`fwidth`, `textureGrad`) to keep voxel pixel art crisp and razor-sharp up close while anti-aliasing texel boundaries smoothly over 1 screen pixel.

---

## 🚀 Building & Running

### Requirements
- **OS**: Windows 10/11 (64-bit)
- **Compiler**: GCC 9+ (MinGW-w64) or MSVC with C++17 support
- **Graphics**: OpenGL 3.3 Core Profile compatible GPU

### Compile from Source
```cmd
build.bat
```
The executable will be built directly to `bin\AetheriaRPG.exe`.

### Controls
| Key | Action |
|-----|--------|
| **W, A, S, D** | Move / Strafe |
| **Space** | Jump / Swim upward |
| **Left Shift** | Sprint |
| **Left Ctrl** | Sneak |
| **R** | Dash / Rotate building piece |
| **C** | Telescopic Zoom (3.1x) |
| **Left Click** | Attack / Mine Voxel |
| **Right Click** | Throw Pal Sphere / Place Block / Use Consumable |
| **E / Tab / I** | Open Inventory & Crafting Table |
| **V** | Cycle Companion Stance (`Follow` -> `Stay` -> `Work`) |
| **F** | Hammer Blueprint Scaffold |
| **B** | Palworld Building Menu |
| **T / Enter** | Open In-Game Co-op Chat |
| **X** | Dismantle Mode |
| **O** | Iris Shaderpack Menu |
| **Esc** | Pause / Settings Menu |

---

## 🌐 Co-op Multiplayer & Worlds Hub (Çok Oyunculu)

Chronicles of Aetheria features a full-featured tabbed **Multiplayer & Worlds Hub** (`> MULTIPLAYER (ÇOK OYUNCULU) <`):
- **📑 Tab 1: Kendi Dünyalarım (My Worlds)**:
  - Select from your local procedural worlds with details on World Name, Seed, Game Mode, and Port.
  - **Host & Play (`DUNYAYI BASLAT`)**: Spin up a local server and host your world for friends across LAN or VPN (Hamachi/Tailscale/Radmin) with one click!
  - **Play Solo (`TEK BASINA OYNA`)**: Instantly enter and explore the selected world offline.
  - **Create World (`+ YENI DUNYA`)**: Procedurally generate fresh worlds with custom World Name, Seed generator (including Randomizer), and Port assignment.
- **🌐 Tab 2: Diğer Dünyalar (Other Worlds & Servers)**:
  - Browse saved remote and community servers with real-time ping latency readouts (ms) and online beacons.
  - **Direct Connect (`DOGRUDAN BAGLANTI`)**: Join any LAN or remote world directly by typing the target `IP:Port`.
  - **Add Server (`+ SUNUCU EKLE`)**: Save your friends' servers to your persistent multiplayer server list.
- **👤 Tab 3: Oyuncu Profili (Player Profile)**:
  - Customize your multiplayer Nickname/Callsign and view connection diagnostics.
- **Dedicated Headless Server**: Run `run_server.bat` (or `bin\AetheriaServer.exe --port 25565 --seed 133742`) to host a high-performance 24/7 realm with console commands (`/list`, `/say`, `/kick`, `/stop`).
- **Synchronized Gameplay**:
  - Real-time block placement and destruction synchronization with delta world history.
  - 3D character avatars, held weapons, and floating nametags with dynamic health bars.
  - In-game chat system (`[T]` / `[Enter]`) with channel colors and automatic message fade-out.
  - For full hosting and network setup instructions, see [docs/MULTIPLAYER_GUIDE.md](docs/MULTIPLAYER_GUIDE.md).

---

## 📜 License
Distributed under the MIT License. See `LICENSE` for details.
