#include "myftp.h"

struct server_s{
    char addr[16];
    unsigned int port;
};

int main(int argc, char **argv){
    
    

    /*****************************
     * 
     * "list" command
     * 
     * ***************************/
    if(strcmp("list", argv[2]) == 0){
        unsigned char buff[MAXLEN];

        int port, numServer, k, blockSize;

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

        
        int *sd = (int *)malloc(sizeof(int) * numServer);
        memset(sd, 0, sizeof(int)*numServer);
        struct sockaddr_in *server_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in) * numServer);

        int *connectedServer = (int*)malloc(sizeof(int)*numServer);
        memset(connectedServer, 0, sizeof(int)*numServer);


        // initialize and connect each socket to each server
        int noServer = 1;
        for (int i = 0; i < numServer; i++)
        {

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

            int tmp = socket(AF_INET, SOCK_STREAM, 0);
            sd[i] = tmp;
            // sd[i] = socket(AF_INET, SOCK_STREAM, 0);

            memset(&server_addr[i], 0, sizeof(server_addr[i]));
            server_addr[i].sin_family = AF_INET;
            server_addr[i].sin_addr.s_addr = inet_addr(buf);
            server_addr[i].sin_port = htons(port);
            socklen_t addrlen = sizeof(server_addr[i]);

            if (connect(sd[i], (struct sockaddr *)&server_addr[i], sizeof(server_addr[i])) < 0)
            {
                // printf("connect to server%d error: %s (ERRNO:%d)\n", i, strerror(errno), errno);
                continue;
            }
            else{
                connectedServer[i]=1;
                noServer = 0;
            }
            // save the largest sd, which will be used in select()
            maxFd = max(maxFd, sd[i]);
        }

        if (noServer==1){
            printf("ERROR: NO Server\n");
            exit(0);
        }
        /*****************************
         * 
         * "list" command
         * 
         * ***************************/
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
                FD_SET(sd[i], &writeFds);            
            }
        }

        int sdID; // idx of socket correspoding to server, i.e. sdID=1 is the socket connect to server 1 (may fail)
        int ret = select(maxFd + 1, NULL, &writeFds, NULL, NULL);
        if (ret < 0){
            perror("select error");
            return -1;
        }
        else if (ret == 0){

        }
        else{
            // check which socket is writable
            for (sdID = 0; sdID < numServer; sdID++){
                if (FD_ISSET(sd[sdID], &writeFds)){

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

            // set unique .tmpFile{pid} as cache
            char tmpFileName[100] = ".tmpFile";
            tmpFileName[8] = 0;
            sprintf(&tmpFileName[8], "%d", (int)getpid());

            if(headerMsg.type == (unsigned char)0xA2){
                // write received packages(.metadata) into a tmp file
                FILE *fd = fopen(tmpFileName, "wb");
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

                struct metadata *data = (struct metadata *)malloc(1029);
                fd = fopen(tmpFileName, "rb");
                remainSize = fileSize;
                
                while (remainSize>0){
                    if (fread(data, 1029, 1, fd) <= 0){
                        printf("fread error\n");
                    }
		            printf("%s\n",data->filename);
                    remainSize -= 1029;
                }
                fclose(fd);

                if(remove(tmpFileName)){
                    printf("Could not delete the file %s \n", tmpFileName);
                }
            }
        }        
    }
    /*****************************
     * 
     * "get" command
     * 
     * ***************************/
    else if(strcmp("get", argv[2]) == 0){
        // read config.txt
        int i;
        for(i = 0;i < 5;i++){
            statusTable[i] = 0;
            remainFileSizeTable[i] = 0;
            idxTable[i] = 0;
        }
        
        FILE* configure_sd = fopen(argv[1], "rb");
        fscanf(configure_sd, "%d%d%d", &N, &K, &blockSize);
    
        char serverList[5][30];
        int portList[5];
        char address[5][50];
        int fileSize;
        for(i = 0;i < N;i++){
            fscanf(configure_sd, "%s", address[i]);
            char *tok;
            tok = strtok(address[i], ":");
            if(tok != NULL){
                strcpy(address[i], tok);
            }else{
                printf("wrong input\n");
                exit(-1);
            }
            tok = strtok(NULL, ":");
            if(tok != NULL){
                portList[i] = atoi(tok);
            }else{
                printf("wrong address\n");
                exit(-1);
            }
        }
        fclose(configure_sd);



        unsigned char buff[max(MAXLEN, blockSize)];
        int sdTable[5];
        int maxfd = 0;
        int countAvailableServer = 0;
        for(i = 0;i < N;i++) {
            sdTable[i] = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in server_addr;
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = inet_addr(address[i]);
            server_addr.sin_port = htons(portList[i]);
            socklen_t addrlen = sizeof(server_addr);
            if(connect(sdTable[i],(struct sockaddr *)&server_addr, sizeof(server_addr))<0){
                printf("connect error: %s (ERRNO:%d)\n",strerror(errno), errno);
                continue;
            }

            if (sdTable[i] > maxfd){
                maxfd = sdTable[i];
            }
            sdTable[countAvailableServer] = sdTable[i];
            countAvailableServer ++;
            if(countAvailableServer == K){
                break;
            }
        } 
        if(countAvailableServer < K){
            printf("too few available servers\n");
            exit(0);
        }
        
        unsigned char *message;
        struct message_s headerMsg;
        int sendNum;
        int recvNum;


        /****** send header(0XB1) *******/
        strcpy(headerMsg.protocol, "myftp");
        headerMsg.type = (unsigned char) 0xB1;
        // get the length of file name
        unsigned int fileNameLen = strlen(argv[3]);
        // messageLen = fileNameLen + HEADERLEN(header) + 1(\0)
        unsigned int messageLen = fileNameLen + HEADERLEN + 1;
        headerMsg.length = htonl(HEADERLEN + fileNameLen + 1);
        message = (unsigned char *)malloc(messageLen);
        // message  = [headerMsg, fileNameLen, '\0]
        memcpy(message, &headerMsg, HEADERLEN);
        memcpy(message + HEADERLEN, argv[3], fileNameLen);
        message[messageLen - 1] = '\0';

        
        /****** send header(0XB2/0XB3) *******/

        while(1){
            FD_ZERO(&request_fds);
            FD_ZERO(&reply_fds);
            for(i = 0 ;i < K;i++){
                if(statusTable[i] == 0x00){
                    FD_SET(sdTable[i], &request_fds);
                }else{
                    FD_SET(sdTable[i], &reply_fds);
                }
            }
            timeout.tv_sec = 3;
            timeout.tv_usec = 0;
            int flagSelect = select(maxfd + 1, &reply_fds, &request_fds, NULL, &timeout);
            if(flagSelect == -1){
                exit(-1);
            }else if(flagSelect == 0){
                continue;
            }

            for(i = 0;i < K;i++){
                if(FD_ISSET(sdTable[i], &request_fds)){

                    if(statusTable[i] == 0x00){
                        if((sendNum = (sendn(sdTable[i], message, messageLen))<0)){
                            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                            exit(0);
                        }
                        statusTable[i] = 0xB2;
                    }
                }
                if (FD_ISSET(sdTable[i], &reply_fds)){
                    if (statusTable[i] == 0xB2){
                        if((recvNum = recvn(sdTable[i], buff, HEADERLEN)) < 0){
                            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
                            exit(0);
                        }

                        memcpy(&headerMsg, buff, HEADERLEN);
                        if(headerMsg.type == 0xB3){
                            // file not exit
                            printf("file not exit\n");
                            exit(0);
                        }else if(headerMsg.type == 0xB2){
                            statusTable[i] = 0xFF;
                        }
                    }else if(statusTable[i] == 0xFF){
                        /****** recv header(0XFF) *******/
                        // get file header -> get file size
                        if((recvNum = recvn(sdTable[i], buff, HEADERLEN)) < 0){
                            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
                            exit(0);
                        }
                        memcpy(&headerMsg, buff, HEADERLEN);
                        if(headerMsg.type == 0xFF){
                            statusTable[i] = 0xEE;
                            idxTable[i] = headerMsg.idx;
                            // fileSize get
                            fileSize = ntohl(headerMsg.length) - HEADERLEN;
                            int remainSize;
                            int fullStripeNum = floor(fileSize/(K*blockSize));
                            remainSize = fullStripeNum*blockSize;
                            int remainFileSize = fileSize - fullStripeNum*(K*blockSize);
                            int remainBlockNum = ceil((float)remainFileSize/blockSize);
                            if(idxTable[i] >= K){
                                if(remainBlockNum - 1e-9 > 0){
                                    remainSize += blockSize;
                                }
                                paddingSizeTable[i] = 0;
                            }else{
                                if(idxTable[i] == remainBlockNum - 1){
                                    remainSize += remainFileSize % blockSize + remainFileSize / blockSize * blockSize;
                                    paddingSizeTable[i] = blockSize - remainSize;
                                }else if(idxTable[i] < remainBlockNum - 1){
                                    remainSize += blockSize;
                                    paddingSizeTable[i] = 0;
                                }else{
                                    paddingSizeTable[i] = blockSize;
                                }
                            }
                            remainFileSizeTable[i] = remainSize;
                            int nextSize;
                            // recv message in packet(len<=blockSize)
                            nextSize = min(remainSize, blockSize);
                            if((recvNum = recvn(sdTable[i], buff, nextSize)) < 0){
                                printf("recv error\n");
                                exit(0);
                            }
                            memcpy(fileNameTable[i], argv[3], fileNameLen);
                            fileNameTable[i][fileNameLen] = idxTable[i] + '0';
                            fileNameTable[i][fileNameLen + 1] = '\0';
                            FILE* fd = fopen(fileNameTable[i], "wb");
                            fwrite(buff, 1, nextSize, fd);
                            fclose(fd);
                            remainSize -= nextSize;
                            remainFileSizeTable[i] = remainSize;
                        }
                    }else if (statusTable[i] == 0xEE){
                        if(remainFileSizeTable[i] > 0){
                            int remainSize = remainFileSizeTable[i];
                            int nextSize = min(remainSize, blockSize);
                            if((recvNum = recvn(sdTable[i], buff, nextSize)) < 0){
                                printf("recv error\n");
                            }
                            FILE* fd = fopen(fileNameTable[i], "ab");
                            fwrite(buff, 1, nextSize, fd);
                            fclose(fd);
                            remainSize -= nextSize;
                            remainFileSizeTable[i] = remainSize;
                        }else{
                            statusTable[i] = 0xDD;
                            int paddingSize = paddingSizeTable[i];
                            int j;
                            for(j = 0;j < paddingSize;j++){
                                buff[j] = 0;
                            }
                            FILE* fd = fopen(fileNameTable[i], "ab");
                            fwrite(buff, 1, paddingSize, fd);
                            fclose(fd);
                        }
                    }
                }
            }
            int flagAllReceived = 1;
            for (i = 0 ;i < K;i++){
                if(statusTable[i] != 0xDD){
                    flagAllReceived = 0;
                    break;
                }
            }
            if(flagAllReceived == 1){
                break;
            }
        }
        for(i = 0 ;i < K;i++){
            decodeFpTable[i] = fopen(fileNameTable[i], "rb");
        }
        for(i = 0;i < K - 1;i++){
            int j;
            int min = idxTable[i];
            int minIdx = i;
            for(j = i + 1;j < K;j++){
                if(idxTable[j] < min){
                    min = idxTable[j];
                    minIdx = j;
                }
            }
            int tmp = idxTable[i];
            idxTable[i] = idxTable[minIdx];
            idxTable[minIdx] = tmp;
            FILE *tmp2 = decodeFpTable[i];
            decodeFpTable[i] = decodeFpTable[minIdx];
            decodeFpTable[minIdx] = tmp2;
        }

        int endOfFile = 0;
        for(i = 0 ;i < K;i++){
            decodeBlock[i] = malloc(blockSize);
        }
        FILE *fileResult = fopen(argv[3], "wb"); 
        int flagRead = 0;
        for(i = 0 ;i < K;i++){
            memset(decodeBlock[i], 0, blockSize);
            flagRead = fread(decodeBlock[i], 1, blockSize, decodeFpTable[i]);
        }
        int remainDecodeSize = fileSize;
        while(remainDecodeSize > 0){
            unsigned char *decodeResult = (unsigned char *)malloc(blockSize * K);

            decodeData(N, K, idxTable, decodeBlock, decodeResult);
            int j;
            fwrite(decodeResult,1, min(blockSize*K, remainDecodeSize), fileResult);
            for(i = 0 ;i < K;i++){
                memset(decodeBlock[i], 0, blockSize);
                flagRead = fread(decodeBlock[i], 1, blockSize, decodeFpTable[i]);
            }
            remainDecodeSize -= blockSize*K;
        }
        for(i = 0;i < K;i++){
            fclose(decodeFpTable[i]);
        }
        fclose(fileResult);
        for (i = 0;i < K;i++){
            remove(fileNameTable[i]);
        }
        for(i = 0;i < K;i++){
            close(sdTable[i]);
        }

    /*****************************
     * 
     * "put" command
     * 
     * ***************************/
    }else if (strcmp("put", argv[2]) == 0){
        unsigned char buff[MAXLEN];

        int i;
        // read the configure here
        FILE* configure_sd = fopen(argv[1], "rb");
        fscanf(configure_sd, "%d%d%d", &N, &K, &blockSize);
    
        // char serverList[5][30];
        int portList[5];
        char address[5][50];
        // int fileSize;
        for(i = 0;i < N;i++){
            fscanf(configure_sd, "%s", address[i]);
            char *tok;
            tok = strtok(address[i], ":");
            if(tok != NULL){
                strcpy(address[i], tok);
            }else{
                printf("wrong input\n");
                exit(-1);
            }
            tok = strtok(NULL, ":");
            if(tok != NULL){
                portList[i] = atoi(tok);
            }else{
                printf("wrong address\n");
                exit(-1);
            }
        }
        fclose(configure_sd);


        
        int sds[N];

        struct server_s servers[N];

        for(i = 0; i < N; i++ ){
            servers[i].port = portList[i];
            memset(servers[i].addr,0,16);
            memcpy(servers[i].addr, address[i],16);
        }
        // below code for test only
        // servers[0].port = 12345;
        // servers[1].port = 12346;

        // memset(servers[0].addr,0,16);
        // memset(servers[1].addr,0,16);

        // strcpy(servers[0].addr,"127.0.0.1");
        // strcpy(servers[1].addr,"127.0.0.1");


        struct sockaddr_in server_addrs[N];
        for(i = 0; i < N; i++){
            sds[i] = socket(AF_INET, SOCK_STREAM, 0);

            memset(&server_addrs[i], 0, sizeof(struct sockaddr_in));
            server_addrs[i].sin_family = AF_INET;
            server_addrs[i].sin_addr.s_addr = inet_addr(servers[i].addr);
            server_addrs[i].sin_port = htons(servers[i].port);
        }

        for(i = 0; i < N; i++){
            if(connect(sds[i],(struct sockaddr *)&server_addrs[i], sizeof(struct sockaddr_in)) < 0){
                printf("connect error: %s (ERRNO:%d) for server: %s:%d\n",
                    strerror(errno), errno,servers[i].addr,servers[i].port);
                exit(0);
            }
            // else{
            //     printf("Connect to server: %s:%d\n", servers[i].addr,servers[i].port);
            // }
        }


        unsigned char *message;
        struct message_s headerMsg;
        int sendNum;
        int recvNum;


        // may need to change the index for the argv[]
        if(access(argv[3], F_OK) != 0){
            printf("file doesn't exit\n");
            exit(0);
        }

        /****** send header(0XC1) *******/
        strcpy(headerMsg.protocol, "myftp");
        headerMsg.type = (unsigned char)0xC1;
        int fileNameSize = strlen(argv[3]);
        // messageLen = headerLen(11) + fileNameSize + '\0'(1)
        unsigned int messageLen = 11 + fileNameSize + 1;
        headerMsg.length = htonl(messageLen);
        // message = [header, fileName, '\0']
        message = (unsigned char *)malloc(11+fileNameSize+1);
        memcpy(message, &headerMsg, 11);
        memcpy(message + 11, argv[3], strlen(argv[3]));
        message[messageLen - 1] = '\0';
        
        char request_send[N]; 
        /* status for the fd: 
        0 for not sent yet
        1 for request sent, reply not received yet
        2 for reply received
        */
        memset(request_send, 0, N);

        fd_set request_fd;
        fd_set reply_fd;

        int max_fd = 0;
        for(i = 0; i < N; i++){
            if(max_fd < sds[i]){
                max_fd = sds[i];
            }
        }

        int reply_received = 0;
        while(reply_received < N){
            FD_ZERO(&request_fd);
            FD_ZERO(&reply_fd);

            int j;
            for(j = 0; j < N; j++){
                if(request_send[j] == 0){
                    FD_SET(sds[j], &request_fd);
                }
                if(request_send[j] == 1){
                    FD_SET(sds[j], &reply_fd);
                }
            }

            int ret = select(max_fd+1, &reply_fd, &request_fd, NULL, NULL);// no timeout
            if(ret < 0){
                printf("Request and reply select error,ret: %d\n", ret);
            }

            for(j = 0; j < N; j++){
                // can send request
                if(FD_ISSET(sds[j], &request_fd)){ 
                    if((sendNum = (sendn(sds[j], message, messageLen)) < 0)){
                        printf("send error: %s (ERRNO:%d) for server: %s:%d\n",
                            strerror(errno), errno,servers[j].addr,servers[j].port);
                        exit(0);
                     }

                     request_send[j] = 1;
                     break;
                }
                // already send request, can recv reply
                if(FD_ISSET(sds[j], &reply_fd)){
                    if((recvNum = recvn(sds[j], buff, 11)) < 0){
                        printf("send error: %s (ERRNO:%d) for server: %s:%d\n",
                            strerror(errno), errno,servers[j].addr,servers[j].port);
                        exit(0);
                    }
                    memcpy(&headerMsg, buff, 11);

                    if(headerMsg.type != (unsigned char)0xC2){
                        printf("Reply header error, server: %s:%d\n", 
                            servers[j].addr,servers[j].port);
                        exit(0);
                    }
                    request_send[j] = 2;
                    reply_received++;
                    break;
                }
            }
        }


        //get file size 
        struct stat statbuff;
        int readFileInfoflag;
        if((readFileInfoflag = stat(argv[3], &statbuff))<0){
            printf("error read file info.");
            exit(0);
        }
        int file_size = statbuff.st_size;

        // stripe_num = ceiling( filesize / (K * blockSize) )
        int stripe_num = (file_size % (K * blockSize) == 0) ? file_size / (K * blockSize) : file_size / (K * blockSize)+1;
        FILE *fd = fopen(argv[3], "rb");
        char * cacheName = malloc(strlen(argv[3]) + 2);
        cacheName[0] = '.';
        memcpy(cacheName+1, argv[3],strlen(argv[3]));
        cacheName[strlen(argv[3])+1] = '\0';

        FILE *cachefd = fopen(cacheName,"wb");

        uint8_t *encode_matrix = malloc(sizeof(uint8_t) * N * K);
        uint8_t *errors_matrix = malloc(sizeof(uint8_t) * K * K);
        uint8_t *invert_matrix = malloc(sizeof(uint8_t) * K * K);
        uint8_t *table = malloc( sizeof(uint8_t) * 32 * K * (N - K) );

        uint8_t **blocksData = (uint8_t**)malloc(sizeof(uint8_t**) * N); 
        // ptr arrays for the blocks

        gf_gen_rs_matrix(encode_matrix, N, K);
        ec_init_tables(K, N - K, &encode_matrix[K*K], table);

        int block_idx;
        // alloc the data blocks for a stripe
        for(block_idx = 0; block_idx < N; block_idx++){
            blocksData[block_idx] = (uint8_t*)malloc(blockSize * sizeof(uint8_t));
        }

        int stripe_idx;
        int read_size = 0;
        /* ******** 
        for each file, we only generate the encode matrix and the table once 
        and use the matrix and table to encode all the stripes ******* */
        for(stripe_idx = 0; stripe_idx < stripe_num; stripe_idx++){

            for(block_idx = 0; block_idx < K; block_idx++){
                memset(blocksData[block_idx], 0, blockSize);
            }
            
            for(block_idx = 0; block_idx < K; block_idx++){
                int next_read_size = 0;
                if(file_size - read_size == 0){ // all the file get read
                    break;
                }else if(file_size - read_size < blockSize){ // rest within one block
                    next_read_size = file_size - read_size;
                }else{ // rest more than one block
                    next_read_size = blockSize;
                }

                if((fread(blocksData[block_idx], 1, next_read_size, fd)) < 0){
                    printf("File read error\n");
                    exit(0);
                }
                read_size += next_read_size;
            }
            
            ec_encode_data(blockSize, K, N-K, table, blocksData, &blocksData[K]);
            // write the cache file to ".<filename>""
            for(block_idx = 0; block_idx < N; block_idx++){
                if((fwrite(blocksData[block_idx], 1, blockSize, cachefd)) < 0){
                    printf("File write error\n");
                    exit(0);
                }
            }
        }

        fclose(cachefd);


        // read the cache which will be sent to servers
        // read the cache here

        if(access(cacheName, F_OK) != 0){
            printf("Cache file doesn't exit\n");
            exit(0);
        }
        if((readFileInfoflag = stat(cacheName, &statbuff))<0){
            printf("error read cache file info.");
            exit(0);
        }
        int cache_size = statbuff.st_size;
        cachefd = fopen(cacheName, "rb");

        //use a list to store the number of blocks to be sent to servers
        int send_nums[N];
        int server_idx;
        for(server_idx = 0; server_idx < N; server_idx++){
            send_nums[server_idx] = -1;// -1 means the file header is not sent yet
        }


        // find the max_fd
        for(server_idx = 0; server_idx < N; server_idx++){
            if(max_fd < sds[server_idx]){
                max_fd = sds[server_idx];
            }
        }

        fd_set file_fd;
        // the buffer for one block
        unsigned char *block_buf = malloc(sizeof(unsigned char) * blockSize);
        while(1){
            int all_send_flag = 1;

            for(server_idx = 0; server_idx < N; server_idx++){
                if(send_nums[server_idx] < stripe_num){
                    all_send_flag = 0;
                }
            }
            if(all_send_flag) break; // all the packets are sent
            
            // set the fd_set
            FD_ZERO(&file_fd);
            for(server_idx = 0; server_idx < N; server_idx++){
                if(send_nums[server_idx] < stripe_num){
                    FD_SET(sds[server_idx], &file_fd);
                }
            }
            
            select(max_fd+1, NULL, &file_fd, NULL, NULL);
            for(server_idx = 0; server_idx < N; server_idx++){
                if(FD_ISSET(sds[server_idx], &file_fd)){
                    if(send_nums[server_idx] == -1){ // send the file header if -1
                        /*  set up the header */
                        strcpy(headerMsg.protocol, "myftp");
                        headerMsg.type = 0xFF;
                        messageLen = 11 + file_size;
                        headerMsg.length = htonl(messageLen);
                        headerMsg.idx = server_idx;
                        memcpy(buff, &headerMsg, 11);

                        if(((sendn(sds[server_idx], buff, 11))<0)){
                            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                            exit(0);
                        }

                        send_nums[server_idx] += 1;
                        break;
                    }
                    // in the case sendNums[idx] >= 0, just send the blocks from the cache file
                    // the block location is calculated as "send_num * N + server_idx" and then multiplied by blockSize
                    if((fseek(cachefd, (send_nums[server_idx] * N + server_idx) * blockSize, SEEK_SET)) < 0){
                        printf("Cache file seek error\n");
                        exit(0);
                    }
                    if((fread(block_buf, 1, blockSize, cachefd)) < 0){
                        printf("Cache file read error \n");
                        exit(0);
                    }
                    if(((sendn(sds[server_idx], block_buf, blockSize))<0)){
                        printf("Send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                        exit(0);
                    }
                    send_nums[server_idx] += 1;

                    break;   
                }
            }


        }


        fclose(cachefd);

        fclose(fd);

        if((remove(cacheName)) != 0){ // remove the cache file here
            printf("Remove cache error\n");
            exit(0);
        }

        for( i = 0; i < N; i++){
            close(sds[i]); // close all sockets
        }
    }
    return 0;
}
