// connect.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "../shared/GameParameters.h"
#include <fcntl.h>
#include <pthread.h>

typedef struct {
    int sockfd;
    bool startReceived;
    int playerCount;
    char** playerInfo;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} SharedData;

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

void* listenerThread(void* arg) {
    SharedData* data = (SharedData*)arg;
    char buf[256];
    while (true) {
        int len = read(data->sockfd, buf, sizeof(buf)-1);
        if (len <= 0) break; // 失敗或斷線
        buf[len] = '\0';

        if (strncmp(buf, "start,", 6) == 0) {
            pthread_mutex_lock(&data->mutex);
            // 解析玩家數
            char* token = strtok(buf + 6, "\n");
            data->playerCount = atoi(token);
            // 分配 playerInfo
            data->playerInfo = (char**)malloc(data->playerCount * sizeof(char*));
            for (int i = 0; i < data->playerCount; i++) {
                token = strtok(NULL, "\n");
                data->playerInfo[i] = strdup(token);
            }
            data->startReceived = true;
            pthread_cond_signal(&data->cond);
            pthread_mutex_unlock(&data->mutex);
        }
    }
    return NULL;
}

bool waitForStart(SharedData* data) {
    pthread_mutex_lock(&data->mutex);
    while (!data->startReceived) {
        pthread_cond_wait(&data->cond, &data->mutex);
    }
    pthread_mutex_unlock(&data->mutex);
    return true;
}

// leave room "exit,room_id\n"
bool exitRoom(int sockfd, const char* roomId) {
    char msg[32] = {0};
    snprintf(msg, sizeof(msg), "exit,%s\n", roomId);
    if (write(sockfd, msg, strlen(msg)) < 0) {
        return false;
    }
    close(sockfd);
    return true;
}