#include	<sys/types.h>
#include	<unistd.h>
#include	<stdio.h>
int main()
{
	printf("sizeof(off_t)=%u\n",sizeof(off_t)) ;
	printf("sizeof(long_long)=%u\n",sizeof(long long)) ;
	printf("sizeof(long)=%u\n",sizeof(long)) ;
	return 0 ;
}

