/* main */



#define	F_SETREG	1	/* set subroutine return register */
#define	F_ISUB		0	/* call interrupt subroutine */
#define	F_SETLINK	1	/* set the 'uc_link' element */



#include	<sys/types.h>
#include	<sys/regset.h>
#include	<ucontext.h>
#include	<stdlib.h>
#include	<stdio.h>

#include	<vsystem.h>



#define	VARDEBUGFD1	"TESTCONTEXT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"



extern int	cfdeci(const char *,int,int *) ;



static int	sub(ucontext_t *,int *) ;

static void	isub(int) ;




int main()
{
	ucontext_t	a ;

	int	c = 0 ;
	int	rs ;
	int	fd_debug ;

	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	fprintf(stdout,"main: started\n") ;

	rs = getcontext(&a) ;

	fprintf(stdout,"main: getcontext() rs=%d\n",rs) ;

	if (c < 1) {

	    fprintf(stdout,"main: sub() \n") ;

	    sub(&a,&c) ;

	    fprintf(stdout,"main: sub() returned\n") ;

	}

	fprintf(stdout,"main: returning\n") ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */


static int sub(ucp,cp)
ucontext_t	*ucp ;
int		*cp ;
{
	ucontext_t	suber, *next = ucp ;


	fprintf(stdout,"sub: this is the subroutine uc_link=%p\n",
	    ucp->uc_link) ;

	*cp += 1 ;

#if	F_SETREG
	ucp->uc_mcontext.gregs[REG_O0] = 1 ;
#endif

#if	F_ISUB

/* duplicate the main-line context */

	(void) memcpy(&suber,ucp,sizeof(ucontext_t)) ;

#if	F_SETLINK
	suber.uc_link = ucp ;
#endif /* F_SETLINK */

	fprintf(stdout,"sub: suber uc_link=%p\n",
	    suber.uc_link) ;

	makecontext(&suber,isub,2,2) ;

	fprintf(stdout,"sub: after makecontext() uc_link=%p\n",
	    suber.uc_link) ;

	next = &suber ;

#endif /* F_ISUB */

	setcontext(next) ;

/* NOTREACHED */

	fprintf(stdout,"sub: returning\n") ;

	return 0 ;
}


static void isub(a1)
int	a1 ;
{


	fprintf(stdout,"isub: this is the interrupt subroutine a1=%d\n",a1) ;

	fflush(stdout) ;

}



