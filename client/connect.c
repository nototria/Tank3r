// connect.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

// 建立 TCP 連線並回傳 socket，失敗回傳 -1
int connectToServer(const char* ip, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

// 接收伺服器給定的 client_id (四位數)，伺服器若滿可能關閉連線
int receiveClientId(int sockfd) {
    char buf[16] = {0};
    int len = read(sockfd, buf, sizeof(buf) - 1);
    if (len <= 0) {
        return -1; // 伺服器關閉或讀取失敗
    }
    return atoi(buf);
}

// 傳送 "client_id,user_name" 給伺服器，user_name 不可含 ','
bool sendUserName(int sockfd, int clientId, const char* userName) {
    if (strchr(userName, ',') != NULL) return false; // 不允許逗號

    char msg[128] = {0};
    snprintf(msg, sizeof(msg), "%04d,%s", clientId, userName);
    if (write(sockfd, msg, strlen(msg)) < 0) {
        return false;
    }
    return true;
}

// 加入房間 "join,-1\n" 或 "join,roomId\n"
bool joinRoom(int sockfd, const char* roomId) {
    char msg[32] = {0};
    if (strcmp(roomId, "-1") == 0) {
        snprintf(msg, sizeof(msg), "join,-1\n");
    } else {
        snprintf(msg, sizeof(msg), "join,%s\n", roomId);
    }
    if (write(sockfd, msg, strlen(msg)) < 0) return false;

    char buf[64] = {0};
    int len = read(sockfd, buf, sizeof(buf) - 1);
    if (len <= 0) return false;
    // 伺服器回傳 "join,room_id\n" or "fail\n"
    if (strncmp(buf, "join,", 5) == 0) {
        return true;
    }
    return false;
}

// 主機可使用 "start,room_id\n"
bool startGame(int sockfd, const char* roomId) {
    char msg[32] = {0};
    snprintf(msg, sizeof(msg), "start,%s\n", roomId);
    return (write(sockfd, msg, strlen(msg)) > 0);
}

// 離開房間 "exit\n"
bool exitRoom(int sockfd) {
    return (write(sockfd, "exit\n", 5) > 0);
}