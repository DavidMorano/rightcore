/* pcspoll_checker */

/* debugging support */


#define	F_VERBOSE	0
#define	F_PRINT		0



#include	<sys/types.h>
#include	<string.h>

#include	"misc.h"

#include	<dmalloc.h>



#define	FD_OUT	4
#define	MAXLEN	10



/* external subroutines */

extern char	*strwcpy(char *,char *,int) ;

void	pcspoll_break() ;




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
#if	F_VERBOSE && F_PRINT
		nprintf("o4","fid=%d p=%08lx size=%d\n",
			fid,np,size) ;
#endif
#endif


	if ((fid == DMALLOC_FUNC_MALLOC) && (size == 11)) {

		char	buf[100] ;


#ifdef	MALLOCLOG
		p = (char *) np ;
		for (sl = 0 ; (sl < MAXLEN) && (*p != '\0') ; sl += 1) ;

#if	F_PRINT
		nprintf("o4","%08lx> %W\n",p,p,sl) ;
#endif
#endif

		pcspoll_break() ;
	}

}


void pcspoll_break()
{

	return ;
}



