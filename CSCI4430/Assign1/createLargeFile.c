#include<stdio.h>

int main(){
	int i;
	FILE *fd = fopen("file0", "wb");
	for(i = 0 ;i < 1024*1024*100;i++){
		fwrite("a", 1, 1, fd);
	}
	return 0;
}
