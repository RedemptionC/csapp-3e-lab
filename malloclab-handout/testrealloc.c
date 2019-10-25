#include <stdio.h>
#include <stdlib.h>

int main()
{
  int a=1;
  realloc(&a,2*sizeof(int));
}
