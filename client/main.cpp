#include <ncurses.h>
#include "../shared/GameObject.h"
#include "../shared/map_generator.cpp"

void renderTank(WINDOW* win, Tank& tank) {
    int x = tank.getX();
    int y = tank.getY();
    int color = tank.getColor();
    wchar_t directionSymbol = tank.getDirectionSymbol();
    cchar_t cc; setcchar(&cc, &directionSymbol, A_NORMAL, 0, NULL);

    wattron(win, COLOR_PAIR(color)); 
    mvwadd_wch(win, y, x, &cc); //tank body
    for (const auto& bullet : tank.getBullets()) { // bullets
        mvwaddch(win, bullet.getY(), bullet.getX(), bullet.getSymbol());
    }
    wattroff(win, COLOR_PAIR(color));
}

void renderStaticObjects(WINDOW* win, const std::vector<MapObject>& objects) {
    for (const auto& obj : objects) {
        mvwaddch(win, obj.getY(), obj.getX(), obj.getSymbol());
    }
}

void renderPlayerInfo(WINDOW* win, const std::string& playerName, int playerHP, int color) {
    werase(win);  // Clear the window

    // Display player name
    mvwprintw(win, 2, 2, "Name: %s", playerName.c_str());

    // Display HP value
    mvwprintw(win, 5, 2, "HP: %d", playerHP);

    // Display HP bar using wide characters
    wattron(win, COLOR_PAIR(color));
    box(win, 0, 0);  // Draw the border
    std::wstring hpBar(playerHP, L'█');  // Each █ represents 2 HP
    mvwaddwstr(win, 7, 3, hpBar.c_str());    // Add wide string to window
    wattroff(win, COLOR_PAIR(color));
    wrefresh(win);  // Refresh the window to display the content
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

void drawCustomBorder(WINDOW* win) {
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
    int ch, cursor_position = 1;
    curs_set(1);
    echo();
    keypad(win, TRUE);
    while (true) {
        int inputX = (maxX - username.length())/2;
        ch = wgetch(win);
        if (ch == '\n') {
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127) {
            if (!username.empty() && cursor_position > 1) {
                username.erase(cursor_position - 2, 1);
                cursor_position--;
            }
        } else if (ch == KEY_LEFT || ch == KEY_RIGHT) {
            switch (ch) {
                case KEY_LEFT:
                    if (cursor_position > 1) cursor_position--;
                    break;
                case KEY_RIGHT:
                    if (cursor_position <= (int)username.length()) cursor_position++;
                    break;
            }
        } else if (ch >= 32 && ch <= 126 && username.length() < 20) {
            username.insert(cursor_position - 1, 1, ch);
            cursor_position++;
        }
        drawUsernameInput(win, username);
        wmove(win, 22, (maxX - username.length() % 2 == 0) ? (maxX - username.length())/2 : (maxX - username.length())/2 - 1 + cursor_position - 1);
        wrefresh(win);
    }
    curs_set(0);
    noecho();
}

void gameLoop(WINDOW* gridWin, int gridWidth, int gridHeight, std::vector<MapObject>& staticObjects, GameState& state, const std::string& username, int playerNum) {
    // status windows
    const int statusBlockWidth = 25;
    const int statusBlockHeight = 10; 
    int startX = (COLS - gridWidth) / 2, startY = (LINES - gridHeight) / 2;
    WINDOW* StatusWin[playerNum];
    StatusWin[0] = newwin(statusBlockHeight, statusBlockWidth, startY, startX - statusBlockWidth);
    if (playerNum >= 2) {
        StatusWin[1] = newwin(statusBlockHeight, statusBlockWidth, startY, startX + gridWidth);
    }
    if (playerNum >= 3) {
        StatusWin[2] = newwin(statusBlockHeight, statusBlockWidth, startY + statusBlockHeight, startX - statusBlockWidth);
    }
    if (playerNum >= 4) {
        StatusWin[3] = newwin(statusBlockHeight, statusBlockWidth, startY + statusBlockHeight, startX + gridWidth);
    }
    // default values set
    keypad(gridWin, TRUE);
    nodelay(gridWin, TRUE);
    drawCustomBorder(gridWin);
    GameTimer timer(0.016); // 60 FPS
    bool loopRunning = true;

    //remote Tank
    std::string PlayerNames[playerNum];
    PlayerNames[0] = username;
    PlayerNames[1] = "Player2";
    PlayerNames[2] = "Player3";
    PlayerNames[3] = "Player4";
    std::vector<Tank> tanks = Tank::createTanks(playerNum, PlayerNames, gridWidth, gridHeight);
    for(int i = 0; i < playerNum; i++){
        tanks[i].setColor(i+1);
    }
    Tank &myTank = tanks[0];
    for(int i = 0; i < playerNum; i++){
        renderTank(gridWin, tanks[i]);
    }

    // game loop
    while (loopRunning) {
        int ch = wgetch(gridWin);
        switch (ch) {
            case KEY_UP: {
                myTank.setDirection(Direction::Up);
                int nextY = myTank.getY() - 1;
                if (nextY > 0 && !myTank.checkTankCollision(myTank.getX(), nextY, staticObjects)) {
                    myTank.setY(nextY);
                }
                break;
            }
            case KEY_DOWN: {
                myTank.setDirection(Direction::Down);
                int nextY = myTank.getY() + 1;
                if (nextY < gridHeight -1 && !myTank.checkTankCollision(myTank.getX(), nextY, staticObjects)) {
                    myTank.setY(nextY);
                }
                break;
            }
            case KEY_LEFT: {
                myTank.setDirection(Direction::Left);
                int nextX = myTank.getX() - 1;
                if (nextX > 0 && !myTank.checkTankCollision(nextX, myTank.getY(), staticObjects)) {
                    myTank.setX(nextX);
                }
                break;
            }
            case KEY_RIGHT: {
                myTank.setDirection(Direction::Right);
                int nextX = myTank.getX() + 1;
                if (nextX < gridWidth -1 && !myTank.checkTankCollision(nextX, myTank.getY(), staticObjects)) {
                    myTank.setX(nextX);
                }
                break;
            }
            case 'f': {
                myTank.fireBullet();
                break;
            }
            case 'q': {
                state = GameState::EndScreen;
                loopRunning = false;
                break;
            }
        }
        if (timer.shouldUpdate()) {
            werase(gridWin);
            renderStaticObjects(gridWin, staticObjects);
            setlocale(LC_ALL, "");
            drawCustomBorder(gridWin);

            for(int i = 0; i < playerNum; i++){
                tanks[i].updateBullets(gridWidth, gridHeight, staticObjects);
                renderTank(gridWin, tanks[i]);
                renderPlayerInfo(StatusWin[i], tanks[i].getName(), tanks[i].getHP(), tanks[i].getColor());
                wrefresh(StatusWin[i]);
            }
            wrefresh(gridWin);
        }
    }

    // Cleanup
    for(int i = 0; i < playerNum; i++){
        werase(StatusWin[i]);
        wrefresh(StatusWin[i]);
        delwin(StatusWin[i]);
    }
}

void drawEndScreen(WINDOW* win) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 5, 10, "You WIN");
    mvwprintw(win, 7, 10, "Press 'r' to restart");
    wrefresh(win);
}

