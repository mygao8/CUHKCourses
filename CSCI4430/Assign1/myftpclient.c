#include "myftp.h"

unsigned char buff[MAXLEN];

int main(int argc, char **argv){
    int port = atoi(argv[2]);
    
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(port);
    socklen_t addrlen = sizeof(server_addr);
    if(connect(sd,(struct sockaddr *)&server_addr, sizeof(server_addr))<0){
        printf("connect error: %s (ERRNO:%d)\n",strerror(errno), errno);
        exit(0);
    }

    unsigned char *message;
    struct message_s headerMsg;
    int sendNum;
    int recvNum;

    /*****************************
     * 
     * "list" command
     * 
     * ***************************/
    if(strcmp("list", argv[3]) == 0){
        /****** send header(0XA1) *******/
        headerMsg.length = htonl(10);
        strcpy(headerMsg.protocol, "myftp");
        headerMsg.type = (unsigned char)0xA1;
        message = (unsigned char *)malloc(10);
        memcpy(message, &headerMsg, 10);

        if((sendNum = (sendn(sd, message, 10))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
        }
        /****** receive header(0XA2) *******/
        if((recvNum = recvn(sd, buff, 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        memcpy(&headerMsg, buff, 10);
        // headerMsg.length - 10 is the length for buff (dir info)
        if((recvNum = recvn(sd, buff, headerMsg.length - 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        if(headerMsg.type == (unsigned char)0xA2){
            printf("%s", buff);
        }
        
    }
    /*****************************
     * 
     * "get" command
     * 
     * ***************************/
    else if(strcmp("get", argv[3]) == 0){
        /****** send header(0XB1) *******/
        strcpy(headerMsg.protocol, "myftp");
        headerMsg.type = (unsigned char) 0xB1;
        // get the length of file name
        unsigned int fileNameLen = strlen(argv[4]);
        // messageLen = fileNameLen + 10(header) + 1(\0)
        unsigned int messageLen = fileNameLen + 11;
        headerMsg.length = htonl(10 + fileNameLen + 1);
        message = (unsigned char *)malloc(messageLen);
        // message  = [headerMsg, fileNameLen, '\0]
        memcpy(message, &headerMsg, 10);
        memcpy(message + 10, argv[4], fileNameLen);
        message[messageLen - 1] = '\0';

        if((sendNum = (sendn(sd, message, messageLen))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        /****** send header(0XB2/0XB3) *******/
        if((recvNum = recvn(sd, buff, 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }

        memcpy(&headerMsg, buff, 10);
        if(headerMsg.type == 0xB3){
            // file not exit
            printf("file not exit\n");
            exit(0);
        }else if(headerMsg.type == 0xB2){
            /****** send header(0XFF) *******/
            // get file header -> get file size
            if((recvNum = recvn(sd, buff, 10)) < 0){
                printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
                exit(0);
            }
            memcpy(&headerMsg, buff, 10);
            FILE *fd = fopen(argv[4], "wb");
            // fileSize get
            int fileSize = ntohl(headerMsg.length) - 10;
            int remainSize = fileSize;
            int nextSize;
            // send message in packet(len<=MAXLEN)
            while(remainSize>0){
                nextSize = min(remainSize, MAXLEN);
                if((recvNum = recvn(sd, buff, nextSize)) < 0){
                    exit(0);
                }
                
                fwrite(buff, 1, nextSize, fd);
                remainSize -= nextSize;
            }
            fclose(fd);
        }
    /*****************************
     * 
     * "put" command
     * 
     * ***************************/
    }else if (strcmp("put", argv[3]) == 0){
        if(access(argv[4], F_OK) != 0){
            printf("file doesn't exit\n");
            exit(0);
        }
        /****** send header(0XC1) *******/
        strcpy(headerMsg.protocol, "myftp");
        headerMsg.type = (unsigned char)0xC1;
        int fileNameSize = strlen(argv[4]);
        // messageLen = headerLen(10) + fileNameSize + '\0'(1)
        unsigned int messageLen = 10 + fileNameSize + 1;
        headerMsg.length = htonl(messageLen);
        // message = [header, fileName, '\0']
        message = (unsigned char *)malloc(10+fileNameSize+1);
        memcpy(message, &headerMsg, 10);
        memcpy(message + 10, argv[4], strlen(argv[4]));
        message[messageLen - 1] = '\0';
        
        if((sendNum = (sendn(sd, message, messageLen))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        /****** recv header(0XC2) *******/
        if((recvNum = recvn(sd, buff, 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }

        memcpy(&headerMsg, buff, 10);
        
        if(headerMsg.type == (unsigned char)0xC2){
            //get file size 
            struct stat statbuff;
            int readFileInfoflag;
            if((readFileInfoflag = stat(argv[4], &statbuff))<0){
                printf("error read file info.");
                exit(0);
            }
            int file_size = statbuff.st_size;
            FILE *fd = fopen(argv[4], "rb");

            /****** send header(0XFF) *******/
            strcpy(headerMsg.protocol, "myftp");
            headerMsg.type = 0xFF;
            messageLen = 10 + file_size;
            headerMsg.length = htonl(messageLen);
            memcpy(buff, &headerMsg, 10);
            if((sendNum = (sendn(sd, buff, 10))<0)){
                printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                exit(0);
            }
            // send file in packet(len <= MAXLEN)
            int remainFileLen = file_size;
            int readLen;
            int nextSize = min(MAXLEN, remainFileLen);
            while((readLen = fread(buff, 1, nextSize, fd))>0){
                if((sendNum = (sendn(sd, buff, nextSize))<0)){
                    printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                    exit(0);
                }
                remainFileLen -= nextSize;
                nextSize = min(MAXLEN, remainFileLen);
            }
            fclose(fd);
        }
    }
    close(sd);
    return 0;
}
