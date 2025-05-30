# Tank3r

**Online Multiplayer Tank Game (CLI)**

Tank3r is a cross-platform, terminal-based tank battle game implemented in C/C++ using the ncurses library. It features a client-server architecture that enables multiple players to engage in real-time combat over a network.

## Table of Contents

* [Overview](#overview)
* [Features](#features)
* [Project Structure](#project-structure)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
* [Usage](#usage)
* [Documentation](#documentation)

## Overview

Tank3r provides a simple yet engaging environment for competitive tank battles within the terminal. Players control tanks on a shared map, navigating obstacles and attempting to eliminate other players.

## Features

* **Real-Time Multiplayer**: Multiple clients connect to a central server to play together.
* **Terminal Graphics**: Renders game graphics using ncurses for text-based visuals.
* **Cross-Platform**: Supports Linux and macOS environments.
* **Modular Architecture**: Clear separation of client, server, and shared code.
* **Configurable Map and Game Parameters**: Easy adjustment of map size and gameplay settings.

## Project Structure

```
Tank3r
├── README.md
├── client
│   ├── GameSync.hpp
│   ├── connect.cpp
│   └── main.cpp
├── devStructure.md
├── report.md
├── server
│   ├── ClientManager.cpp
│   ├── ClientManager.hpp
│   ├── GameServer.cpp
│   ├── GameServer.hpp
│   ├── Makefile
│   ├── RoomManager.cpp
│   ├── RoomManager.hpp
│   └── main.cpp
├── server.md
└── shared
    ├── GameObject.h
    ├── GameParameters.h
    ├── GameUtils.hpp
    ├── InputStruct.hpp
    ├── UpdateStruct.hpp
    └── map_generator.cpp
```

## Prerequisites

* **Compiler**: g++ (C++17 support)
* **ncurses Library**:

  * On Linux: `libncursesw5-dev`
  * On macOS: `ncurses` via Homebrew

## Installation

1. **Clone the repository**:

   ```bash
   git clone https://github.com/nototria/Tank3r.git
   cd Tank3r
   ```

2. **Build the Server**:

   ```bash
   cd server
   make
   ```

3. **Build the Client**:

   * On Linux:

     ```bash
     cd ../client
     sudo apt install libncursesw5-dev
     g++ -std=c++17 -lncursesw main.cpp -o tank3r
     ```
   * On macOS:

     ```bash
     cd ../client
     brew install ncurses
     g++ -std=c++17 -D_XOPEN_SOURCE_EXTENDED -lncurses main.cpp -o tank3r
     ```

## Usage

1. **Start the Server**:

   ```bash
   cd server
   ./server
   ```

2. **Launch the Client**:

   ```bash
   cd client
   ./tank3r <server_ip>
   ```

   Replace `<server_ip>` with the IP address of the server (e.g., `127.0.0.1`).

3. **Gameplay Controls**:

   * Use the arrow keys or `WASD` to move your tank.
   * Press the spacebar to fire a shell.
   * Navigate obstacles and outmaneuver opponents to win.

## Documentation

* **Development Structure**: See `devStructure.md` for an overview of architecture and coding guidelines.
* **Server Details**: See `server.md` for in-depth server implementation and networking logic.
* **Project Report**: See `report.md` for design rationale, performance analysis, and future work.
