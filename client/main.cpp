#include <map>
#include <ncurses.h>
#include "../shared/GameObject.h"
#include "../shared/map_generator.cpp"
#include "../client/connect.cpp"
#include"GameSync.hpp"

// define
std::vector<int> checkBulletTankCollisions(std::map<int,Tank>& tanksMap);
void handleBulletCollisions(std::map<int,Tank>& tanksMap);
void initGame(int& width, int& height, WINDOW*& titleWin, WINDOW*& inputWin, WINDOW*& roomWin, WINDOW*& gameWin, WINDOW*& endWin, GameState& state);
void renderTank(WINDOW* win, Tank& tank);
void renderStaticObjects(WINDOW* win, const std::vector<MapObject>& objects);
void renderPlayerInfo(WINDOW* win, const std::string& playerName, int playerHP, int color);
void drawTitleScreen(WINDOW* win);
void drawUsernameInput(WINDOW* win, const std::string& username);
void drawCustomBorder1(WINDOW* win);
void drawCustomBorder2(WINDOW* win);
void joinRoomById(WINDOW* win, std::string& roomId, int& connfd, int& clientId, std::string& username);
void quickJoin(WINDOW* win, std::string& roomId, int& connfd, int& clientId, std::string& username);
void RoomMenu(std::string serverIP, WINDOW* win, GameState& state, std::string& roomId, int& connfd, int& clientId, std::string& username);
void InRoomMenu(WINDOW* win, GameState& state, std::string& roomId, int& connfd, int &playerNum);
void gameLoop(std::string serverIP, WINDOW* gridWin, int gridWidth, int gridHeight, std::vector<MapObject>& staticObjects, GameState& state, const int& clientId, int playerNum, std::map<int,std::string>&id2Names);
void drawWinScreen(WINDOW* win);
void drawLoseScreen(WINDOW* win);
void drawTieScreen(WINDOW* win);

// client side
void initGame(int& width, int& height, WINDOW*& titleWin, WINDOW*& inputWin, WINDOW*& roomWin, WINDOW*& gameWin, WINDOW*& endWin, GameState& state) {
    initscr();
    noecho();
    cbreak();
    start_color();
    curs_set(0);
    setlocale(LC_ALL, "");

    // Initialize color pairs
    init_color(COLOR_GRAY, 300, 300, 300);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    init_pair(8, COLOR_GRAY, COLOR_BLACK);

    init_pair(11, COLOR_RED, COLOR_CYAN);
    init_pair(12, COLOR_GREEN, COLOR_CYAN);
    init_pair(13, COLOR_YELLOW, COLOR_CYAN);
    init_pair(14, COLOR_BLUE, COLOR_CYAN);
    init_pair(15, COLOR_MAGENTA, COLOR_CYAN);
    init_pair(16, COLOR_CYAN, COLOR_CYAN);
    init_pair(17, COLOR_WHITE, COLOR_CYAN);

    width = SCREEN_WIDTH;
    height = SCREEN_HEIGHT;

    int startX = (COLS - width) / 2;
    int startY = (LINES - height) / 2;

    // Create windows
    titleWin = newwin(height, width, startY, startX);
    inputWin = newwin(height, width, startY, startX);
    roomWin = newwin(height, width, startY, startX);
    gameWin = newwin(height, width, startY, startX);
    endWin = newwin(height, width, startY, startX);

    // Initialize game state and username
    state = GameState::TitleScreen;
}

void renderTank(WINDOW* win, Tank& tank) {
    int x = tank.getX();
    int y = tank.getY();
    int color = tank.getColor();
    wchar_t directionSymbol = tank.getDirectionSymbol();
    cchar_t cc; setcchar(&cc, &directionSymbol, A_NORMAL, 0, NULL);

    wattron(win, COLOR_PAIR(color)); 
    mvwadd_wch(win, y, x, &cc); //tank body
    wattroff(win, COLOR_PAIR(color));
    for (const auto& bullet : tank.getBullets()) { // bullets
        if (bullet.getInWater()) {
            wattron(win, COLOR_PAIR(color+10));
            mvwaddch(win, bullet.getY(), bullet.getX(), bullet.getSymbol());
            wattroff(win, COLOR_PAIR(color+10));
        } else {
            wattron(win, COLOR_PAIR(color));
            mvwaddch(win, bullet.getY(), bullet.getX(), bullet.getSymbol());
            wattroff(win, COLOR_PAIR(color));
        }
    }
}

void renderStaticObjects(WINDOW* win, const std::vector<MapObject>& objects) {
    setlocale(LC_ALL, "");
    for (const auto& obj : objects) {
        if(obj.getType() == MapObjectType::wall) {
            wattron(win, COLOR_PAIR(COLOR_GRAY));
        } else if(obj.getType() == MapObjectType::water) {
            wattron(win, COLOR_PAIR(COLOR_CYAN));
        }
        mvwaddwstr(win, obj.getY(), obj.getX(), obj.getSymbol().c_str());
        wattroff(win, COLOR_PAIR(COLOR_GRAY) & COLOR_PAIR(COLOR_CYAN));
    }
}

