#include<stdio.h>

int main(){
	int i;
	FILE *fd = fopen("file0", "wb");
	for(i = 0 ;i < 1024*256*1;i++){
		fwrite("abcdefgh", 1, 8, fd);
	}
	return 0;
}
