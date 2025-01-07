---  
title: "UNP Final Project - Tank3r"
mainfont: "Carlito"  
author: "112550202 陳芷桓 112550056 林洹佑"  
CJKmainfont: "Microsoft JhengHei" 
colorlinks: true
toc: true
template: "eisvogel"
listings: true
titlepage: true
titlepage-rule-color: "360049"
#--pdf-engine=xelatex  --from markdown --template eisvogel --listings
---  

# 簡介
<!--
<code style="background-color:powderblue;">：專題題目簡介、成員分工、開發與執行環境、特殊需求等。</code>
-->
## Tank3r 簡介

Tank3r 是一款基於 C++ 的線上 2D 鳥瞰視角坦克遊戲。你必須躲避障礙，衝鋒陷陣，利用子彈斬殺敵方坦克，小心來自對岸的炮彈！無限制開啟房間，支援最多 4 人同房開戰！

## 分工
|組員|學號|負責範圍|
|:---:|:---:|:-------|
|林洹佑|112550056|Server/網路交互設計|
|陳芷桓|112550202|Client/遊戲邏輯設計|

## 開發與執行環境
Arch Linux / macOS / Ubuntu

# 方法與設計
<!--
<code style="background-color:powderblue;">：Server與Client程式個別功能與分工原則、Server與Client程式互動規則與資料傳輸格式、例外狀況之分析與處理等。</code>
-->
因爲是即時線上射擊遊戲(FPS)，client 必須負責部大部分運算以及渲染工作，再經由 GameSync 與 server 交互即時資訊，利用 UDP 以降低延遲。

在遊戲間 Server/Client 端會執行的邏輯有：
- server:
    - 坦克移動
    - 子彈命中
    - 玩家退出
    - 接收與傳送資訊給 client
- client:
    - 子彈碰撞
    - 畫面渲染
    - 遊戲狀態更新
    - 接收與傳送資訊給 server

## Server
- 使用 `poll.h` 做 multiplexing 來同時和多個 client 用TCP進行溝通
- 使用 `pthread.h` 開出 listen 遊戲操作的 thread
- 使用 `pthread.h` 為每個 room 開出一個 thread 來執行game loop
- 使用 `queue`, `stringstream` 來作為輸入 buffer 並在 threads 之間共享資料

### 資料傳輸格式
#### 連線及房間系統
`client_id`和`room_id` 為4位數數字
- **client 指令**
    - "`client_id`,`user_name`\\n"\
    設定 `cliend_id` 的 `user_name`
    - "join,`room_id`\\n"\
    請求加入 `room_id` 的房間，若 `room_id` 為 -1 則隨機加入
    - "exit,`room_id`\\n"\
    離開房間
    - "start,`room_id`\\n"\
    房主可以請求開始遊戲

- **server 回覆**
    - "`client_id`\\n"\
    告知 client 的`client_id`
    - "join,`room_id`\\n"\
    告知 client 成功加入 `room_id`
    - "fail\\n"\
    告知 start 或 join 操作失敗
    - "host,`room_id`\n"
    告知此 client 為房主
    - "start,`player_count`\\n`client_id`,`user_name`\\n...seed,`seed_number`\n"
    開始遊戲的必要資訊

#### 遊戲通訊
- **遊戲操作**
    - "`key`,`client_id`,`seq`"\
    `key` 是按鍵，只會有 { 'w', 'a', 's', 'd', ' '}\
    `seq` 是操作的 sequence number ，用於 client side prediction
- **遊戲更新**
    - "u,`client_id`,`x`,`y`,`direction`,`seq`"\
    更新坦克座標
    - "f,`client_id`,`x`,`y`,`direction`,`seq`"\
    坦克開火
    - "h,`client_id`,`health`"\
    更新坦克血量

### 互動規則
- 建立連線
    1. client 由 TCP 連線到 server
    2. server 傳送 "`client_id`\\n"，
    3. client 傳送 "`client_id`,`user_name`\\n"
    4. client 傳送 "join,`room_id`\\n"
    5. server 回傳 "fail\n" 代表加入失敗，"join,`room_id`\\n"代表加入成功
