#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<dirent.h>
#include<pthread.h>

#define MAXLEN 1024
#define MAXJOIN 1024
#define min(x,y) ((x) < (y)?(x):(y))


struct message_s {
    unsigned char protocol[5];
    unsigned char type;
    unsigned int length;
} __attribute__((packed));

struct _threadParam{
    int client_sd;
    int threadClientIdx;
};

struct _threadClient{
    struct _threadParam threadParam;
    int available;
    pthread_t thread;
};

struct _threadClient threadClient[MAXJOIN];

int sendn(int sd, void *buf, int buf_len);
int recvn(int sd, void *buf, int buf_len);
void *threadFun(void *arg);
