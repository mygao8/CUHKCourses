/*
 * ssl_client.c
 * - Adapted from
 *   http://h71000.www7.hp.com/doc/83final/ba554_90007/ch05s03.html
 * - Modified by Patrick P. C. Lee.
 */

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
 
int main() {

	int     err;
	int     verify_client = OFF; /* To verify a client certificate, set ON */
	int     sock;
	struct sockaddr_in server_addr;
	char  *str;
	char    buf [4096];
	char    hello[80];

	SSL_CTX         *ctx;
	SSL             *ssl;
	SSL_METHOD      *meth;
	X509            *server_cert;

	short int       s_port = 5555;
	const char      *s_ipaddr = "127.0.0.1";

	/*----------------------------------------------------------*/
	printf ("Message to be sent to the SSL server: ");
	fgets (hello, 80, stdin);

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

	/* ------------------------------------------------------------- */
	/* Set up a TCP socket */

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);       
	if (sock == -1) {
		perror("socket");
		exit(-1);
	}

	memset (&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_port        = htons(s_port);       /* Server Port number */
	server_addr.sin_addr.s_addr = inet_addr(s_ipaddr); /* Server IP */

	/* Establish a TCP/IP connection to the SSL client */
	err = connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)); 
	if (err == -1) {
		perror("connect");
		exit(-1);
	}
	
	/* ----------------------------------------------- */
	/* An SSL structure is created */

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		fprintf(stderr, "ERR: cannot create ssl structure\n");
		exit(-1);
	}

	/* Assign the socket into the SSL structure */
	SSL_set_fd(ssl, sock);

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

	/*-------- DATA EXCHANGE - send message and receive reply. -------*/
	/* Send data to the SSL server */
	err = SSL_write(ssl, hello, strlen(hello));  
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	/* Receive data from the SSL server */
	err = SSL_read(ssl, buf, sizeof(buf)-1);                     
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}
	buf[err] = '\0';
	printf ("Received %d chars:'%s'\n", err, buf);

	/*--------------- SSL closure ---------------*/
	/* Shutdown the client side of the SSL connection */
	err = SSL_shutdown(ssl);
	if (err == -1) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}

	/* Terminate communication on a socket */
	err = close(sock);
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
