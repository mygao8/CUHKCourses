#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<isa-l.h>

#define k 2
#define n 3
#define blockSize 4
void main(){
    unsigned char **src = (unsigned char **)malloc(k);
    src[0] = (unsigned char *)malloc(blockSize);
    src[1] = (unsigned char *)malloc(blockSize);
    unsigned char **dest = (unsigned char **)malloc(n-k);
    dest[0] = (unsigned char *)malloc(blockSize);
    strcpy(src[0], "yz00");
    strcpy(src[1], "0000");
    strcpy(dest[0], "aaaa");
    

    uint8_t encodeMatrix[n*k];
    gf_gen_rs_matrix(encodeMatrix, n, k);
    int  i;
    uint8_t table[32*k*(n-k)];
    ec_init_tables(k, n-k, &encodeMatrix[k*k],table);
    for(i = 0;i < k;i++){
        int j;
        for(j = 0;j < blockSize;j++){
            printf("%x ", src[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    
    ec_encode_data(blockSize, k, n-k, table, src, dest);
    

    FILE *f = fopen("testFile3", "ab");
    fwrite(dest[0], 1, blockSize, f);
    fclose(f);

    for(i = 0;i < n-k;i++){
        int j;
        for(j = 0;j < blockSize;j++){
            printf("%x ", dest[i][j]);
        }
    }
    printf("\n");

    uint8_t sel[k*k] = {0,1,1,1};
    uint8_t invertMatrix[k*k];
    gf_invert_matrix(sel, invertMatrix, k);
    for(i = 0;i < k*k;i++){
        printf("%x ", invertMatrix[i]);
    }
    printf("\n");
    unsigned char **src2 = (unsigned char **)malloc(k);
    src2[0] = src[1];
    src2[1] = dest[0];
    ec_init_tables(k, n-k, sel,table);
    ec_encode_data(blockSize, k, n-k, table, src, dest);
    for(i = 0;i < n-k;i++){
        int j;
        for(j = 0;j < blockSize;j++){
            printf("%x ", dest[i][j]);
        }
    }
}