void renderPlayerInfo(WINDOW* win, const std::string& playerName, int playerHP, int color) {
    werase(win);
    // name
    mvwprintw(win, 2, 2, "%s", playerName.c_str());
    // HP value
    mvwprintw(win, 5, 2, "HP: %d", playerHP);
    // HP bar
    wattron(win, COLOR_PAIR(color));
    drawCustomBorder2(win);
    std::wstring hpBar(playerHP, L'█');
    mvwaddwstr(win, 7, 3, hpBar.c_str());
    wattroff(win, COLOR_PAIR(color));
    wrefresh(win);
}

void drawTitleScreen(WINDOW* win) {
    setlocale(LC_ALL, "");
    keypad(win, TRUE);
    nodelay(win, TRUE);

    int maxY, maxX;
    getmaxyx(win, maxY, maxX);
    const wchar_t* tank3rArt[] = {
        L"████████╗ █████╗ ███╗   ██╗██╗  ██╗██████╗ ██████╗ ",
        L"╚══██╔══╝██╔══██╗████╗  ██║██║ ██╔╝╚════██╗██╔══██╗",
        L"   ██║   ███████║██╔██╗ ██║█████╔╝  █████╔╝██████╔╝",
        L"   ██║   ██╔══██║██║╚██╗██║██╔═██╗  ╚═══██╗██╔══██╗",
        L"   ██║   ██║  ██║██║ ╚████║██║  ██╗██████╔╝██║  ██║",
        L"   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝",};
    // const wchar_t* tank3rArt[] = {
    //     L"░▒▓████████▓▒░  ░▒▓██████▓▒░  ░▒▓███████▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓███████▓▒░  ░▒▓███████▓▒░  ",
    //     L"   ░▒▓█▓▒░     ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░        ░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ",
    //     L"   ░▒▓█▓▒░     ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░        ░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ",
    //     L"   ░▒▓█▓▒░     ░▒▓████████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓███████▓▒░  ░▒▓███████▓▒░  ░▒▓███████▓▒░  ",
    //     L"   ░▒▓█▓▒░     ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░        ░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ",
    //     L"   ░▒▓█▓▒░     ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░        ░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ",
    //     L"   ░▒▓█▓▒░     ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓███████▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ "};
    
    int artWidth = wcslen(tank3rArt[0]);
    int startX = (maxX - artWidth) / 2;
    int artLines = sizeof(tank3rArt) / sizeof(tank3rArt[0]);
    int startY = (maxY - artLines) / 3;

    // Display
    werase(win);
    // box(win, 0, 0);
    wattron(win, COLOR_PAIR(COLOR_YELLOW));
    for (int i = 0; i < artLines; i++) {
        mvwaddwstr(win, startY + i, startX, tank3rArt[i]);
    }
    wattroff(win, COLOR_PAIR(COLOR_YELLOW));

    // Subtitle
    const char* subtitle = "Press 'e' to start";
    int subtitleY = startY + artLines + 5;
    int subtitleX = (maxX - strlen(subtitle)) / 2;

    wattron(win, COLOR_PAIR(COLOR_CYAN) | A_BOLD | A_BLINK);
    mvwprintw(win, subtitleY, subtitleX, "%s", subtitle);
    wattroff(win, COLOR_PAIR(COLOR_CYAN)| A_BOLD | A_BLINK);
    while (true) {
        int ch = wgetch(win);
        if (ch == 'e' || ch == 'E') {break;}

    }
    wrefresh(win);
}

