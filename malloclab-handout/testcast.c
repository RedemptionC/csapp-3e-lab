#include <stdio.h>

int main()
{
    // int a=50;
    // int *b=&a;
    // char * c=(char *)b;
    // printf("%p %p %d %c\n",b,c,*b,*c);
    int a=-1;
    int *b=&a;
    unsigned int *c=&a;
    printf("%d %d\n",*b,*c);
}