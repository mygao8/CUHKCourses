#include "myftp.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

unsigned char buff[MAXLEN];

void setFDs(int *sd, int numServer, fd_set fds){
    for (int i = 0; i < numServer; i++){
        if (sd[i] != 0){
            // socket with a successful connect
            FD_SET(sd[i], &fds);            
        }
    }
}

int main(int argc, char **argv){

    int port, numServer, k, blockSize;

    int *sd = (int *)malloc(sizeof(int) * numServer);
    memset(sd, 0, sizeof(int)*numServer);
    struct sockaddr_in *server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in) * numServer);

    unsigned char *message;
    struct message_s headerMsg;
    int sendNum, recvNum;

    fd_set writeFds;
    int maxFd = 0;
    char listBuf[MAXLEN];
    char buf[32];
    memset(buf, '\0', sizeof(buf));
    // read parameters from config.txt
    FILE *fp = fopen("clientconfig.txt", "r");
    if (fp == NULL){
        printf("error reading config\n");
        return -1;
    }
    printf("initial fp:%d\n",fp);
    // read numServer(n)
    if (fgets(buf, sizeof(buf), fp) != NULL){
        numServer = atoi(buf);
    }
    memset(buf, '\0', sizeof(buf));
    // read k
    if (fgets(buf, sizeof(buf), fp) != NULL){
        k = atoi(buf);
    }
    memset(buf, '\0', sizeof(buf));
    // read blockSize
    if (fgets(buf, sizeof(buf), fp) != NULL){
        blockSize = atoi(buf);
    }
    memset(buf, '\0', sizeof(buf));

    int *connectedServer = (int*)malloc(sizeof(int)*numServer);
    memset(connectedServer, 0, sizeof(int)*numServer);

    // initialize and connect each socket to each server
    for (int i = 0; i < numServer; i++)
    {
        printf("i: %d  \n", i);
        memset(buf, '\0', sizeof(buf));
        // read addr and split by ':'
        if (fgets(buf, sizeof(buf), fp) != NULL){
            int j;
            for (j = 0; buf[j] != '\0'; j++)
            {
                if (buf[j] == ':'){
                    buf[j] = '\0';
                    break;
                }
            }
            port = atoi(&buf[j + 1]);
        }
        printf("set socket\n");

        int tmp = socket(AF_INET, SOCK_STREAM, 0);
        sd[i] = tmp;
        // sd[i] = socket(AF_INET, SOCK_STREAM, 0);

        printf("set server_addr\n");
        memset(&server_addr[i], 0, sizeof(server_addr[i]));
        server_addr[i].sin_family = AF_INET;
        server_addr[i].sin_addr.s_addr = inet_addr(buf);
        server_addr[i].sin_port = htons(port);
        socklen_t addrlen = sizeof(server_addr[i]);

        if (connect(sd[i], (struct sockaddr *)&server_addr[i], sizeof(server_addr[i])) < 0)
        {
            printf("connect to server%d error: %s (ERRNO:%d)\n", i, strerror(errno), errno);
            continue;
        }
        else{
            printf("connect to server%d successfully, sd=%d\n", i, sd[i]);
            connectedServer[i]=1;
        }
        // save the largest sd, which will be used in select()
        maxFd = max(maxFd, sd[i]);
        for (int i = 0; i < numServer; i++) {printf("sd[%d]:%d ",i, sd[i]); printf("\n");}
    }
    if (fp == NULL){
        printf("NULL\n");
    }
    else{
        printf("fp=%d\n", fp);
    }
    // fclose(fp);
    // printf("before exit\n");
    // exit(0);

    /*****************************
     * 
     * "list" command
     * 
     * ***************************/
    printf("enter list");
    if(strcmp("list", argv[2]) == 0){
        /****** send header(0XA1) *******/
        headerMsg.length = htonl(headerLen);
        strcpy(headerMsg.protocol, "myftp");
        headerMsg.type = (unsigned char)0xA1;
        message = (unsigned char *)malloc(headerLen);
        memcpy(message, &headerMsg, headerLen);

        /*** send request ***/
        FD_ZERO(&writeFds);
        for (int i = 0; i < numServer; i++){
            if (connectedServer[i] != 0){
                // socket with a successful connect
                printf("sd=%d will be set in writeFds\n",sd[i]);
                FD_SET(sd[i], &writeFds);            
            }
        }

        int sdID; // idx of socket correspoding to server, i.e. sdID=1 is the socket connect to server 1 (may fail)
        int ret = select(maxFd + 1, NULL, &writeFds, NULL, NULL);
        printf("ret of select: %d\n",ret);
        if (ret < 0){
            perror("select error");
            return -1;
        }
        else if (ret == 0){
            printf("select time out\r\n");
        }
        else{
            // check which socket is writable
            for (sdID = 0; sdID < numServer; sdID++){
                if (FD_ISSET(sd[sdID], &writeFds)){
                    // printf("index of fd:%d  socket:%d\n", sdID, sd[sdID]);
                    // i is the fd of writable socket
                    // sdID, the index of fd in writeFds should be the same as socket
                    printf("sdID: %d, sd=%d, enter sendn\n", sdID, sd[sdID]);
                    if((sendNum = (sendn(sd[sdID], message, headerLen))<0)){
                        printf("send to server%d: error: %s (ERRNO:%d)\n", sdID, strerror(errno), errno);
                    }
                    else{
                        // successfully send request to a server
                        break;
                    }
                }
            }
        


            /*** receive message ***/

            /****** receive header(0XA2) *******/
            if((recvNum = recvn(sd[sdID], buff, headerLen)) < 0){
                printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
                exit(0);
            }
            memcpy(&headerMsg, buff, headerLen);

            if(headerMsg.type == (unsigned char)0xA2){
                // write received packages(.metadata) into a tmp file
                FILE *fd = fopen(".tmpForList", "wb");
                // fileSize get
                int fileSize = ntohl(headerMsg.length) - headerLen;
                int remainSize = fileSize;
                int nextSize;
                // recv message in packet(len<=MAXLEN)
                while(remainSize>0){
                    nextSize = min(remainSize, MAXLEN);
                    if((recvNum = recvn(sd[sdID], buff, nextSize)) < 0){
                        printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
                        exit(0);
                    }
                    
                    fwrite(buff, 1, nextSize, fd);
                    remainSize -= nextSize;
                }
                fclose(fd);

                struct metadata *data = (struct metadata *)malloc(sizeof(struct metadata));
                fd = fopen(".tmpForList", "rb");
                remainSize = fileSize;
                printf("remainSize:%d, struct metadata:%d\n",remainSize, sizeof(struct metadata));
                while (remainSize>0){
                    if (fread(data, sizeof(struct metadata), 1, fd) <= 0){
                        printf("fread error\n");
                    }
                    printf("%s size:%d idx:%d\n", data->filename, data->filesize, data->idx);
                    remainSize -= sizeof(struct metadata);
                }
                fclose(fd);

                if(remove(".tmpForList")){
                    printf("Could not delete the file &s \n",".tmpForList");
                }
                else{
                    printf("Delete tmp files for list\n");
                }
            }
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

        /****** recv header(0XB2/0XB3) *******/
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
            // recv message in packet(len<=MAXLEN)
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