void drawUsernameInput(WINDOW* win, const std::string& username) {
    int maxY, maxX;
    getmaxyx(win, maxY, maxX);
    const wchar_t* tank3rArt[] = {
        L"  _____   __   __     ___     ___     _  _      ___               _  _      ___    __  __     ___   ",
        L" |_   _|  \\ \\ / /    | _ \\   |_ _|   | \\| |    / __|      o O O  | \\| |    /   \\  |  \\/  |   | __|  ",
        L"   | |     \\ V /     |  _/    | |    | .` |   | (_ |     o       | .` |    | - |  | |\\/| |   | _|   ",
        L"  _|_|_    _|_|_    _|_|_    |___|   |_|\\_|    \\___|    TS__[O]  |_|\\_|    |_|_|  |_|__|_|   |___|  ",
        L"_|\"\"\"\"\"| _| \"\"\" | _| \"\"\" | _|\"\"\"\"\"| _|\"\"\"\"\"| _|\"\"\"\"\"|  {======| _|\"\"\"\"\"| _|\"\"\"\"\"| _|\"\"\"\"\"| _|\"\"\"\"\"| ",
        L"\"`-0-0-' \"`-0-0-' \"`-0-0-' \"`-0-0-' \"`-0-0-' \"`-0-0-' ./o--000' \"`-0-0-' \"`-0-0-' \"`-0-0-' \"`-0-0-' "};
    int artWidth = wcslen(tank3rArt[0]);
    int startX = (maxX - artWidth) / 2;
    int artLines = sizeof(tank3rArt) / sizeof(tank3rArt[0]);
    int startY = (maxY - artLines) / 5;


    // Display
    werase(win);
    // logo
    wattron(win, COLOR_PAIR(COLOR_YELLOW));
        for (int i = 0; i < artLines; i++) {
            mvwaddwstr(win, startY + i, startX, tank3rArt[i]);
        }
    wattroff(win, COLOR_PAIR(COLOR_YELLOW));

    // instructions
    const char* subtitle = "Please enter your username and press Enter:";
    int subtitleX = (maxX - strlen(subtitle))/2;
    int subtitleY = startY + artLines + 3;
    wattron(win, A_BOLD);
        mvwprintw(win,  subtitleY, subtitleX, "%s", subtitle);
    wattroff(win, A_BOLD);

    // Label and input field
    int inputX = (maxX - username.length() % 2 == 0) ? (maxX - username.length())/2 : (maxX - username.length())/2 - 1;
    int inputY = startY + artLines + 10;
    wattron(win, A_UNDERLINE | A_BOLD);
        mvwprintw(win, inputY, inputX, "%s", username.c_str());
    wattroff(win, A_UNDERLINE | A_BOLD);

    // decorative footer
    int footerX = (maxX - 40)/2;
    int footerY = startY + artLines + 20;
    mvwprintw(win, footerY, footerX,   "***************************************");
    mvwprintw(win, footerY+1, footerX, "*       Welcome to Tank3r Game        *");
    mvwprintw(win, footerY+2, footerX, "***************************************");
    wrefresh(win);
}

void drawCustomBorder1(WINDOW* win) {
    std::setlocale(LC_ALL, "");
    cchar_t vertical, horizontal, topLeft, topRight, bottomLeft, bottomRight;
    wchar_t verticalChar[] = {L'▒', 0};
    wchar_t horizontalChar[] = {L'▒', 0};
    wchar_t topLeftChar[] = {L'▒', 0};
    wchar_t topRightChar[] = {L'▒', 0};
    wchar_t bottomLeftChar[] = {L'▒', 0};
    wchar_t bottomRightChar[] = {L'▒', 0};
    setcchar(&vertical, verticalChar, 0, 0, nullptr);
    setcchar(&horizontal, horizontalChar, 0, 0, nullptr);
    setcchar(&topLeft, topLeftChar, 0, 0, nullptr);
    setcchar(&topRight, topRightChar, 0, 0, nullptr);
    setcchar(&bottomLeft, bottomLeftChar, 0, 0, nullptr);
    setcchar(&bottomRight, bottomRightChar, 0, 0, nullptr);
    wattron(win, COLOR_PAIR(COLOR_CYAN));
    wborder_set(win, &vertical, &vertical, &horizontal, &horizontal,&topLeft, &topRight, &bottomLeft, &bottomRight);
    wattroff(win, COLOR_PAIR(COLOR_CYAN));
}

void drawCustomBorder2(WINDOW* win) {
    std::setlocale(LC_ALL, "");
    cchar_t vertical, horizontal, topLeft, topRight, bottomLeft, bottomRight;
    wchar_t verticalChar[] = {L'║', 0};
    wchar_t horizontalChar[] = {L'═', 0};
    wchar_t topLeftChar[] = {L'╔', 0};
    wchar_t topRightChar[] = {L'╗', 0};
    wchar_t bottomLeftChar[] = {L'╚', 0};
    wchar_t bottomRightChar[] = {L'╝', 0};
    setcchar(&vertical, verticalChar, 0, 0, nullptr);
    setcchar(&horizontal, horizontalChar, 0, 0, nullptr);
    setcchar(&topLeft, topLeftChar, 0, 0, nullptr);
    setcchar(&topRight, topRightChar, 0, 0, nullptr);
    setcchar(&bottomLeft, bottomLeftChar, 0, 0, nullptr);
    setcchar(&bottomRight, bottomRightChar, 0, 0, nullptr);
    wborder_set(win, &vertical, &vertical, &horizontal, &horizontal,&topLeft, &topRight, &bottomLeft, &bottomRight);
}

void handleUsernameInput(WINDOW* win, std::string& username) {
    int maxY, maxX;
    getmaxyx(win, maxY, maxX);
    drawUsernameInput(win, username);
    int ch, cursor_position = username.length() + 1;
    noecho();
    keypad(win, TRUE);
    while (true) {
        int inputX = (maxX - username.length())/2;
        ch = wgetch(win);
        if (ch == '\n') {
            break;
        } else if(ch == ','){
            curs_set(0);
            mvwprintw(win, 20, (maxX-21)/2, "Invalid character ','");
            continue;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            curs_set(1);
            if (!username.empty() && cursor_position > 1) {
                username.erase(cursor_position - 2, 1);
                cursor_position--;
            }
        } else if (ch == KEY_LEFT || ch == KEY_RIGHT) {
            curs_set(1);
            switch (ch) {
                case KEY_LEFT:
                    if (cursor_position > 1) cursor_position--;
                    break;
                case KEY_RIGHT:
                    if (cursor_position <= (int)username.length()) cursor_position++;
                    break;
            }
        } else if (ch >= 32 && ch <= 126 && username.length() < 20) {
            curs_set(1);
            username.insert(cursor_position - 1, 1, ch);
            cursor_position++;
        }
        drawUsernameInput(win, username);
        wmove(win, 22, (maxX - username.length() % 2 == 0) ? (maxX - username.length())/2 : (maxX - username.length())/2 - 1 + cursor_position - 1);
        wrefresh(win);
    }
    curs_set(0);
}

