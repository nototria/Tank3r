// connect.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "../shared/GameParameters.h"
#include <fcntl.h>

void suppress_stderr() {
    int fd = open("/dev/null", O_WRONLY);
    if (fd != -1) {
        dup2(fd, STDERR_FILENO);  // Redirect stderr to /dev/null
        close(fd);
    }
}

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

int receiveClientId(int sockfd) {
    char buf[16] = {0};
    int len = read(sockfd, buf, sizeof(buf) - 1);
    if (len <= 0) {
        return -1; // 伺服器關閉或讀取失敗
    }
    buf[len] = '\0';
    return atoi(buf);
}

bool sendUserName(int sockfd, int clientId, const char* userName) {
    if (strchr(userName, ',') != NULL) return false; // 不允許逗號

    char msg[128] = {0};
    snprintf(msg, sizeof(msg), "%04d,%s\n", clientId, userName);
    if (write(sockfd, msg, strlen(msg)) < 0) {
        return false;
    }
    return true;
}

char* joinRoom(int sockfd, const char* roomId) {
    char msg[32] = {0};
    if (strcmp(roomId, "-1") == 0) {
        snprintf(msg, sizeof(msg), "join,-1\n");
    } else {
        snprintf(msg, sizeof(msg), "join,%s\n", roomId);
    }
    if (write(sockfd, msg, strlen(msg)) < 0) return NULL;

    char buf[64] = {0};
    int len = read(sockfd, buf, sizeof(buf) - 1);
    buf[len] = '\0';
    // 伺服器回傳 "join,room_id\n" or "fail\n"
    if (strncmp(buf, "join,", 5) == 0) {
        return buf + 5;
    }
    return NULL;
}

bool startGame(int sockfd, const char* roomId) {
    char msg[32] = {0};
    snprintf(msg, sizeof(msg), "start,%s\n", roomId);
    return (write(sockfd, msg, strlen(msg)) > 0);
}

// 離開房間 "exit\n"
bool exitRoom(int sockfd) {
    return (write(sockfd, "exit\n", 5) > 0);
}