/*
 * aes.cc
 * - Show the usage of AES encryption/decryption
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>

#define AES_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

int main(int argc, char** argv) {
	EVP_CIPHER_CTX* ctx;
	EVP_CIPHER_CTX* ctx2;
	unsigned char key[AES_KEY_SIZE];		
	unsigned char iv[AES_BLOCK_SIZE];	
	unsigned char* input_string;
	unsigned char* encrypt_string;
	unsigned char* decrypt_string;
	int plaintext_len = 0;		// plaintext length
	int ciphertext_len = 0;	// ciphertext length 
	int len;	
	int i;

	// check usage
	if (argc != 2) {
		fprintf(stderr, "%s <plain text>\n", argv[0]);
		exit(-1);
	}
	
	// include all algorithms
	OpenSSL_add_all_algorithms();

	/* 
	 * Encryption
	 */
	// set the encryption length
	len = 0;
	if ((strlen(argv[1]) + 1) % AES_BLOCK_SIZE == 0) {
		len = strlen(argv[1]) + 1;
	} else {
		len = ((strlen(argv[1]) + 1) / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
	}

	// set the input string
	input_string = (unsigned char*)calloc(len, sizeof(unsigned char));
	if (input_string == NULL) {
		fprintf(stderr, "Unable to allocate memory for input_string\n");
		exit(-1);
	}
	strncpy((char*)input_string, argv[1], strlen(argv[1]));
	plaintext_len = len; 
	
	// Define IV	
	memset(iv, 0, AES_BLOCK_SIZE);
	
	// Generate AES 256-bit key
	for (i=0; i<AES_KEY_SIZE; ++i) {
		key[i] = 32 + i;
	}

	// Set encryption key
	ctx = EVP_CIPHER_CTX_new(); 
	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
		fprintf(stderr, "Unable to set encryption key in AES\n");
		exit(-1);
	}
	
	// alloc encrypt_string
	encrypt_string = (unsigned char*)calloc(plaintext_len + AES_BLOCK_SIZE,
			sizeof(unsigned char));	
	if (encrypt_string == NULL) {
		fprintf(stderr, "Unable to allocate memory for encrypt_string\n");
		exit(-1);
	}
	
	// encrypt
	EVP_EncryptUpdate(ctx, encrypt_string, &len, input_string, plaintext_len);
	ciphertext_len = len; 
	if (EVP_EncryptFinal_ex(ctx, encrypt_string + len, &len) != 1) {
		fprintf(stderr, "Unable to encrypt\n");
		exit(-1);
	}
	ciphertext_len += len; 
	
	/*
	 * Decryption
	 */
	// malloc decrypt_string
	decrypt_string = (unsigned char*)calloc(ciphertext_len, 
			sizeof(unsigned char));
	if (decrypt_string == NULL) {
		fprintf(stderr, "Unable to allocate memory for decrypt_string\n");
		exit(-1);
	}
	
	// Set decryption key
	ctx2 = EVP_CIPHER_CTX_new(); 
	memset(iv, 0, AES_BLOCK_SIZE);
	if (EVP_DecryptInit_ex(ctx2, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
		fprintf(stderr, "Unable to set decryption key in AES\n");
		exit(-1);
	}

	// decrypt
	EVP_DecryptUpdate(ctx2, decrypt_string, &len, encrypt_string,
		ciphertext_len);
	plaintext_len = len; 
	if (EVP_DecryptFinal_ex(ctx2, decrypt_string + len, &len) != 1) {
		fprintf(stderr, "Unable to decrypt\n");
		exit(-1);
	}
	plaintext_len += len; 

	// print
	printf("input_string = %s\n", input_string);
	printf("encrypted string (%d) = ", ciphertext_len);
	for (i=0; i<ciphertext_len; ++i) {
		printf("%x%x", (encrypt_string[i] >> 4) & 0xf, 
				encrypt_string[i] & 0xf);	
	}
	printf("\n");
	printf("decrypted string = %s\n", decrypt_string);

	// free
	EVP_CIPHER_CTX_free(ctx);
	EVP_CIPHER_CTX_free(ctx2);

	return 0;
}
