#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *fp = fopen("file0", "r");
    char buf[100];
    int ret = fgets(buf, sizeof(buf), fp);
    if (ret != NULL){
        printf("1 %d\n",ret);
    }
    ret = fgets(buf, sizeof(buf), fp);
    if (ret != NULL){
        printf("2 %d\n",ret);
    }
    else
    {
        printf("2 NULL\n");
    }
    
    ret = fgets(buf, sizeof(buf), fp);
    if (ret != NULL){
        printf("3 %d\n",ret);
    }else{
        printf("3 NULL\n");
    }
    printf("before fclose: fp=%d\n",fp);

    fclose(fp);

    printf("after fclose: fp=%d\n",fp);
}