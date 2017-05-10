/* skipheaders */


#define	CF_DEBUG	0


/************************************************************************
 *									
	= David A.D. Morano, 94/01/06
	What a piece of )^&$#$(%^#% !!
*
*
************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<errno.h>
#include	<unistd.h>
#include	<string.h>
#include	<signal.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdio.h>

#include	<baops.h>
#include	<bfile.h>
#include	<char.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"prompt.h"
#include	"header.h"



/* external subroutines */

extern int	isheader() ;


/* external data */


/* global data */

extern struct global	g ;




int skipheaders(fp,buf,blen)
bfile	*fp ;
char	buf[] ;
int	blen ;
{
	int	len, l, i ;
	int	f_leading = TRUE ;
	int	f_bol, f_eol ;
	int	f_done, f_again ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("skipheaders: entered\n") ;
#endif

	len = MIN(blen,BUFLEN) ;

	f_again = FALSE ;
	f_bol = TRUE ;
	while ((l = breadline(fp,buf,len)) > 0) {

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("skipheaders: >%W",buf,l) ;
#endif

	    f_eol = FALSE ;
	    if (buf[l - 1] == '\n') f_eol = TRUE ;

	    buf[l] = '\0' ;
	    if (f_bol) {

		if (buf[0] == '\n') return 0 ;

	        f_done = TRUE ;
	        if (f_leading) {

	            f_done = ! ((strncmp(buf,"From ",5) == 0) ||
	                (strncmp(buf,">From ",6) == 0)) ;

	            if (f_done) f_leading = FALSE ;

	        }

	        if (f_again && CHAR_ISWHITE(buf[0])) {

	            f_done = FALSE ;

	        } else {

	            f_again = FALSE ;
	            if (isheader(buf)) {

	                f_done = FALSE ;
	                f_again = TRUE ;
	            }

	        }

	        if (f_done) break ;

	    } /* end if (BOL) */

	    f_bol = f_eol ;

	} /* end while (getting lines) */

	return l ;
}
/* end subroutine (skipheaders) */



