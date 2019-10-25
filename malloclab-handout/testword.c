#include <stdio.h>

int main()
{
    char a[]="你";
    int i=0;
    while(a[i]!='\0')
    {
        printf("%c\n",a[i++]);
    }
}