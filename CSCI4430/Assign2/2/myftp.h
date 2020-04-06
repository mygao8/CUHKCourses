#ifndef _MYFTP_
#define _MYFTP_

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
#include<isa-l.h>
#include<sys/select.h>
#include<sys/types.h>
#include<math.h>


#define MAXJOIN 1024
#define min(x,y) ((x) < (y)?(x):(y))
#define max(x,y) ((x) > (y)?(x):(y))
#define HEADERLEN 11
#define headerLen 11
#define FILEINFOSIZE 1029
#define MAXLEN 1024

struct fileInfo_s{
    unsigned char fileName[1024];
    unsigned int fileSize;
    unsigned char idx;
} __attribute__((packed));

struct metadata{
    unsigned char filename[1024];
    unsigned int filesize;
    unsigned char idx;
} __attribute__((packed));

struct message_s {
    unsigned char protocol[5];
    unsigned char type;
    unsigned int length;
    unsigned char idx;
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
typedef struct stripe{
    int sid;
    unsigned char **data_block;
    unsigned char **parity_block;
}Stripe;

struct timeval timeout;
fd_set request_fds;
fd_set reply_fds;

struct _threadClient threadClient[MAXJOIN];

int sendn(int sd, void *buf, int buf_len);
int recvn(int sd, void *buf, int buf_len);
void *threadFun(void *arg);
uint8_t* decodeData(int n, int k, int *workNodes, unsigned char **data, unsigned char *result);

int N, K, blockSize, portNumber;
uint8_t *encodeMatrix, *errorMatrix, *invertMatrix;

unsigned char statusTable[5];
unsigned int remainFileSizeTable[5];
unsigned int paddingSizeTable[5];
unsigned int idxTable[5];
char fileNameTable[5][260];
uint8_t *decodeBlock[5];
FILE *decodeFpTable[5];
#endif