void joinRoomById(WINDOW* win, std::string& roomId, int& connfd, int& clientId, std::string& username) {
    // Prompt user to enter a room ID
    nodelay(win, FALSE); // Blocking mode
    wattron(win, COLOR_PAIR(COLOR_YELLOW));
    mvwprintw(win, 18, 34, "Enter Room ID (4 digits): ");
    wrefresh(win);
    echo();
    curs_set(1);
    char input[4];
    bool validInput = false;
    while (!validInput) {
        mvwgetnstr(win, 18, 60, input, 4);
        roomId = std::string(input);
        if (roomId.length() == 4 && std::all_of(roomId.begin(), roomId.end(), ::isdigit)) {
            validInput = true;
        } else {
            mvwprintw(win, 18, 60, "    ");
            mvwprintw(win, 16, 30, "Invalid Room ID. Please enter 4 digits.");
            wrefresh(win);
        }
    }
    noecho();
    curs_set(0);
    mvwprintw(win, 20, 40, "Joining Room %s...", roomId.c_str());
    wrefresh(win);
    wattron(win, COLOR_PAIR(COLOR_YELLOW));

    // do the actual connection
    clientId = receiveClientId(connfd);
    sendUserName(connfd, clientId, username.c_str());
    joinRoom(connfd, roomId.c_str());
}

void quickJoin(WINDOW* win, std::string& roomId, int& connfd, int& clientId, std::string& username) {
    wattron(win, COLOR_PAIR(COLOR_YELLOW));
    mvwprintw(win, 18, 33, "Quick Joining to a Random Room...");
    wattron(win, COLOR_PAIR(COLOR_YELLOW));
    wrefresh(win);

    // do the actual connection
    clientId = receiveClientId(connfd);
    sendUserName(connfd, clientId, username.c_str());
    roomId = joinRoom(connfd, "-1");
}

void RoomMenu(std::string serverIP, WINDOW* win, GameState& state, std::string& roomId, int& connfd, int& clientId, std::string& username) {
    setlocale(LC_ALL, "");
    keypad(win, TRUE);
    nodelay(win, TRUE); // non-blocking mode

    int maxY, maxX;
    getmaxyx(win, maxY, maxX);
    const wchar_t* menuArt[] = {
        L"██████╗  ██████╗  ██████╗ ███╗   ███╗",
        L"██╔══██╗██╔═══██╗██╔═══██╗████╗ ████║",
        L"██████╔╝██║   ██║██║   ██║██╔████╔██║",
        L"██╔══██╗██║   ██║██║   ██║██║╚██╔╝██║",
        L"██║  ██║╚██████╔╝╚██████╔╝██║ ╚═╝ ██║",
        L"╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝     ╚═╝",};
    int artWidth = wcslen(menuArt[0]);
    int startX = (maxX - artWidth) / 2;
    int artLines = sizeof(menuArt) / sizeof(menuArt[0]);
    int startY = 6;

    // Display ASCII art
    werase(win);
    wattron(win, COLOR_PAIR(1));
    for (int i = 0; i < artLines; i++) {
        mvwaddwstr(win, startY + i, startX, menuArt[i]);
    }
    wattroff(win, COLOR_PAIR(1));

    // Welcome message
    const char* welcome_msg = "Welcome to the Room Menu!";
    int welcome_msgY = startY + artLines + 2;
    int welcome_msgX = (maxX - strlen(welcome_msg)) / 2;
    wattron(win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win, welcome_msgY, welcome_msgX, "%s", welcome_msg);
    wattroff(win, COLOR_PAIR(2) | A_BOLD);

    // Option hints
    const char* options[] = {
        "Join or create a room by typing Room ID",
        "Quick Join (Auto Join a Random Room)",
        "Re-type username",
    };

    int numOptions = sizeof(options) / sizeof(options[0]);
    int optionStartY = welcome_msgY + 3;
    int optionStartX = (maxX - 40) / 2;
    int selectedOption = 0;
    while (true) {
        
        // Handle input
        int ch = wgetch(win);
        if(ch != '\n'){
            for (int i = 0; i < numOptions; ++i) {
                if (i == selectedOption) {
                    wattron(win, COLOR_PAIR(3) | A_REVERSE);
                } else {
                    wattron(win, COLOR_PAIR(3));
                }
                mvwprintw(win, optionStartY + i, optionStartX, "%s", options[i]);
                wattroff(win, COLOR_PAIR(3) | A_REVERSE);
            }
        }else{
            mvwprintw(win, optionStartY + 0, optionStartX, "                                        ");
            mvwprintw(win, optionStartY + 1, optionStartX, "                                        ");
            mvwprintw(win, optionStartY + 2, optionStartX, "                                        ");
        }
        switch (ch) {
            case KEY_UP:
                selectedOption = (selectedOption - 1 + numOptions) % numOptions;
                break;
            case KEY_DOWN:
                selectedOption = (selectedOption + 1) % numOptions;
                break;
            case '\n':  // Enter key
                switch (selectedOption+1) {
                    case 1:  // Join a room by ID
                        // connect to server
                        if((connfd = connectToServer(serverIP.c_str(), SERV_PORT)) == -1){
                            werase(win);
                            mvwprintw(win, optionStartY + 0, optionStartX, "Failed to connect to the server");
                            wrefresh(win);
                            napms(2000);
                            return;
                        }else{
                            state = GameState::InRoom;
                            joinRoomById(win, roomId, connfd, clientId, username);
                        }
                        break;
                    case 2:  // Quick Join
                        // connect to server
                        if((connfd = connectToServer(serverIP.c_str(), SERV_PORT)) == -1){
                            werase(win);
                            mvwprintw(win, optionStartY + 0, optionStartX, "Failed to connect to the server");
                            wrefresh(win);
                            napms(2000);
                            return;
                        } else{
                            state = GameState::InRoom;
                            quickJoin(win, roomId, connfd, clientId, username);
                        }
                        break;

                    case 3: // back to typing username
                        state = GameState::UsernameInput;
                        break;
                }
                wrefresh(win);
                return;
                break;
        }
    }
}

