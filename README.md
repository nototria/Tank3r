# Tank3r
Online tanks game based on C/C++ in CLI

## compile
to compile server, just run `make` under `server` directory

to compile client, run the following commands under `client` directory

**ubuntu**
```
apt install libncursesw5-dev
g++ -std=c++17 -lncursesw main.cpp -o Tank3r
```

**macOS**
```
brew install ncurses
g++ -std=c++17 -D_XOPEN_SOURCE_EXTENDED -lncurses main.cpp -o Tank3r
```
## usage
```
Tank3r 127.0.0.1
```
