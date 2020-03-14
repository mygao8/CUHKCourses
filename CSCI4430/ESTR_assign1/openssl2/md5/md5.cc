/*
 * md5.cc
 * - Using md5 with openSSL. MD5 returns a 128-bit hash value from a string.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/md5.h>

int main(int argc, char** argv) {
	MD5_CTX hash_ctx;
	char input_string[128];
	unsigned char hash_ret[16];
	int i;

	// check usage
	if (argc != 2) {
		fprintf(stderr, "%s <input string>\n", argv[0]);
		exit(-1);
	}

	// set the input string
	snprintf(input_string, sizeof(input_string), "%s\n", argv[1]);

	// initialize a hash context 
	MD5_Init(&hash_ctx);

	// update the input string to the hash context (you can update
	// more string to the hash context)
	MD5_Update(&hash_ctx, input_string, strlen(input_string));

	// compute the hash result
	MD5_Final(hash_ret, &hash_ctx);

	// print
	printf("Input string: %s", input_string);
	printf("Output string: ");
	for (i=0; i<32; ++i) {
		if (i % 2 == 0) {
			printf("%x", (hash_ret[i/2] >> 4) & 0xf);
		} else {
			printf("%x", (hash_ret[i/2]) & 0xf);
		}
	}
	printf("\n");

	return 0;
}