void InRoomMenu(WINDOW* win, GameState& state, std::string& roomId, int& connfd, int &playerNum) {
    setlocale(LC_ALL, "");
    keypad(win, TRUE);
    nodelay(win, TRUE); // non-blocking mode

    int maxY, maxX;
    getmaxyx(win, maxY, maxX);
    const wchar_t* menuArt[] = {
        L"██████╗  ██████╗  ██████╗ ███╗   ███╗",
        L"██╔══██╗██╔═══██╗██╔═══██╗████╗ ████║",
        L"██████╔╝██║   ██║██║   ██║██╔████╔██║",
        L"██╔══██╗██║   ██║██║   ██║██║╚██╔╝██║",
        L"██║  ██║╚██████╔╝╚██████╔╝██║ ╚═╝ ██║",
        L"╚═╝  ╚═╝ ╚═════╝  ╚═════╝ ╚═╝     ╚═╝",};

    int artWidth = wcslen(menuArt[0]);
    int artLines = sizeof(menuArt) / sizeof(menuArt[0]);
    int artStartX = (maxX - artWidth) / 2;
    int artStartY = 6;

    // Display ASCII art
    werase(win);
    wattron(win, COLOR_PAIR(1));
    for (int i = 0; i < artLines; i++) {
        mvwaddwstr(win, artStartY + i, artStartX, menuArt[i]);
    }
    wattroff(win, COLOR_PAIR(1));

    // Room information message
    const char* roomMsg = "You are in the Room with ID: ";
    int roomMsgY = artStartY + artLines + 2;
    int roomMsgX = (maxX - strlen(roomMsg) - roomId.length()) / 2;
    wattron(win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(win, roomMsgY, roomMsgX, "%s%s", roomMsg, roomId.c_str());
    wattroff(win, COLOR_PAIR(2) | A_BOLD);


    std::string options[2]{"", "Exit Room"};

    int numOptions = 2;

    int selectedOption = 1;

    bool isHost=false;
    while (true) {
        // Option hints
        options[0]=isHost ? "Start Game" : "Waiting for the Host to Start...";
        int optionStartY = roomMsgY + 3;
        int optionStartX = (maxX - options[0].length()) / 2;
        for(int i = 0; i < numOptions; ++i){
            mvwprintw(win, optionStartY + i, optionStartX-15,"                                             ");
        }
        for (int i = 0; i < numOptions; ++i) {
            if (i == selectedOption) {
                wattron(win, COLOR_PAIR(3) | A_REVERSE);
            } else {
                wattron(win, COLOR_PAIR(3));
            }
            optionStartX = (maxX - options[i].length()) / 2;
            mvwprintw(win, optionStartY + i, optionStartX, "%s", options[i].c_str());
            wattroff(win, COLOR_PAIR(3) | A_REVERSE);
        }
        wrefresh(win);
        bool sigStart=false;
        InRoomListen(connfd, isHost, sigStart, playerNum);
        if(sigStart){
            state = GameState::GameLoop;
            return;
        }
        
        // Handle input
        int ch = wgetch(win);
        switch (ch) {
            case KEY_UP:
                if (isHost) selectedOption = (selectedOption - 1 + numOptions) % numOptions;
                break;
            case KEY_DOWN:
                if (isHost) selectedOption = (selectedOption + 1) % numOptions;
                break;
            case '\n': // Enter key
                if (isHost && selectedOption == 0) {
                    // Start Game logic (Host only)
                    startGame(connfd, roomId.c_str());
                } else if (selectedOption == 1 || !isHost) {
                    // Exit Room
                    exitRoom(connfd, roomId.c_str());
                    state = GameState::RoomMenu;
                    return;
                }
                break;
        }
    }
}

void gameLoop(std::string serverIP, WINDOW* gridWin, int gridWidth, int gridHeight, std::vector<MapObject>& staticObjects, GameState& state, const int& clientId, int playerNum, std::map<int,std::string>&id2Names) {
    // status windows
    int startX = (COLS - gridWidth) / 2, startY = (LINES - gridHeight) / 2;
    std::map<int, WINDOW*> StatusWin;
    std::vector<int> clientIds;
    for(auto &[id,_]: id2Names){
        clientIds.push_back(id);
    }
    StatusWin[clientIds[0]] = newwin(statusBlockHeight, statusBlockWidth, startY, startX - statusBlockWidth);
    if (playerNum >= 2) {
        StatusWin[clientIds[1]] = newwin(statusBlockHeight, statusBlockWidth, startY, startX + gridWidth);
    }
    if (playerNum >= 3) {
        StatusWin[clientIds[2]] = newwin(statusBlockHeight, statusBlockWidth, startY + gridHeight - statusBlockHeight, startX - statusBlockWidth);
    }
    if (playerNum >= 4) {
        StatusWin[clientIds[3]] = newwin(statusBlockHeight, statusBlockWidth, startY + gridHeight - statusBlockHeight, startX + gridWidth);
    }
    // default values set
    keypad(gridWin, TRUE);
    nodelay(gridWin, TRUE);
    drawCustomBorder1(gridWin);
    bool loopRunning = true;

    //tanks
    std::map<int, Tank> tankMap =  Tank::createTank(playerNum, clientIds, gridWidth, gridHeight);;
    Tank &myTank = tankMap[clientId];

    for(auto &[tankId,tank]: tankMap){
        tank.setName(id2Names[tankId]);
        if(tankId != clientId){
            tank.setColor(COLOR_RED);
        }else{
            tank.setColor(COLOR_GREEN);
        }
        renderTank(gridWin, tank);
    }

    //setup udp connection
    int udpfd = connectUDP(serverIP.c_str(), UDP_PORT);
    GameSync gameSync(udpfd, clientId, myTank);

    // game loop
    int lastKey = -1; // track the last key
    gameSync.start_inGame_listen();
    GameTimer timer(0.128); // 7.5 FPS
    while (loopRunning) {

        // server side: playernum count simulation
        int tmpPlayerNum = playerNum;
        for(auto &[tankId,tank]: tankMap){
            if(!tank.IsAlive()){
                tmpPlayerNum--;
            }
        }
        if (tmpPlayerNum == 1 && myTank.getHP() > 0) { // test only(reajust after server side implementation)
            state = GameState::WinScreen;
            loopRunning = false;
            break;
        } else if (myTank.getHP() <= 0) {
            state = GameState::LoseScreen;
            loopRunning = false;
            break;
        }
        // end of simulation

        int ch = wgetch(gridWin);
        if (ch != ERR) {lastKey = ch;}

        if (timer.shouldUpdate()) {
            /*
            switch (lastKey) {
                case ' ': {
                    myTank.fireBullet();
                    gameSync.send_input(' ');
                    break;
                }
                case 'w': {
                    myTank.setDirection(Direction::Up);
                    int nextY = myTank.getY() - 1;
                    if (nextY > 0 && !myTank.checkTankCollision(myTank.getX(), nextY, staticObjects)) {
                        // myTank.setY(nextY);
                        gameSync.send_input('w');
                    }
                    break;
                }
                case 's': {
                    myTank.setDirection(Direction::Down);
                    int nextY = myTank.getY() + 1;
                    if (nextY < gridHeight - 1 && !myTank.checkTankCollision(myTank.getX(), nextY, staticObjects)) {
                        // myTank.setY(nextY);
                        gameSync.send_input('s');
                    }
                    break;
                }
                case 'a': {
                    myTank.setDirection(Direction::Left);
                    int nextX = myTank.getX() - 1;
                    if (nextX > 0 && !myTank.checkTankCollision(nextX, myTank.getY(), staticObjects)) {
                        // myTank.setX(nextX);
                        gameSync.send_input('a');
                    }
                    break;
                }
                case 'd': {
                    myTank.setDirection(Direction::Right);
                    int nextX = myTank.getX() + 1;
                    if (nextX < gridWidth - 1 && !myTank.checkTankCollision(nextX, myTank.getY(), staticObjects)) {
                        // myTank.setX(nextX);
                        gameSync.send_input('d');
                    }
                    break;
                }
                case 'q': {
                    state = GameState::WinScreen;
                    loopRunning = false;
                    break;
                }
            }
            */
            switch (lastKey){
            case ' ':
                gameSync.send_input(' ');
                myTank.fireBullet();
                break;
            case 'w': gameSync.send_input('w');break;
            case 's': gameSync.send_input('s');break;
            case 'a': gameSync.send_input('a');break;
            case 'd': gameSync.send_input('d');break;
            case 4: {
                loopRunning = false;
                state = GameState::RoomMenu;
                break;
            }
            }
            lastKey = -1;   // Reset last key
            gameSync.update_tank(tankMap, staticObjects);

            // Render updates
            werase(gridWin);
            renderStaticObjects(gridWin, staticObjects);
            setlocale(LC_ALL, "");
            drawCustomBorder1(gridWin);

            for(auto &[tankId,tank]: tankMap){
                if(tank.getHP() > 0){
                    tank.updateBullets(gridWidth, gridHeight, staticObjects);
                    handleBulletCollisions(tankMap);
                    checkBulletTankCollisions(tankMap);
                    renderTank(gridWin, tank);
                }
                renderPlayerInfo(StatusWin[tankId], tank.getName(), tank.getHP(), tank.getColor());
                wrefresh(StatusWin[tankId]);
            }
            wrefresh(gridWin);
        }
    }
    gameSync.stop_inGame_listen();
    // Cleanup
    for(auto &client: clientIds){
        werase(StatusWin[client]);
        wrefresh(StatusWin[client]);
        delwin(StatusWin[client]);
    }
}

void drawWinScreen(WINDOW* win) {
    setlocale(LC_ALL, "");
    int maxY, maxX;
    getmaxyx(win, maxY, maxX);

    const wchar_t* winArt[] = {
        L"██╗    ██╗██╗███╗   ██╗",
        L"██║    ██║██║████╗  ██║",
        L"██║ █╗ ██║██║██╔██╗ ██║",
        L"██║███╗██║██║██║╚██╗██║",
        L"╚███╔███╔╝██║██║ ╚████║",
        L" ╚══╝╚══╝ ╚═╝╚═╝  ╚═══╝"
    };

    int artWidth = wcslen(winArt[0]);
    int artLines = sizeof(winArt) / sizeof(winArt[0]);
    int artStartX = (maxX - artWidth) / 2;
    int artStartY = (maxY - artLines) / 3;

    // Clear the window and draw border
    werase(win);
    wattron(win, COLOR_PAIR(1));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(1));

    // Render ASCII art
    wattron(win, COLOR_PAIR(2));
    for (int i = 0; i < artLines; i++) {
        mvwaddwstr(win, artStartY + i, artStartX, winArt[i]);
    }
    wattroff(win, COLOR_PAIR(2));

    // Victory message
    const char* victoryMsg = "Congratulations! You WIN!";
    int victoryMsgX = (maxX - strlen(victoryMsg)) / 2;
    int victoryMsgY = artStartY + artLines + 2;
    wattron(win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win, victoryMsgY, victoryMsgX, "%s", victoryMsg);
    wattroff(win, COLOR_PAIR(3) | A_BOLD);

    // Restart prompt
    const char* restartPrompt = "Press 'r' to restart or 'q' to quit.";
    int restartPromptX = (maxX - strlen(restartPrompt)) / 2;
    int restartPromptY = victoryMsgY + 2;
    wattron(win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win, restartPromptY, restartPromptX, "%s", restartPrompt);
    wattroff(win, COLOR_PAIR(4) | A_BOLD);

    // Refresh window
    wrefresh(win);
}

