#include <stdio.h>

int main()
{
 int *a=NULL;
 int *b=a;
 int c[]={1,2};
 a=&c[0];
 printf("%d\n",*b);//segfault
}
