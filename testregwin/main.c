/* main */


#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdio.h>



/* local structures */

struct stuff {
	char	name[MAXNAMELEN + 1] ;
} ;


/* forward references */

static int	sub(int,int) ;





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct stuff	s, *sp = &s ;

	double a, b, c ;


	strcpy(s.name,"hello world") ;

	a = 1.0 ;
	b = 2.0 ;
	c = sub(a,b) ;

	fprintf(stdout,"answer is %12.4f size=%u\n",
		c,sizeof(sp->name)) ;

	fclose(stdout) ;
	
	return 0 ;
}
/* end subroutine (main) */


static int sub(a,b)
int	a, b ;
{


	return (a + b) ;
}




