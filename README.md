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

### 2. 🔮 Authentic Minecraft-Style Mob Ecosystem & Companion System
- **Farm Animals & Peaceful Wildlife**:
  - **Cow**: Mottled brown & white hide, square head with pink muzzle, horn cuboids, pink udder underneath, 4 blocky legs. Drops Raw Beef and Leather.
  - **Sheep**: Fluffy white wool fleece coat block, exposed sheep head, slender legs. Drops Wool and Raw Mutton.
  - **Pig**: Iconic square pink cube body, square head, protruding 3D snout cuboid with nostrils, 4 stubby legs. Drops Raw Porkchops.
  - **Chicken**: White feather body, head with yellow beak and red wattle, flapping side wings, yellow legs with feet. Drops Feathers and Raw Chicken.
  - **Horse**: Blocky body, angled neck, head with ears and snout, dark mane, flowing animated tail, 4 hooved legs.
- **Monsters & Hostile Foes**:
  - **Creeper**: Iconic mottled green camouflage body, 4 stubby legs, frowning pixel face. Features authentic fuse hissing, visual 32% body swelling & white flashing animation, and lethal explosive detonation!
  - **Enderman**: Towering 2.9 blocks tall, obsidian void black skin, long slender limbs reaching past knees, glowing purple eye slits. Instantly teleports away with purple particles when attacked!
  - **Zombie**: Classic Steve geometry with iconic cyan tunic, indigo blue trousers, decaying green skin, and arms permanently outstretched horizontally (-90° pitch).
  - **Skeleton**: Off-white bones, hollow eye socket skull, exposed ribcage, thin bone limbs, wielding a 3D blocky wooden bow.
  - **Spider**: Wide flat cephalothorax, bulbous abdomen, 8 glowing ruby red eyes, fangs, and 8 sprawling articulated crawling legs with rippling gait.
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

### 7. 🧬 Hierarchical Skeletal Rigging, Swept Physics & Random World Seeds
- **Hierarchical Articulated Rigging (`appendRiggedBox`)**:
  - Limbs and body parts pivot realistically at joint sockets (hips, shoulders, necks) with zero detachment.
  - Walk cycles adapt smoothly to entity velocity ($v > 0.01\text{m/s}$).
  - Mob features are dynamically articulated: Chicken side wings flap rapidly, Sheep head grazes with idle bobbing, Cow horns and snout nod with the skull, Horse mane and tail sway, Spider's 8 sprawling legs ripple in opposing 4-phase pairs, and Creeper swells by up to 32% during fuse ignition.
- **Continuous Swept Voxel Physics & Zero-Clipping Engine**:
  - Swept axis collision displacement calculates exact boundary clearances on Y, X, and Z axes without tunneling or clipping through terrain blocks.
  - Strict open-interval AABB collision logic removes edge/corner false-positive sticking.
  - Auto-step climbing (0.5m) verifies vertical headroom before ascension to prevent pushing entities into overhead ceilings.
  - Anti-clipping penetration recovery gently pops stuck entities to safe open space if forcibly pushed into terrain.
