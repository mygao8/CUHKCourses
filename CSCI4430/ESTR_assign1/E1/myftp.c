#include "myftp.h"

// set the certificate and key file name
#define RSA_SERVER_CERT     "cert.pem"
#define RSA_SERVER_KEY      "key.pem"

/*
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
*/



void *threadFun(void *arg){
    // handle arg
    unsigned char buff[MAXLEN];
    struct _threadParam threadParam;
    memcpy(&threadParam, (struct _threadParam *)arg, sizeof(threadParam));
    int client_sd = threadParam.client_sd;
    int n;

    // set up SSL variables
    int err;
    SSL_CTX         *ctx;
    SSL            *ssl;
    SSL_METHOD      *meth;
    X509            *client_cert = NULL;

    /*----------------------------------------------------------------*/
    /* Register all algorithms */
    OpenSSL_add_all_algorithms();

    /* Load encryption & hashing algorithms for the SSL program */
    SSL_library_init();

    /* Load the error strings for SSL & CRYPTO APIs */
    SSL_load_error_strings();

    /* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
    meth = (SSL_METHOD*)TLSv1_method();

    /* Create a SSL_CTX structure */
    ctx = SSL_CTX_new(meth);

    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    /* Load the server certificate into the SSL_CTX structure */
    if (SSL_CTX_use_certificate_file(ctx, RSA_SERVER_CERT, SSL_FILETYPE_PEM)
            <= 0) {
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    /* set password for the private key file. Use this statement carefully */
    SSL_CTX_set_default_passwd_cb_userdata(ctx, (char*)"4430");


    /* Load the private-key corresponding to the server certificate */
    if (SSL_CTX_use_PrivateKey_file(ctx, RSA_SERVER_KEY, SSL_FILETYPE_PEM) <=
            0) { 
        ERR_print_errors_fp(stderr);
        exit(1);
    }

    /* Check if the server certificate and private-key matches */
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr,
                "Private key does not match the certificate public key\n");
        exit(1);
    }


    /* ----------------------------------------------- */
    /* TCP connection is ready. */
    /* A SSL structure is created */
    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        fprintf(stderr, "ERR: unable to create the ssl structure\n");
        exit(-1);
    }

    /* Assign the socket into the SSL structure (SSL and socket without BIO) */
    SSL_set_fd(ssl, client_sd);

    /* Perform SSL Handshake on the SSL server */
    err = SSL_accept(ssl);
    if (err == -1) {
        ERR_print_errors_fp(stderr); 
        exit(1);
    }


    // read header of 10 bytes
    if((n = SSL_read(ssl, buff, 10)) < 0){
        ERR_print_errors_fp(stderr); 
        exit(1);
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
        if((sendNum = (SSL_write(ssl, sendString, len_header_msg_response))<0)){
            ERR_print_errors_fp(stderr); 
            exit(1);
        }
        
    }else if(header_msg.type == (unsigned char)0xB1){

        int fileNameSize = ntohl(header_msg.length) - 10;
        unsigned char *sendFileName = (unsigned char *)malloc(fileNameSize);

        if((n = SSL_read(ssl, buff, fileNameSize)) < 0){
            ERR_print_errors_fp(stderr); 
            exit(1);
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
        if((sendNum = (SSL_write(ssl, sendString, 10))<0)){
            ERR_print_errors_fp(stderr); 
            exit(1);
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
            if((sendNum = SSL_write(ssl, buff, 10))<0){
                ERR_print_errors_fp(stderr); 
                exit(1);
            }
            int nextSize=  min(MAXLEN, remainFileLen);
            while((readLen = fread(buff, 1, nextSize, fd))>0){
                nextSize=  min(MAXLEN, remainFileLen);
                if((sendNum = (SSL_write(ssl, buff, nextSize))<0)){
                    ERR_print_errors_fp(stderr); 
                    exit(1);
                }
                remainFileLen -= nextSize;
            }
            fclose(fd);
        }

    }else if(header_msg.type == (unsigned char)0xC1){
        int fileNameSize = ntohl(header_msg.length) - 10;
        unsigned char *recvFileName = (unsigned char *)malloc(fileNameSize);

        if((n = SSL_read(ssl, buff, fileNameSize)) < 0){
            ERR_print_errors_fp(stderr); 
            exit(1);
        }
        memcpy(recvFileName, buff, fileNameSize);
        struct message_s header_msg_response;
        strcpy(header_msg_response.protocol, "myftp");
        header_msg_response.type = (unsigned char)0xC2;
        header_msg_response.length = htonl(10);
        
        unsigned char *sendString = (unsigned char *)malloc(10);
        memcpy(sendString, &header_msg_response, 10);
        if((sendNum = (SSL_write(ssl, sendString, 10))<0)){
            ERR_print_errors_fp(stderr); 
            exit(1);
        }
        if((recvNum = SSL_read(ssl, buff, 10)) < 0){
            ERR_print_errors_fp(stderr); 
            exit(1);
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
                if((recvNum = SSL_read(ssl, buff, nextSize)) < 0){
                    ERR_print_errors_fp(stderr); 
                    exit(1);
                }
                fwrite(buff, 1, nextSize, fd);
                remainSize -= nextSize;
            }
            fclose(fd);
        }
    }

    /*--------------- SSL closure ---------------*/
    /* Shutdown this side (server) of the connection. */
    err = SSL_shutdown(ssl);
    if (err == -1) {
        ERR_print_errors_fp(stderr); 
        exit(1);
    }

    err = close(client_sd);
    if (err == -1) {
        perror("socket_close");
        exit(-1);
    }

    /* Free the SSL structure */
    SSL_free(ssl);
    
    SSL_CTX_free(ctx);

    int threadIdx = threadParam.threadClientIdx;
    //printf("finish idx:%d\n", threadIdx);
    fflush(stdout);
    threadClient[threadIdx].available = 1;
}
