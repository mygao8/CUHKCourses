/*
 * ssl_server.c
 * - Adapted from
 *   http://h71000.www7.hp.com/doc/83final/ba554_90007/ch05s03.html
 * - Modified by Patrick P. C. Lee.
 */

#include <stdio.h>
#include <stdlib.h>
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

#define RSA_SERVER_CERT     "cert.pem"
#define RSA_SERVER_KEY      "key.pem"

#define ON         1
#define OFF        0

#define RETURN_NULL(x) if ((x)==NULL) exit(1)
#define RETURN_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define RETURN_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(1); }

int main()
{
	int     err;
	int     verify_client = OFF; /* To verify a client certificate, set ON */
	int     listen_sock;
	int     sock;
	struct sockaddr_in sa_serv;
	struct sockaddr_in sa_cli;
	socklen_t client_len;
	char    *str;
	char     buf[4096];

	SSL_CTX         *ctx;
	SSL            *ssl;
	SSL_METHOD      *meth;
	X509            *client_cert = NULL;

	short int       s_port = 5555;
	
	/*----------------------------------------------------------------*/
	/* Register all algorithms */
	OpenSSL_add_all_algorithms();

	/* Load encryption & hashing algorithms for the SSL program */
	SSL_library_init();

	/* Load the error strings for SSL & CRYPTO APIs */
	SSL_load_error_strings();

	/* Create a SSL_METHOD structure (choose a SSL/TLS protocol version) */
	meth = (SSL_METHOD*)SSLv23_method();

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

	if(verify_client == ON) {
		/* Load the RSA CA certificate into the SSL_CTX structure */
		if (!SSL_CTX_load_verify_locations(ctx, "server_ca.crt", NULL)) {
			ERR_print_errors_fp(stderr);
			exit(1);
		}
		/* Set to require peer (client) certificate verification */
		SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,NULL);

		/* Set the verification depth to 1 */
		SSL_CTX_set_verify_depth(ctx,1);
	}
	
	/* ----------------------------------------------- */
	/* Set up a TCP socket */

	listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);   
	if (listen_sock == -1) {
		perror("socket");
		exit(-1);
	}
	memset (&sa_serv, 0, sizeof(sa_serv));
	sa_serv.sin_family      = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port        = htons (s_port);          /* Server Port number */

	err = bind(listen_sock, (struct sockaddr*)&sa_serv,sizeof(sa_serv));
	if (err == -1) {
		perror("bind");
		exit(-1);
	}

	/* Wait for an incoming TCP connection. */
	err = listen(listen_sock, 5);
	if (err == -1) {
		perror("listen");
		exit(-1);
	}

	/* Socket for a TCP/IP connection is created */
	printf("Accepting connection...\n");

	client_len = sizeof(struct sockaddr_in);
	sock = accept(listen_sock, (struct sockaddr*)&sa_cli, &client_len);
	if (sock == -1) {
		perror("accept");
		exit(-1);
	}

	// terminating the listen socket
	close (listen_sock);

	printf ("Connection from %lx, port %x\n", 
			(unsigned long) sa_cli.sin_addr.s_addr, 
			sa_cli.sin_port);

	/* ----------------------------------------------- */
	/* TCP connection is ready. */
	/* A SSL structure is created */
	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		fprintf(stderr, "ERR: unable to create the ssl structure\n");
		exit(-1);
	}

	/* Assign the socket into the SSL structure (SSL and socket without BIO) */
	SSL_set_fd(ssl, sock);

	/* Perform SSL Handshake on the SSL server */
	err = SSL_accept(ssl);
	if (err == -1) {
		ERR_print_errors_fp(stderr); 
		exit(1);
	}

	/* Informational output (optional) */
	printf("SSL connection using %s\n", SSL_get_cipher (ssl));

	if (verify_client == ON) {
		/* Get the client's certificate (optional) */
		client_cert = SSL_get_peer_certificate(ssl);
		if (client_cert != NULL) {
			printf ("Client certificate:\n");     
			str = X509_NAME_oneline(X509_get_subject_name(client_cert), 0, 0);
			RETURN_NULL(str);
			printf ("\t subject: %s\n", str);
			free (str);
			str = X509_NAME_oneline(X509_get_issuer_name(client_cert), 0, 0);
			RETURN_NULL(str);
			printf ("\t issuer: %s\n", str);
			free (str);
			X509_free(client_cert);
		} else {
			printf("The SSL client does not have certificate.\n");
		}
	}

	/*------- DATA EXCHANGE - Receive message and send reply. -------*/
	/* Receive data from the SSL client */
	err = SSL_read(ssl, buf, sizeof(buf) - 1);
	if (err == -1) {
		ERR_print_errors_fp(stderr); 
		exit(1);
	}
	buf[err] = '\0';
	printf ("Received %d chars:'%s'\n", err, buf);

	/* Send data to the SSL client */
	err = SSL_write(ssl, "This message is from the SSL server", 
			strlen("This message is from the SSL server"));
	if (err == -1) {
		ERR_print_errors_fp(stderr); 
		exit(1);
	}

	/*--------------- SSL closure ---------------*/
	/* Shutdown this side (server) of the connection. */
	err = SSL_shutdown(ssl);
	if (err == -1) {
		ERR_print_errors_fp(stderr); 
		exit(1);
	}

	/* Terminate communication on a socket */
	err = close(sock);
	if (err == -1) {
		perror("close");
		exit(-1);
	}

	/* Free the SSL structure */
	SSL_free(ssl);

	/* Free the SSL_CTX structure */
	SSL_CTX_free(ctx);

	return 0;
}