void drawLoseScreen(WINDOW* win) {
    setlocale(LC_ALL, "");
    int maxY, maxX;
    getmaxyx(win, maxY, maxX);

    const wchar_t* loseArt[] = {
        L" ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ ",
        L"██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗",
        L"██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝",
        L"██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗",
        L"╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝ ╚████╔╝ ███████╗██║  ██║",
        L" ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝"
    };

    int artWidth = wcslen(loseArt[0]);
    int artLines = sizeof(loseArt) / sizeof(loseArt[0]);
    int artStartX = (maxX - artWidth) / 2;
    int artStartY = (maxY - artLines) / 3;

    // Clear the window and draw border
    werase(win);
    wattron(win, COLOR_PAIR(1));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(1));

    // Render ASCII art
    wattron(win, COLOR_PAIR(2));
    for (int i = 0; i < artLines; i++) {
        mvwaddwstr(win, artStartY + i, artStartX, loseArt[i]);
    }
    wattroff(win, COLOR_PAIR(2));

    // Lose message
    const char* loseMsg = "Sorry! You LOSE!";
    int loseMsgX = (maxX - strlen(loseMsg)) / 2;
    int loseMsgY = artStartY + artLines + 2;
    wattron(win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win, loseMsgY, loseMsgX, "%s", loseMsg);
    wattroff(win, COLOR_PAIR(3) | A_BOLD);

    // Restart prompt
    const char* restartPrompt = "Press 'r' to restart or 'q' to quit.";
    int restartPromptX = (maxX - strlen(restartPrompt)) / 2;
    int restartPromptY = loseMsgY + 2;
    wattron(win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win, restartPromptY, restartPromptX, "%s", restartPrompt);
    wattroff(win, COLOR_PAIR(4) | A_BOLD);

    // Refresh window
    wrefresh(win);
}

