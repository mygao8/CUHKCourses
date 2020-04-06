#include <stdio.h>
#include <string.h>
struct fileInfo_s{
unsigned char fileName[256];
unsigned int fileSize;
unsigned char idx;
}__attribute__((packed));

void main(){
struct fileInfo_s fileInfo;
strcpy(fileInfo.fileName, "testFile\0");
fileInfo.fileSize = 26;
fileInfo.idx = 2;

FILE *f = fopen(".metafile", "wb");
printf("size:%d\n", sizeof(fileInfo));
fwrite(&fileInfo, 1, sizeof(fileInfo), f);
fclose(f);
}
