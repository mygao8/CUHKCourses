#include <stdio.h>
#include <stdlib.h>
#include "myftp.h"
#define min(x,y) ((x) < (y)?(x):(y))

int main(){
    FILE *fd = fopen("data/.metadata", "wb");  
    struct metadata *data = (struct metadata *)malloc(sizeof(struct metadata));
    strcpy(data->filename, "test1");
    data->filesize = 100;
    data->idx = 1;
    int ret = fwrite(data, sizeof(struct metadata), 1, fd);
    

    // printf("data:%x filename:%x, filesize:%x, idx:%x\n", data, data->filename, &data->filesize, &data->idx);
    // while (i <100){
    //     i++;
    // }

    // printf("%d %d %d\n",ret, sizeof(struct metadata), sizeof(data));
    // printf("%d %d %d\n", sizeof(data->filename), sizeof(data->filesize), sizeof(data->idx));
    if (ret<0){
        printf("write error");
    }

    strcpy(data->filename, "test12");
    data->filesize = 1000;
    data->idx = 3;
    ret = fwrite(data, 1, sizeof(struct metadata), fd);
    if (ret<0){
        printf("write error");
    }

    strcpy(data->filename, "test124");
    data->filesize = 190;
    data->idx = 9;
    ret = fwrite(data, 1, sizeof(struct metadata), fd);
    if (ret<0){
        printf("write error");
    }

    fclose(fd);
}