- 在房間中
    - client 收到 "host,`room_id`\\n" 代表此 client 成為房主
    - client 送出 "exit,`room_id`\\n" 以退出房間， server 會通知新成為房主的 client
    - client 斷線則等同於退出房間
    - 房主送出 "start,`room_id`\\n"，若房間至少有兩個人就會開始遊戲
- 開始遊戲
    1. server 送出 "start,`player_count`\\n`client_id`,`user_name`\\n...seed,`seed_number`\n"
    2. server 和 client 間開始用 UDP 同步遊戲資訊
    3. client 血量歸 0 後切斷 TCP 連線
    4. 若 client 斷線則視為退出遊戲，血量歸 0
    5. 最後存活者勝利並且斷開 TCP 連線

## Client

遵循 OOP 與 functional programming 的原則，以 stateful 的流程設計各個畫面與遊戲狀態轉移增加日後擴展性的方便度與減輕維護的難易度。

### 使用的特殊套件

- ncursesw (wide character version): 
    - 利用多個 Windows 更新達到視窗切換與視窗管理的功能。
    - 利用 `mvadd` 系列函式將不同字元放置在特定位置，再利用 `wrefresh` 與 `class GameTimer` 對時，同時刷新所有物件以避免畫面撕裂。
    - 利用 `wattron` 改變字體屬性（顏色，閃爍效果，粗體等）增加畫面豐富度。

### 遊戲核心邏輯

依靠 main 中的 state machine 維護狀態轉移，在 RoomMenu 過後將主控權轉交到 RoomMenu 中的 sub-state machine 並在 GameLoop 之後返還控制權

### 資料同步

實作 `GameSync` class

- 將遊戲操作送給 server
- 接收 server 的更新
- 控制遊戲物件
- 實作 client side prediction

# 成果

<!--
<code style="background-color:powderblue;">：最後成果的主要功能與特色。</code>
-->

## Server

### 特色

- 使用一個 UDP port 和所有 client 通訊
- 採用 composite 的方式，將不同功能分離

### 實作細節

#### ClientManager.hpp/cpp
定義 `ClientData` class\
以 `client_id` 管理不同 client 的狀態

#### RoomManager.hpp/cpp
定義 `RoomData` class\
以 `room_id` 管理不同 room 的狀態

#### InputStruct.hpp
定義 `InputStruct` ，作為儲存於 buffer 中的類別\
以及解析字串用的 constructor

#### UpdateStruct.hpp
定義 `UpdateStruct` ，作為儲存於 buffer 中的類別\
以及解析字串用的 constructor

#### GameServer.hpp/cpp
負責管理連線、解析指令、發送指令、\
呼叫 `ClientData` 和 `RoomData` 對應的 member function、執行 game loop\
運用 threading，將 UDP listen 和 game loop 分到其他的 thread\
使得以上操作可以同時進行

#### GameSync.hpp
在 client 端使用\
建立一個 thread 來接收 server 傳送的遊戲更新\
**member functions**

- `send_input` \
將 input 存到 input_buffer 和送往 server

- `update_tank`\
此 function 負責執行 client side prediction\
從 update_buffer 抓資料並將 input_buffer 內\
seq number <= 最新 update  seq number 的 input 清除\
由最新的 update 和 input_buffer 的內容算出 local player tank 的位置\
其他非本機玩家的位置則直接由 update 決定\
這個 function 也會更新玩家的其他資料

- `start_inGame_list`\
創造一個 thread 去接收 server 傳送的 update，\
並將資料存進 update_buffer

## Client

### 特色

- 依視窗與狀態劃分函式
- 使用 Unicode characters 搭配 ncursesw 進階字元控制豐富畫面效果
- 利用種子碼生成隨機地圖 pass map by seed 避免 bulky data transfer 

### 實作細節

#### main.cpp

1. 狀態驅動的流程設計，按順序切換模組。
2. 使用 switch 高效處理多種遊戲狀態。
3. 將視窗、邏輯、和伺服器通訊分離，確保程式結構清晰且易於維護。

