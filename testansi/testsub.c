/* testsub */

#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>

#include	<localmisc.h>


int main()
{
	ushort	us = 256 ;
	uchar	uc = 130 ;

	printf("main: %d\n",(us-uc)) ;

	return 0 ;
}

