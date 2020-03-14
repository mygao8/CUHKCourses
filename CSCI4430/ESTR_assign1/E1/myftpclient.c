#include "myftp.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
 
#define RSA_CLIENT_CA_CERT      "cacert.pem"
 
#define ON      1
#define OFF     0
 
unsigned char buff[MAXLEN];

int main(int argc, char **argv){
    int err;
    int verify_client = OFF;
    char *str;

	SSL_CTX         *ctx;
	SSL             *ssl;
	SSL_METHOD      *meth;
	X509            *server_cert;
    
	/* Register all algorithm */
	OpenSSL_add_all_algorithms();

	/* Load encryption & hashing algorithms for the SSL program */
	SSL_library_init();

	/* Load the error strings for SSL & CRYPTO APIs */
	SSL_load_error_strings();

	/* Create an SSL_METHOD structure (choose an SSL/TLS protocol version) */
	meth = (SSL_METHOD*)SSLv23_method();

	/* Create an SSL_CTX structure */
	ctx = SSL_CTX_new(meth);
	if (ctx == NULL) {
		fprintf(stderr, "ERR: Unable to create ctx\n");
		exit(-1);
	}

	/*----------------------------------------------------------*/
	if(verify_client == ON) {
		/* Load the client certificate into the SSL_CTX structure */
		if (SSL_CTX_use_certificate_file(ctx, "client.crt", 
					SSL_FILETYPE_PEM) <= 0) {
			ERR_print_errors_fp(stderr);
			exit(1);
		}

		/* Load the private-key corresponding to the client certificate */
		if (SSL_CTX_use_PrivateKey_file(ctx, "client.key", 
					SSL_FILETYPE_PEM) <= 0) {
			ERR_print_errors_fp(stderr);
			exit(1);
		}

		/* Check if the client certificate and private-key matches */
		if (!SSL_CTX_check_private_key(ctx)) {
			fprintf(stderr, 
					"Private key does not match the certificate public key\n");
			exit(1);
		}
	}

	/* Load the RSA CA certificate into the SSL_CTX structure */
	/* This will allow this client to verify the server's     */
	/* certificate.                                           */

	if (!SSL_CTX_load_verify_locations(ctx, RSA_CLIENT_CA_CERT, NULL)) {
		ERR_print_errors_fp(stderr);
		exit(1);
	}

	/* Set flag in context to require peer (server) certificate */
	/* verification */

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	SSL_CTX_set_verify_depth(ctx, 1);

    /* Set up a TCP socket */
    int port = atoi(argv[2]);
    struct sockaddr_in server_addr;
    
    int sd = socket(AF_INET, SOCK_STREAM, 0);
   
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(port);
    //socklen_t addrlen = sizeof(server_addr);
   
    /* Establish a TCP/IP connection to the SSL server */
    if(connect(sd,(struct sockaddr *)&server_addr, sizeof(server_addr))<0){
        printf("connect error: %s (ERRNO:%d)\n",strerror(errno), errno);
        exit(0);
    }

	/* ----------------------------------------------- */
	/* An SSL structure is created */

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		fprintf(stderr, "ERR: cannot create ssl structure\n");
		exit(-1);
	}

	/* Assign the socket into the SSL structure */
	SSL_set_fd(ssl, sd);

	/* Perform SSL Handshake on the SSL client */
	printf("SSL handshake starts\n");
	err = SSL_connect(ssl);
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	/* Informational output (optional) */
	printf ("SSL connection using %s\n", SSL_get_cipher (ssl));

	/* Get the server's certificate */
	server_cert = SSL_get_peer_certificate(ssl);    
	if (server_cert != NULL) {
		printf ("Server certificate:\n");

		str = X509_NAME_oneline(X509_get_subject_name(server_cert),0,0);
		if (str == NULL) {
			exit(-1);
		}
		printf ("\t subject: %s\n", str);
		free (str);

		str = X509_NAME_oneline(X509_get_issuer_name(server_cert),0,0);
		if (str == NULL) {
			exit(-1);
		}
		printf ("\t issuer: %s\n", str);
		free(str);

		X509_free (server_cert);
	} else {
		printf("The SSL server does not have certificate.\n");
	}

    unsigned char *message;
    struct message_s headerMsg;
    int sendNum;
    int recvNum;

    /*****************************
     * 
     * "list
     * 
     * ***************************/
    if(strcmp("list", argv[3]) == 0){
        /****** send header(0XA1) *******/
        headerMsg.length = htonl(10);
        strcpy(headerMsg.protocol, "myftp");
        headerMsg.type = (unsigned char)0xA1;
        message = (unsigned char *)malloc(10);
        memcpy(message, &headerMsg, 10);

        if((sendNum = (SSL_write(ssl, message, 10))<0)){
	    ERR_print_errors_fp(stderr);
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
        }
        /****** receive header(0XA2) *******/
        if((recvNum = SSL_read(ssl, buff, 10)) < 0){
	    ERR_print_errors_fp(stderr);
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(-1);
        }
        memcpy(&headerMsg, buff, 10);
        // headerMsg.length - 10 is the length for buff (dir info)
	// maybe the whole headerMsg is filled, we have no \0 at the end
        if((recvNum = SSL_read(ssl, buff, headerMsg.length - 10)) < 0){
	    ERR_print_errors_fp(stderr); 
            printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(-1);
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
        // message  = [headerMsg, fileName, '\0]
        memcpy(message, &headerMsg, 10);
        memcpy(message + 10, argv[4], fileNameLen);
        message[messageLen - 1] = '\0';

        if((sendNum = (SSL_write(ssl, message, messageLen))<0)){
	    ERR_print_errors_fp(stderr);
            printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(-1);
        }
        /****** send header(0XB2/0XB3) *******/
        if((recvNum = SSL_read(ssl, buff, 10)) < 0){
            ERR_print_errors_fp(stderr);
	    printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(-1);
        }

        memcpy(&headerMsg, buff, 10);
        if(headerMsg.type == 0xB3){
            // file not exit
            printf("file not exit\n");
            exit(-1);
        }else if(headerMsg.type == 0xB2){
            /****** send header(0XFF) *******/
            // get file header -> get file size
            if((recvNum = SSL_read(ssl, buff, 10)) < 0){
                ERR_print_errors_fp(stderr);
		printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
                exit(-1);
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
                if((recvNum = SSL_read(ssl, buff, nextSize)) < 0){
                    exit(-1);
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
            exit(-1);
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
        
        if((sendNum = (SSL_write(ssl, message, messageLen))<0)){
            ERR_print_errors_fp(stderr);
	    printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(-1);
        }
        /****** recv header(0XC2) *******/
        if((recvNum = SSL_read(ssl, buff, 10)) < 0){
            ERR_print_errors_fp(stderr);
	    printf("recv error: %s (ERRNO:%d)\n",strerror(errno), errno);
            exit(-1);
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
            if((sendNum = (SSL_write(ssl, buff, 10))<0)){
                ERR_print_errors_fp(stderr);
		printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                exit(-1);
            }
            // send file in packet(len <= MAXLEN)
            int remainFileLen = file_size;
            int readLen;
            int nextSize = min(MAXLEN, remainFileLen);
            while((readLen = fread(buff, 1, nextSize, fd))>0){
                if((sendNum = (SSL_write(ssl, buff, nextSize))<0)){
                    ERR_print_errors_fp(stderr);
		    printf("send error: %s (ERRNO:%d)\n",strerror(errno), errno);
                    exit(-1);
                }
                remainFileLen -= nextSize;
                nextSize = min(MAXLEN, remainFileLen);
            }
            fclose(fd);
        }
    }

	/*--------------- SSL closure ---------------*/
	/* Shutdown the client side of the SSL connection */
	err = SSL_shutdown(ssl);
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	/* Terminate communication on a socket */
	err = close(sd);
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	/* Free the SSL structure */
	SSL_free(ssl);

	/* Free the SSL_CTX structure */
	SSL_CTX_free(ctx);

    return 0;
}