#### GameOjbect.h

1. 以 GameObject 為基底，坦克 (Tank)、子彈 (Bullet)、地圖物件 (MapObject) 繼承並特化，實現清晰的遊戲元件結構與功能分離。
2. 坦克 (Tank)、子彈 (Bullet)、地圖物件 (MapObject) 遊戲時鐘等物件特性與交互實作。
3. 利用 constructor 快速將玩家身份格式化為坦克群。

#### map_generator.cpp

1. 核心邏輯：
    - placeDenseClusters：放置稠密的牆壁或水域
    - connectRandomPoints：隨機連接地圖中的點，生成走廊
    - connectCorners：確保地圖的四個角落相連
    - drawWaterBorder：繪製水域邊界
    - connectLargeHolesToCorners：將大區域空洞與角落相連
    - cleanLonelyObjects：清理孤立的小障礙物
    - fillHolesWithWalls：將非連通的空洞填充為牆壁
2. 特色：
    - 高度可自定性，所有函式皆有多個參數可以設定
    - 多項防堵措施，避免生成玩家無法接觸之地圖

#### GameParameters.h

常用遊戲固定參數與類別定義

#### connect.cpp

利用 C Library 的網路函式庫與達成 client 對 server 的連線與訊號傳送。
 
# 結論

在專題的初期，我們構想利用 UDP 來製作一個 real-time 坦克遊戲，real-time 這個特性導致我們實作上面臨巨大的挑戰。使用 UDP 處理連線就不再像是 TCP 一般的單純，需要自己處理連線與掉包，並且深入實作時我們發現伺服器的多執行緒環境讓我們的程式處理變得極為複雜。遊戲伺服器需要即時處理多個玩家的輸入並同步更新遊戲畫面，而這些操作必須處理得非常精確，否則可能會出現資料丟失或遊戲狀態錯亂。由於需要實現即時對戰功能，我們使用 TCP 來確保前期資料傳送的可靠性，在遊戲間則使用 UDP 來提高遊戲的反應速度。在這個階段，我們花了不少時間在同步 Server/Client 之間的資訊，考慮到若將子彈運算完全交由 Server 運算會讓 Client/Server資訊同步變得很難處理，我們參考大多數線上即時對戰遊戲的 Server/Client 分工，將子彈運算完全交由 Client 渲染，只將坦克狀態與位置與 Server 同步，最終達到滿意的延遲效果。

Client 能夠改進的地方

- 更多使用者交互：增加遊戲內經驗或是商城系統，並且建立程式外的資料庫來保存玩家資訊與狀態。
- 用戶界面提升： Client 在遊戲畫面顯示、操作流暢度以及反應速度方面依然有很大的進步空間，雖然有使用 ncurses 來做進階畫面控制，但是最小單位還是被局限在一個字元的大小，可以使用 SFML 或其他引擎來實作渲染。
- 因為時間沒有掌握好，Client 端的源代碼都沒有遵守定義實作分離的規定，可以再將檔案切分開來，以利於未來的擴展性與維護性。

Server 能夠改進的地方

- 連線加密 : 明文傳輸，意味著此應用程式的安全性極差，可以使用類似。
- 錯誤處理 : 在錯誤處理方面沒有多做，某些錯誤會導致 server 程式直接退出。
- 連線逾時 : 可能會有 client 連線後不加入房間或不開始遊戲，server 應該使用timer，並將逾時的 client 踢除。
<!--
<code style="background-color:powderblue;">：專題製作心得、遭遇困難及解決經過、成果未來改進或延伸方向等。</code>
-->

# 文獻與附錄
<!--
<code style="background-color:powderblue;">：參考文獻列表、原始程式碼等。</code>
-->
## 附錄
[source code](https://github.com/nototria/Tank3r)

## 文獻
[client-server-game-architecture](https://www.gabrielgambetta.com/client-server-game-architecture.html)
[ncurses](https://shengyu7697.github.io/cpp-ncurses/)

--------------------
