#include	<stdio.h>
extern int	_lib_version ;

int main()
{
	int	v = _lib_version ;
	printf("lib_version=%d\n",v) ;
	return 0 ;
}

