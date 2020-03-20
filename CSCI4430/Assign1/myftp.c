#include "myftp.h"
int sendn(int sd, void *buf, int buf_len)
{

    int n_left = buf_len;
    int n;
    while(n_left > 0){
        if((n = send(sd, buf+(buf_len-n_left), n_left, 0))<0){
            if(errno == EINTR){
                n = 0;
            }else{
                return -1;
            }
        }else if(n == 0){
            return 0;
        }
        n_left -= n;
    }
    return buf_len;
}
int recvn(int sd, void *buf, int buf_len){
    int n_left = buf_len;
    int n;
    while(n_left > 0){
        if((n = recv(sd, buf+(buf_len - n_left), n_left, 0))<0){
            if(errno == EINTR)
                n = 0;
            else
                return -1;
        }else if(n  == 0){
            return 0;
        }
        n_left -= n;
    }
    return buf_len;
}

void *threadFun(void *arg){
    // handle arg
    unsigned char buff[MAXLEN];
    struct _threadParam threadParam;
    memcpy(&threadParam, (struct _threadParam *)arg, sizeof(threadParam));
    int client_sd = threadParam.client_sd;
    int n;
    if((n = recvn(client_sd, buff, 10)) < 0){
        printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
        exit(0);
    }
    buff[n] = '\0';
    int i;

    struct message_s header_msg;
    memcpy(&header_msg, buff, 10);
    int sendNum, recvNum;
    
    if(header_msg.type == (unsigned char)0xA1){
        DIR *dir;
        struct dirent *ptr_dir;
        dir = opendir("data/");
        int fileListSize = 1;
        while((ptr_dir = readdir(dir))!=NULL){
            fileListSize += strlen(ptr_dir->d_name);
            fileListSize += 1;
        }
        closedir(dir);
        char *fileList = (char *)malloc(fileListSize);
        dir = opendir("data/");
        int fileListStrPtr = 0;
        while((ptr_dir = readdir(dir))!=NULL){
            memcpy(fileList + fileListStrPtr, ptr_dir->d_name, strlen(ptr_dir->d_name));
            fileListStrPtr += strlen(ptr_dir->d_name);
            fileList[fileListStrPtr] = '\n';
            fileListStrPtr ++;
        }
        fileList[fileListSize - 1] = '\0';
        struct message_s header_msg_response;
        strcpy(header_msg_response.protocol, "myftp");
        header_msg_response.type = (unsigned char)0xA2;
        unsigned int len_header_msg_response = 10+strlen(fileList)+1;
        header_msg_response.length = htonl(len_header_msg_response);
        unsigned char *sendString = (unsigned char *)malloc(len_header_msg_response);
        memcpy(sendString, &header_msg_response, 10);
        memcpy(sendString+10, fileList, strlen(fileList));
        sendString[len_header_msg_response - 1] = '\0';
        if((sendNum = (sendn(client_sd, sendString, len_header_msg_response))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
        }
        
    }else if(header_msg.type == (unsigned char)0xB1){

        int fileNameSize = ntohl(header_msg.length) - 10;
        unsigned char *sendFileName = (unsigned char *)malloc(fileNameSize);

        if((n = recvn(client_sd, buff, fileNameSize)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        memcpy(sendFileName, buff, fileNameSize);
        char *sendFileDir = (char *)malloc(fileNameSize + 5+1);
        strcpy(sendFileDir, "data/");
        strcpy(sendFileDir+5, sendFileName);
        sendFileDir[fileNameSize+6] = '\0';
        int flagExitFile = 0;
        if(access(sendFileDir, F_OK) != 0){
            flagExitFile = 0;
        }else{
            flagExitFile = 1;
        }
    
        struct message_s header_msg_response;
        strcpy(header_msg_response.protocol, "myftp");
        header_msg_response.length = htonl(10);
        
        if(flagExitFile){
            header_msg_response.type = (unsigned char)0xB2;
        }else{
            header_msg_response.type = (unsigned char)0xB3;
        }
        unsigned char *sendString = (unsigned char *)malloc(10);
        memcpy(sendString, &header_msg_response, 10);
        if((sendNum = (sendn(client_sd, sendString, 10))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
        }
        if(flagExitFile){
            FILE *fd = fopen(sendFileDir, "rb");
            strcpy(header_msg.protocol, "myftp");
            header_msg.type = 0xFF;
            struct stat statbuff;
            int readFileInfoflag;
            if((readFileInfoflag = stat(sendFileDir, &statbuff))<0){
                printf("error read file info.");
                exit(0);
            }
            int file_size = statbuff.st_size;
            int len_header_msg = 10 + file_size;
            header_msg.length = htonl(len_header_msg);
            memcpy(buff, &header_msg, 10);
            int remainFileLen = file_size;
            int readLen;
            if((sendNum = sendn(client_sd, buff, 10))<0){
                printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                exit(0);
            }
            int nextSize=  min(MAXLEN, remainFileLen);
            while((readLen = fread(buff, 1, nextSize, fd))>0){
                nextSize=  min(MAXLEN, remainFileLen);
                if((sendNum = (sendn(client_sd, buff, nextSize))<0)){
                    printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                    exit(0);
                }
                remainFileLen -= nextSize;
            }
            fclose(fd);
        }

    }else if(header_msg.type == (unsigned char)0xC1){
        int fileNameSize = ntohl(header_msg.length) - 10;
        unsigned char *recvFileName = (unsigned char *)malloc(fileNameSize);

        if((n = recvn(client_sd, buff, fileNameSize)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        memcpy(recvFileName, buff, fileNameSize);
        struct message_s header_msg_response;
        strcpy(header_msg_response.protocol, "myftp");
        header_msg_response.type = (unsigned char)0xC2;
        header_msg_response.length = htonl(10);
        
        unsigned char *sendString = (unsigned char *)malloc(10);
        memcpy(sendString, &header_msg_response, 10);
        if((sendNum = (sendn(client_sd, sendString, 10))<0)){
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
        }
        if((recvNum = recvn(client_sd, buff, 10)) < 0){
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(0);
        }
        memcpy(&header_msg, buff, 10);
        if(header_msg.type == (unsigned char)0xFF){
            char *recvFileDir = (char *)malloc(fileNameSize + 5+1);
            strcpy(recvFileDir, "data/");
            strcpy(recvFileDir+5, recvFileName);
            recvFileDir[fileNameSize+6] = '\0';
            
            FILE *fd = fopen(recvFileDir, "wb");
            int fileSize = ntohl(header_msg.length) - 10;
            int remainSize = fileSize;
            int nextSize;
            while(remainSize>0){
                nextSize = min(remainSize, MAXLEN);
                if((recvNum = recvn(client_sd, buff, nextSize)) < 0){
                    printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
                    exit(0);
                }
                fwrite(buff, 1, nextSize, fd);
                remainSize -= nextSize;
            }
            fclose(fd);
        }
    }
    close(client_sd);
    int threadIdx = threadParam.threadClientIdx;
    //printf("finish idx:%d\n", threadIdx);
    fflush(stdout);
    threadClient[threadIdx].available = 1;
}
