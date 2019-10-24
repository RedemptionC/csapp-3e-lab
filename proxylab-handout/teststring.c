#include <stdio.h>
#include <string.h>

int main()
{
    char s[100]="Host:www.baidu.com:80";
    // sprintf(s,"80");
    char *p=index(s,':');
    char *p2=index(p+1,':');
    printf("%s\n%s\n",p,p2);
}