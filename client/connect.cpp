// connect.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include<sstream>
#include "../shared/GameParameters.h"
#include "../shared/GameObject.h"
#include<poll.h>
#include<set>

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


void InRoomListen(int sockfd, bool &isHost, bool &startGame, int &playerNum){
    struct pollfd pollfd_list[1];
    pollfd_list[0].fd = sockfd;
    pollfd_list[0].events = POLLRDNORM;
    char recv_buffer[1024];
    int nready = poll(pollfd_list, 1, 10);
    if (nready <= 0) return;
    //read line
    int len=0;
    for(;len<1024;++len){
        if(read(sockfd,recv_buffer+len,1)<=0) break;
        if(recv_buffer[len]=='\n') break;
    }
    recv_buffer[len]='\0';
    if(strncmp(recv_buffer,"host,",5)==0){
        isHost=true;
    }
    if(strncmp(recv_buffer,"start,",6)==0){
        startGame=true;
        playerNum=atoi(recv_buffer+6);
    }
}

void getStartInfo(const int sockfd, const int playerNum, std::map<int,std::string> &id2Name, int &seed){
    char recv_buffer[1024];
    int len;
    for(int i=0;i<playerNum;++i){
        //read line
        for(len=0;len<1024;++len){
            if(read(sockfd,recv_buffer+len,1)<=0) break;
            if(recv_buffer[len]=='\n') break;
        }
        recv_buffer[len]='\0';

        if(strncmp(recv_buffer,"seed,",5)==0){
            seed=atoi(recv_buffer+5);
        }else{
            int id=atoi(recv_buffer);
            id2Name[id]=recv_buffer+5;
        }
    }
    //read line
    for(len=0;len<1024;++len){
        if(read(sockfd,recv_buffer+len,1)<=0) break;
        if(recv_buffer[len]=='\n') break;
    }
    recv_buffer[len]='\0';
    if(strncmp(recv_buffer,"seed,",5)==0) seed=atoi(recv_buffer+5);
}

int connectUDP(const char* ip, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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
    return sockfd;
}
