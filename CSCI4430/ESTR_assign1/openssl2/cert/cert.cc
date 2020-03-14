/*
 * cert.cc
 * - Show the usage of sign/verify based on certificate
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#define CERTKEY_LEN 128

int main(int argc, char** argv) {
	X509* cert;							// certificate context
	EVP_MD_CTX evp_md_ctx;              // message digest context
	EVP_PKEY* priv_key;	
	EVP_PKEY* pub_key;
	
	FILE* fp;
	unsigned char* input_string;
	unsigned char* sign_string;
	unsigned int sig_len;
	unsigned int i;

	// check usage
	if (argc != 2) {
		fprintf(stderr, "%s <plain text>\n", argv[0]);
		exit(-1);
	}

	// set the input string
	input_string = (unsigned char*)calloc(strlen(argv[1]) + 1,
			sizeof(unsigned char));
	if (input_string == NULL) {
		fprintf(stderr, "Unable to allocate memory for input_string\n");
		exit(-1);
	}
	strncpy((char*)input_string, argv[1], strlen(argv[1]));
	
	/*** SIGN ***/
	
	OpenSSL_add_all_algorithms();
	//EVP_add_cipher(EVP_aes_128_cbc());	
	//EVP_add_digest(EVP_sha1());

	// load the private key
	if ((fp = fopen("key.pem", "r")) == NULL) {
		fprintf(stderr, "Unable to open private key file\n");
		exit(-1);
	}
	priv_key = PEM_read_PrivateKey(fp, NULL, NULL, (char*)"4430");
	if (priv_key == NULL) {
		fprintf(stderr, "cannot read private key.\n");
		exit(-1);
	}
	fclose(fp);

	// alloc sign_string
	sig_len = CERTKEY_LEN;
	sign_string = (unsigned char*)calloc(sig_len, sizeof(unsigned char));	
	if (sign_string == NULL) {
		fprintf(stderr, "Unable to allocate memory for sign_string\n");
		exit(-1);
	}

	// sign input_string
	EVP_SignInit(&evp_md_ctx, EVP_sha1());
	EVP_SignUpdate(&evp_md_ctx, input_string, strlen((char*)input_string));
	if (EVP_SignFinal(&evp_md_ctx, sign_string, &sig_len, priv_key) == 0) { 
		// release the loading
		EVP_cleanup();
		fprintf(stderr, "Unable to sign\n");
		exit(-1);
	}

	// release the loading
	//EVP_cleanup();

	/*** VERIFY ***/

	// load the certificate and public key
	if ((fp = fopen("cert.pem", "r")) == NULL) {           
		fprintf(stderr, "cannot open x509 cert file\n");
		exit(-1);                                         
	}
	if ((cert = PEM_read_X509(fp, NULL, NULL, NULL)) == NULL) {
		fprintf(stderr, "cannot read cert file\n");
		exit(-1);
	} 
	if ((pub_key = X509_get_pubkey(cert)) == NULL) {
		fprintf(stderr, "cannot read x509's public key\n");
		exit(-1);                                         
	} 
	fclose(fp);

	// dynamic loading of digest algorithm  
	//EVP_add_digest(EVP_sha1());

	// verify
	EVP_VerifyInit(&evp_md_ctx, EVP_sha1());                  
	EVP_VerifyUpdate(&evp_md_ctx, input_string, strlen((char*)input_string));
	int is_valid_signature = EVP_VerifyFinal(&evp_md_ctx, sign_string,
			sig_len, pub_key);
		
	// release the loading                                
	// EVP_cleanup();                                        

	// print
	printf("input_string = %s\n", input_string);
	printf("signed string = ");
	for (i=0; i<sig_len; ++i) {
		printf("%x%x", (sign_string[i] >> 4) & 0xf, 
				sign_string[i] & 0xf);	
	}
	printf("\n");
	printf("is_valid_signature? = %d\n", is_valid_signature);

	return 0;
}
