
#include	<stdio.h>


/* ARGSUSED */
int main(int argc,const char **argv,const char **envv)
{
	const char	*s = "Hello world!" ;
	const char	*fmt = "%7s\n" ;

	printf(fmt,s) ;
	fmt = "%2d\n" ;
	printf(fmt,100) ;
	return 0 ;
}