void drawTieScreen(WINDOW* win) {
    setlocale(LC_ALL, "");
    int maxY, maxX;
    getmaxyx(win, maxY, maxX);

    const wchar_t* tieArt[] = {
        L"████████╗██╗███████╗",
        L"╚══██╔══╝██║██╔════╝",
        L"   ██║   ██║█████╗  ",
        L"   ██║   ██║██╔══╝  ",
        L"   ██║   ██║███████╗",
        L"   ╚═╝   ╚═╝╚══════╝"
    };

    int artWidth = wcslen(tieArt[0]);
    int artLines = sizeof(tieArt) / sizeof(tieArt[0]);
    int artStartX = (maxX - artWidth) / 2;
    int artStartY = (maxY - artLines) / 3;

    // Clear the window and draw border
    werase(win);
    wattron(win, COLOR_PAIR(1));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(1));

    // Render ASCII art
    wattron(win, COLOR_PAIR(2));
    for (int i = 0; i < artLines; i++) {
        mvwaddwstr(win, artStartY + i, artStartX, tieArt[i]);
    }
    wattroff(win, COLOR_PAIR(2));

    // Tie message
    const char* tieMsg = "It's a TIE!";
    int tieMsgX = (maxX - strlen(tieMsg)) / 2;
    int tieMsgY = artStartY + artLines + 2;
    wattron(win, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(win, tieMsgY, tieMsgX, "%s", tieMsg);
    wattroff(win, COLOR_PAIR(3) | A_BOLD);

    // Restart prompt
    const char* restartPrompt = "Press 'r' to restart or 'q' to quit.";
    int restartPromptX = (maxX - strlen(restartPrompt)) / 2;
    int restartPromptY = tieMsgY + 2;
    wattron(win, COLOR_PAIR(4) | A_BOLD);
    mvwprintw(win, restartPromptY, restartPromptX, "%s", restartPrompt);
    wattroff(win, COLOR_PAIR(4) | A_BOLD);

    // Refresh window
    wrefresh(win);
}