- **Procedural Random World Seed System**:
  - World generation supports dynamic 32-bit seeds randomly seeded from hardware entropy (`std::random_device` $\oplus$ high-resolution clock).
  - Singleplayer features a one-click random world generation launcher (`> PLAY SINGLEPLAYER (RASTGELE DUNYA) <`).
  - Multiplayer interface includes an instant procedural seed randomizer button.
  - In-game `/seed` and `/worldseed` chat commands display the active seed anytime.
  - Comprehensive Minecraft `[F3]` debug screen displays real-time FPS, sub-block XYZ coordinates, block coordinates, chunk offset, compass facing direction, current biome, and the active procedural seed.

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
| **Space** | Jump / Swim upward / **Double Jump (Havada İkinci Zıplama)** |
| **Left Shift** | Sprint |
| **Left Ctrl** | Sneak |
| **R** | Dash / Rotate building piece |
| **C** | Telescopic Zoom (3.1x) |
| **Left Click** | Attack / Mine Voxel |
| **Right Click** | Open Chest / Throw Pal Sphere / Place Block / Use Consumable |
| **E / Tab / I** | Open Inventory & Crafting Table |
| **K** | **Yetenek & Büyü Ağacı (Skill Tree & SP)** |
| **Q / G** | **Alev Topu Büyüsü (Fireball Spell)** / Shift+Q Drop Item |
| **Z** | **Buz Fırtınası Büyüsü (Frost Nova Spell)** |
| **H** | **Kutsal Şifa Büyüsü (Holy Heal Spell)** |
| **V** | Cycle Companion Stance (`Follow` -> `Stay` -> `Work`) |
| **F** | Hammer Blueprint Scaffold |
| **B** | Palworld Building Menu |
| **F3** | **Minecraft F3 Debug Ekranı (FPS, XYZ Koordinatları, Chunk, Biome, Dünya Seed'i)** |
| **F1** | **Sinematik HUD Aç/Kapat (Toggle Clean HUD)** |
| **F5** | **Üçüncü Şahıs / Birinci Şahıs Kamera (Toggle F5 Camera)** |
| **T / Enter** | **Oyun İçi Sohbet (Chat & `/seed` Komutu)** |
| **X** | Dismantle Mode |
| **O** | Iris Shaderpack Menu |
| **Esc** | Pause / Settings Menu |

---

## 🔮 Yetenek Ağacı & Büyüler (Skill Tree & Active Spells)

Her seviye atlandığında kazanılan **Yetenek Puanları (SP - Skill Points)** ile harcanabilir aktif büyüler ve pasif güçlendirmeler (`[K]` Tuşu):
- **Çift Zıplama (Double Jump)**: Havada `[SPACE]` tuşuna basarak ikinci kez havada zıplama yeteneği (15 Stamina).
- **Alev Topu (Fireball - `[Q]` / `[G]`)**: Hedefe doğru patlayıcı alev topu fırlatır (45 Alan Hasarı + Düşmanları Geri İtme, 25 Mana).
- **Buz Fırtınası (Frost Nova - `[Z]`)**: Çevredeki 8.5 blok içindeki tüm canavarları dondurur ve yavaşlatır (30 Büyü Hasarı, 30 Mana).
- **Kutsal Şifa (Holy Heal - `[H]`)**: Kutsal ışık enerjisiyle anında +40 HP can yeniler (40 Mana).
- **Pasif Yetenekler**:
  - **Çelik Deri (Iron Skin)**: Seviye başına +4 Zırh/Defans kazandırır.
  - **Hızlı İyileşme (Regeneration)**: Her 2 saniyede bir pasif olarak +1.5 Can yeniler.
  - **Mana Akışı (Mana Surge)**: +30 Max Mana ve +40% Mana dolum hızı artışı.
  - **Rüzgar Adımları (Swiftness)**: +15% Koşu ve depar hareket hızı artışı.
  - **Savaş Öfkesi (Berserker)**: Seviye başına +12% Silah ve büyü saldırı hasarı artışı.

---

## 🧝 Karakter Irkları & Özel Pasifler (RPG Playable Races)

Karakter giriş ekranında seçilebilen 5 benzersiz RPG ırkı ve oynanış özellikleri:
1. **İnsan (Human)**:
   - *Dengeli Uyum*: Her seviye atlandığında diğer ırklardan farklı olarak **+2 Yetenek Puanı (SP)** kazanır.
2. **Elf**:
   - *Kadim Büyücü*: **+50% Daha Hızlı Mana Yenilenmesi**, +20 Max Mana ve büyü/ok saldırılarında ekstra güç.
3. **İblis (Demon)**:
   - *Cehennem Ateşi*: **Lav ve Ateş Bağışıklığı (%100 Lava & Fire Immunity)**! Lavda yürüyebilir, yüzebilir ve yanmaz. +8 Karanlık Ateş hasarı.
4. **Vampir (Vampire)**:
   - *Gece Avcısı*: **%20 Can Çalma (Life Steal)**! Düşmanlara ve canavarlara vurulan her darbenin %20'si anında oyuncuya can (HP) olarak geri döner.
5. **Slime**:
   - *Elastik Gövde*: **Düşme Hasarı Almaz (No Fall Damage)**! Yere yüksekten çarpınca elastik zıplama fiziği (Bounce) ve +25% Zıplama yüksekliği.

---

## 📦 Ahşap Sandık & Konteyner Arayüzü (Wooden Storage Chests)

- **3x3 Çalışma Masası Reçetesi**: Herhangi 8 adet ahşap tahta ile ortası boş çerçeve şeklinde dizilerek üretilir (Block ID 337 / 333). Hızlı üretim menüsünde de yer alır.
- **27 Slotlu Konteyner Depolama GUI'si**:
  - Dünyaya yerleştirilen sandığa (veya Palworld `Storage_Chest` yapısına) sağ tıklandığında 27 slotluk depolama arayüzü açılır.
  - **Shift + Sol Tık (Hızlı Transfer)**: Sandık ile çanta arasında eşyaları tek tıkla aktarır.
  - **Sağ Tık**: Eşya destesini ikiye böler veya tek tek yerleştirir.
  - **Hızlı Butonlar**:
    - `[HEPSİNİ AL]`: Sandıktaki tüm eşyaları oyuncunun çantasına aktarır.
    - `[HEPSİNİ KOY]`: Oyuncunun çantasındaki eşyaları sandığa depolar.
    - `[HIZLI YIĞ]`: Sandıkta bulunan eşyaları çantanızdan otomatik olarak birleştirir ve istifler.
- **Kırılınca Eşyaları Düşürme**: Sandık kırıldığında veya kazıldığında içindeki tüm eşyalar fiziksel 3D eşya olarak dünyaya saçılır.

---

## 👤 Account Login & Character Profile (Hesap Girişi)

Chronicles of Aetheria features a dedicated **Account Login & Profile Screen** (`> HESAP GIRISI & PROFIL <`):
- **Custom Player Nickname**: Type and set your custom player name with full keyboard typing, backspace editing, and real-time cursor blinking.
- **Random Name Generator (`RASTGELE`)**: Generates cool adventure and fantasy callsigs with 1 click.
- **RPG Character Class Archetypes**:
  - **Savaşçı (Warrior)**: +20 Max HP bonus, +5 Sword Melee Power.
  - **Büyücü (Mage)**: +50 Max Mana bonus, Arcane Affinity.
  - **Okçu (Ranger)**: +15% Sprint Speed bonus, Rapid Archery.
  - **Paladin (Paladin)**: +10 Base Armor Defense, Holy Resilience.
- **Quick Account Switching**: Saved profile chips to switch instantly between previous profiles.
- **Remember Me (`Beni Hatırla`)**: Automatically stores and logs into your profile on launch.
- **Full In-Game Synchronization**: Your chosen name appears in the HUD status deck, in-game chat messages, multiplayer nametags, and the main menu top badge (`[ HESAP: <Isim> ]`).

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

## ⛏️ Otantik Minecraft Deneyimi & Görsel Yenileme (Minecraft Vanilla Remaster)

Chronicles of Aetheria, Minecraft'ın nostaljik görsel estetiğini ve temel mekaniklerini baştan sona birebir yansıtacak şekilde yenilenmiştir:

### 1. 🎛️ Vanilla Minecraft HUD & Göstergeler
- **9 Yuvalı Taş Hotbar**:
  - Ekranın alt ortasında 9 yuvalı taş grisi (`#8B8B8B` / `#373737` / `#FFFFFF`), içe gömülü yuvalar.
  - **3D Beyaz Çerçeveli Seçili Yuva**: Seçilen yuvanın dışına 2-3 piksel taşan, kabartmalı klasik Minecraft çerçevesi.
  - **Eşya Sayıları**: Sağ alt köşede beyaz piksel fontu ve siyah gölgeli (+1, +1 offset) klasik sayı gösterimi.
  - **Action-Bar Tooltip**: Yuva değiştirildiğinde ekranın ortasında süzülen ve 2.5 saniyede sönen eşya adı yazısı.
- **Piksel Kalpler (Can)**:
  - Hotbarın sol üstünde 10 adet Minecraft piksel kalbi (koyu kırmızı çerçeve, parlak kırmızı dolgu, sol üstte 1x1 beyaz parlama pikseli, yarım kalp ve boş kap desteği).
  - Can %20'nin altına indiğinde kalplerin panikle zıplama/titreme animasyonu ve hasar anında beyaz/sarı parlama efekti.
- **Piksel Tavuk Butları (Açlık & Kondisyon)**:
  - Hotbarın sağ üstünde 10 adet kızarmış tavuk budu (beyaz kemik ucu, leziz kahverengi et, yarım but desteği).
  - Kondisyon düştüğünde titreme animasyonu.
- **Zırh Göğüslükleri & Oksijen Baloncukları**:
  - Kalplerin üstünde kuşanılan zırh defansına göre dolan gümüş göğüslük ikonları.
  - Su altına girildiğinde (`underwater`) tavuk butlarının üzerinde beliren 10 adet mavi oksijen baloncuk göstergesi.
- **Kireç Yeşili XP Barı & Minecraft Seviye Numarası**:
  - Hotbarın tam üstünü kaplayan koyu yeşil çerçeveli ve parlak neon kireç yeşili (`#80FF20`) deneyim barı.
  - Üzerinde Minecraft dikey bölme çentikleri.
  - XP çubuğunun tam ortasında kalın, yeşil renkli ve koyu gölgeli **Minecraft Seviye Numarası** (örn. `12`).
- **15x15 Piksel Klasik Nişangah**:
  - Ortası delik, ters renkli/yarı saydam klasik `+` nişangahı ve altında 1.9+ saldırı dolum şarj göstergesi.

### 2. 🔲 3D Hedeflenen Blok Tel Çerçevesi & Çatlama Aşamaları (0-9)
- Oyuncu 5.5 blok menzilde herhangi bir bloğa baktığında, o bloğu çevreleyen **12 kenarlı ince siyah tel çerçeve (Bounding Box Outline)** çizilir.
- Blok kazılırken bloğun yüzeylerinde aşama aşama çatlayan **Minecraft çatlama animasyonu (Destroy Stages 0-9)** gösterilir.

### 3. 🖐️ Birinci Şahıs Hareketli Steve Kolu & Alet Görünümü (Viewmodel)
- Birinci şahıs kamerasında ekranın sağ alt köşesinde:
  - Boş elde: Steve'in pikselli turkuaz kolluklu ve ten rengi yumruğu.
  - Alet tutarken: Çapraz duran 3D alet modeli (Kılıç, Kazma, Balta, Kürek).
  - Blok tutarken: Elde tutulan küçük 3D izometrik küp blok.
  - Yürürken doğal el sallanma / bobbing hareketi.
  - Vururken veya blok kırarken/koyarken Minecraft'ın karakteristik **aşağı yay çizerek inen el sallama animasyonu**.

### 4. 📊 Minecraft Java F3 Debug Ekranı & F1 HUD Gizleme
- **`[F3]` Tuşu**: Minecraft Java Edition tarzı F3 hata ayıklama panelini açar/kapatır:
  - *Sol Taraf*: Sürüm, FPS ve kare süresi, XYZ kesin koordinatlar, Block koordinatları, Chunk bilgisi, Facing (Kuzey/Güney/Doğu/Batı ve açılar), Biyom, Işık seviyesi ve Hedeflenen blok bilgisi.
  - *Sağ Taraf*: OpenGL 3.3 Core Profile, GPU marka/model, bellek kullanımı, ekran çözünürlüğü.
- **`[F1]` Tuşu**: Sinematik ekran görüntüleri ve videolar için tüm HUD arayüzünü tek tuşla gizler/gösterir.

### 5. 🧱 Keskin 16x16 Piksel Dokuları (Nearest-Neighbor Filter)
- `GL_TEXTURE_MAG_FILTER` parametresi `GL_NEAREST` yapılarak bloklara yakından bakıldığında bulanıklaşma tamamen kaldırılmış, kristal netliğinde nostaljik 16x16 Minecraft piksel grafikleri elde edilmiştir.

---

## 📜 License
Distributed under the MIT License. See `LICENSE` for details.
