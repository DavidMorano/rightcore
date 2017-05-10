/* shifter */


#include	<stdio.h>


#define	N	4



typedef unsigned int	uint ;



main()
{
	uint	uiw = 0xffffffff ;

	int	iw = -1 ;


	printf("uiw=%08x\n",(uiw >> N)) ;

	printf("iw=%08x\n",(iw >> N)) ;


	fclose(stdout) ;

	return 0 ;
}