int main(int argc, char* argv[]) {
    // TBD: get other players info from server (playerNum)
    int playerNum = 4;
    std::string PlayerNames[playerNum];
    std::string ClientIds[playerNum];
    std::map<int, std::string> id2Names;
    std::string username="";
    std::string roomId="";
    std::string serverIP = "";
    int connfd = -1;
    int clientId = -1;
    if(argc == 2){
        serverIP = argv[1];
    }else{
        serverIP = serverIP;
    }
    WINDOW *titleWin, *inputWin, *roomWin,*gameWin, *endWin;
    GameState state;
    suppress_stderr();
    int width, height;
    initGame(width, height, titleWin, inputWin, roomWin, gameWin, endWin, state);

    bool running = true;
    while (running) {
        switch (state) {
            case GameState::TitleScreen: {
                drawTitleScreen(titleWin);
                state = GameState::UsernameInput;
                break;
            }

            case GameState::UsernameInput: {
                handleUsernameInput(inputWin, username);
                state = GameState::RoomMenu;
                break;
            }

            case GameState::RoomMenu: {
                RoomMenu(serverIP, roomWin, state, roomId, connfd, clientId, username);
                break;
            }

            case GameState::InRoom: {
                InRoomMenu(roomWin, state, roomId, connfd, playerNum);
                break;
            }

            case GameState::GameLoop: {
                //get other players info from server (PlayerNames)
                std::map<int, std::string> id2Names;
                int seed;
                getStartInfo(connfd, playerNum, id2Names, seed);
                //Generate static objects
                std::vector<MapObject> staticObjects = generateMap(width, height, seed);
                gameLoop(serverIP, gameWin, width, height, staticObjects, state, clientId, playerNum, id2Names);
                close(connfd);
                break;
            }

            case GameState::LoseScreen:
            case GameState::WinScreen:
            case GameState::TieScreen: {
                if(state == GameState::WinScreen) drawWinScreen(endWin);
                else if(state == GameState::LoseScreen) drawLoseScreen(endWin);
                else if(state == GameState::TieScreen) drawTieScreen(endWin);
                int ch = wgetch(endWin);
                if (ch == 'r' || ch == 'R') {
                    state = GameState::UsernameInput;
                }else if (ch == 'q' || ch == 'Q') {
                    running = false;
                }
                break;
            }
        }
    }
    delwin(titleWin);delwin(inputWin);delwin(roomWin);delwin(gameWin);delwin(endWin);
    endwin();
    return 0;
}
