# Flipper Mon

A Pokémon-style monster battling RPG for the Flipper Zero.

---

### ⚠️ Work in Progress

This project is currently under active development. Features are incomplete, bugs are expected, and the gameplay is not yet fully balanced. Please feel free to report any issues or suggest new features!

---

## About The Project

Flipper Mon is a passion project to create a retro-style RPG adventure that can be played on the Flipper Zero's monochrome screen. It draws heavy inspiration from the classic Game Boy Pokémon games, featuring tile-based exploration, random encounters, and a turn-based battle system. The project can be followed on here as well as on https://www.purplefox.io/blog/flipper-mon

### Screenshots

| Exploration View | Battle Splash | Battle Scene | 
| :---: | :---: | :---: |
| ![Exploration Screenshot](https://www.purplefox.io/images/flipper_mon_explore.png) | ![Battle Screenshot](https://www.purplefox.io/images/flipper_mon_battle_splash.png) | ![Battle Scene Screenshot](https://www.purplefox.io/images/flipper_mon_battle_scene.png)

---

## How It Works

The game is built in C using the Flipper Zero SDK and is structured around a few core concepts:

#### 1. Scene Manager & Game Loop
The game operates on a simple scene manager (`SceneExploration`, `SceneBattle`) within the main application loop. This loop processes input events from the Flipper's D-Pad and buttons, updates the game state, and calls the appropriate draw function for the current scene.

#### 2. Data-Driven Design
All creatures, moves, and stats are defined in a data-driven way. For example, `pokemon.h` contains the `struct` definitions for a Pokémon and its moves, while `pokemon.c` holds arrays of base stats and default move sets. This makes it easy to add new creatures or rebalance existing ones without changing the core engine code.

#### 3. Monochrome Bitmaps for Sprites
All visual assets, including the player character, the creatures (front and back sprites), and the world tiles, are stored as monochrome bitmaps. These are C-style `unsigned char` arrays that are drawn directly to the canvas using the Flipper Zero's rendering functions. This is an efficient way to handle graphics on the device's hardware.

---

## How to Build and Run

To compile and run Flipper Mon on your device, you need a working Flipper Zero development environment with the **ufbt** (Unofficial Build Tool) installed.

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/purplefox-io/flipper-mon.git
    ```

2.  **Navigate to the project directory:**
    ```bash
    cd flipper_mon
    ```

3.  **Build the application:**
    Run the build tool from the project root. This will compile the code and create a `.fap` application file.
    ```bash
    ufbt
    ```

4.  **Launch on your Flipper:**
    Connect your Flipper Zero to your computer via USB. The following command will automatically deploy the `.fap` file to your Flipper's SD card and launch the game.
    ```bash
    ufbt launch
    ```

Enjoy your adventure!