int main() {
    initscr();
    noecho();
    cbreak();
    start_color();
    curs_set(0);
    setlocale(LC_ALL, "");
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);

    int playerNum = 4;
    int width = 100, height = 40;
    int startX = (COLS - width) / 2, startY = (LINES - height) / 2;
    WINDOW* titleWin = newwin(height, width, startY, startX);
    WINDOW* inputWin = newwin(height, width, startY, startX);
    WINDOW* gameWin = newwin(height, width, startY, startX);
    WINDOW* endWin = newwin(height, width, startY, startX);
    GameState state = GameState::TitleScreen;
    std::string username = "";

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
                state = GameState::GameLoop;
                break;
            }

            case GameState::GameLoop: {
                std::vector<MapObject> staticObjects = generateStaticObjects(width, height, 100, 100);
                gameLoop(gameWin, width, height, staticObjects, state, username, playerNum);
                state = GameState::EndScreen;
                break;
            }

            case GameState::EndScreen: {
                drawEndScreen(endWin);
                int ch = wgetch(endWin);
                if (ch == 'r' || ch == 'R') {
                    state = GameState::UsernameInput;
                }
                break;
            }
        }
    }

    delwin(titleWin);
    delwin(inputWin);
    delwin(gameWin);
    delwin(endWin);
    endwin();
    return 0;
}