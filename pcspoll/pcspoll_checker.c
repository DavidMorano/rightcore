/* pcspoll_checker */

/* debugging support */


#define	CF_VERBOSE	0
#define	CF_PRINT	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */

#undef	MAXLEN
#define	MAXLEN	10


/* external subroutines */

extern char	*strwcpy(char *,char *,int) ;

void		pcspoll_break() ;


/* exported subroutines */


void pcspoll_checker(filename,line,fid,size,align,op,np)
char		filename[] ;
unsigned int	line ;
int		fid ;
DMALLOC_SIZE	size ;
DMALLOC_SIZE	align ;
DMALLOC_PNT	op ;
DMALLOC_PNT	np ;
{
	int	sl, i ;

	char	*p ;


#ifdef	MALLOCLOG
#if	CF_VERBOSE && CF_PRINT
	nprintf("o4","fid=%d p=%08lx size=%d\n",
	    fid,np,size) ;
#endif
#endif


	if ((fid == DMALLOC_FUNC_MALLOC) && (size == 11)) {

	    char	buf[100] ;


#ifdef	MALLOCLOG
	    p = (char *) np ;
	    for (sl = 0 ; (sl < MAXLEN) && (*p != '\0') ; sl += 1) ;

#if	CF_PRINT
	    nprintf("o4","%08lx> %W\n",p,p,sl) ;
#endif
#endif

	    pcspoll_break() ;
	}

}
/* end subroutine (pcspoll_checker) */


void pcspoll_break()
{

	return ;
}



