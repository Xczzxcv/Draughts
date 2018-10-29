#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(void){
    int i,
        *box_ptr,
        N = (int)pow(10, 10);
    long long M = (long long)pow(2, 50);
    printf("%d\n", sizeof(int));
    return;
    for(i=0;i<N;i++){
        box_ptr = malloc(M * sizeof(int));
        if(!box_ptr){
            puts("FUCK THAT IS");
            exit(666);
        }
        if(!(i % 1000))
            printf("%d\n", i);
//        free(box_ptr);
        //puts("");
    }
    puts("wait...");
    getch();
}
