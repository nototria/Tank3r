# FILE: README.md
# Tank3r
Online tank game developed in C/C++ with a command-line interface (CLI).

---

## Table of Contents

- [FILE: README.md](#file-readmemd)
- [Tank3r](#tank3r)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Features](#features)
  - [Installation](#installation)
    - [Prerequisites](#prerequisites)
    - [Steps](#steps)
  - [Usage Instructions](#usage-instructions)
  - [Project Structure](#project-structure)

## Introduction

**Tank3r** is a multiplayer online tank battle game developed in C/C++ and designed to run on a command-line interface (CLI). The game supports randomly generated maps featuring terrains such as water, rocks, ice, and tanks, providing a real-time combat experience.

## Features

- **Multiplayer Gameplay**: Supports multiple players in simultaneous online battles.
- **Random Map Generation**: Each game features a unique map with various terrain elements like water, rocks, and ice.
- **Real-Time Communication**: Ensures game state synchronization between clients and the server.
- **CLI Visual Interface**: Utilizes ANSI escape codes for efficient visual effects in the command-line interface.
- **Cross-Platform Support**: Runs on Linux and can be extended to other platforms.

## Installation

### Prerequisites

- **Operating System**: Linux
- **Compiler**: GCC or Clang
- **CMake**: Version 3.10 or above

### Steps

1. **Clone the Repository**

    ```bash
    git clone https://github.com/yourusername/Tank3r.git
    cd Tank3r
    ```

2. **Create Build Directory**

    ```bash
    mkdir build
    cd build
    ```

3. **Build the Project**

    ```bash
    cmake ..
    make
    ```

4. **Run the Server**

    ```bash
    ./server
    ```

5. **Run the Client**

    In a separate terminal, execute:

    ```bash
    ./client
    ```

## Usage Instructions

1. After starting the server, clients can connect using the server's IP address and port.
2. Once in the game, players can control their tanks by moving, rotating, and firing.
3. Maps are randomly generated for each session, providing unique gameplay experiences.
4. Multiple players can join simultaneously for competitive battles.

## Project Structure

```plaintext
Tank3r/
├── server/
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Server.cpp
│   │   ├── Server.h
│   │   ├── Game.cpp
│   │   └── Game.h
│   ├── tests/
│   │   └── ServerTest.cpp
│   └── Makefile
├── client/
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Client.cpp
│   │   ├── Client.h
│   │   ├── Renderer.cpp
│   │   └── Renderer.h
│   ├── tests/
│   │   └── ClientTest.cpp
│   └── Makefile
├── shared/
│   ├── src/
│   │   ├── Protocol.cpp
│   │   ├── Protocol.h
│   │   ├── MapGenerator.cpp
│   │   └── MapGenerator.h
│   └── Makefile
├── docs/
│   └── README.md
├── .gitignore
└── CMakeLists.txt
```