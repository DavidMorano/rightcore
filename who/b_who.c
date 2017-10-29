

/* external variables */

const char	**environ ;


/* exported subroutines */


int b_who(int argc,cchar **argv,void *contextp)
{
	int	ex ;

	ex = main(argc,argv,environ) ;

	return ex ;
}
/* end subroutine (b_who) */


