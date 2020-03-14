/*
 * rsa_pem.cc
 * - Show the usage of RSA that reads from key files.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

int main(int argc, char** argv) {
	FILE* fp;
	RSA* rsa1;

	// check usage
	if (argc != 2) {
		fprintf(stderr, "%s <RSA private key param file>\n", argv[0]);
		exit(-1);
	}

	// open the RSA private key PEM file 
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Unable to open %s for RSA priv params\n", argv[1]);
		return NULL;
	}
	if ((rsa1 = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
		fprintf(stderr, "Unable to read private key parameters\n");
		return NULL;
	}                                                         
	fclose(fp);
	
	// print 
	printf("Content of Private key PEM file\n");
	RSA_print_fp(stdout, rsa1, 0);
	printf("\n");	


/*
	RSA* rsa2;

	// open the RSA public key PEM file
	fp = fopen(argv[2], "r");
	if (fp == NULL) {
		fprintf(stderr, "Unable to open %s for RSA pub params\n", argv[1]);
		return NULL;
	}
	if ((rsa2 = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL)) == NULL) {
		fprintf(stderr, "Unable to read public key parameters\n");
		return NULL;
	}                                                         
	fclose(fp);
	
	printf("Content of Public key PEM file\n");
	RSA_print_fp(stdout, rsa2, 0);
	printf("\n");	
*/
	return 0;
}
