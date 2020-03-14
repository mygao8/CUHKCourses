/*
 * dh.cc
 * - Show the usage of Diffie-Hellman. 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/pem.h>

int main(int argc, char** argv) {
	FILE* fp;
	DH* dh1;
	DH* dh2;
	int i;

	// check usage
	if (argc != 2) {
		fprintf(stderr, "%s <DH param file>\n", argv[0]);
		exit(-1);
	}

	// open the PEM file and generate the DH keys
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Unable to open %s for DH params\n", argv[1]);
		return NULL;
	}
	if ((dh1 = PEM_read_DHparams(fp, NULL, NULL, NULL)) == NULL) {
		fprintf(stderr, "Unable to read parameters\n");
		return NULL;
	}                                                         
	fclose(fp);

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Unable to open %s for DH params\n", argv[1]);
		return NULL;
	}
	if ((dh2 = PEM_read_DHparams(fp, NULL, NULL, NULL)) == NULL) {
		fprintf(stderr, "Unable to read parameters\n");
		return NULL;
	}                                                         
	fclose(fp);

	if (!DH_generate_key(dh1)) {
		fprintf(stderr, "Unable to generate public/private keys\n");
		return NULL;
	}
	if (!DH_generate_key(dh2)) {
		fprintf(stderr, "Unable to generate public/private keys\n");
		return NULL;
	}

	// key exchange
	unsigned char* key1 = (unsigned char*)calloc(DH_size(dh1),
			sizeof(unsigned char));
	unsigned char* key2 = (unsigned char*)calloc(DH_size(dh2),
			sizeof(unsigned char));
	DH_compute_key(key1, dh2->pub_key, dh1);
	DH_compute_key(key2, dh1->pub_key, dh2);

	// print
	printf("DH's p = %s\n", BN_bn2hex(dh1->p));
	printf("DH's g = %s\n", BN_bn2hex(dh1->g));
	printf("key1 = ");
	for (i=0; i<DH_size(dh1); ++i) {
		printf("%x%x", (key1[i] >> 4) & 0xf, key1[i] & 0xf);
	}
	printf("\n");
	printf("key2 = ");
	for (i=0; i<DH_size(dh2); ++i) {
		printf("%x%x", (key2[i] >> 4) & 0xf, key2[i] & 0xf);
	}
	printf("\n");

	return 0;
}
