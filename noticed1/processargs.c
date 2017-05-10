/* processargs */

/* process server file program arguments */
/* version %I% last modified %G% */


#define	CF_DEBUG	0


/* revision history:

	= 91/09/01, David A­D­ Morano

	This program was originally written.


*/



/*****************************************************************************

	These subroutines are used to parse the SERVER program arguments
	from an expanded (substituted) server file entry.  Basically,
	we just "field-SHELL" out arguments and put them into the
	supplied vector string object.


*****************************************************************************/




#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ftw.h>
#include	<dirent.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	BUFLEN
#define	BUFLEN	(4 * MAXPATHLEN)
#endif



/* external subroutines */


/* externals variables */


/* forward references */


/* local structures */


int processargs(pip,args,alp)
struct proginfo	*pip ;
char		args[] ;
vecstr		*alp ;
{
	FIELD	fsb ;

	int	rs = SR_OK ;
	int	i = 0 ;

	uchar	terms[32] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("processargs: ent >%s<\n",args) ;
#endif

	if ((args == NULL) || (args[0] == '\0')) goto ret ;

	fieldterms(terms,0," \t") ;

	if ((field_start(&fsb,args,-1)) >= 0) {
	    const int	flen = BUFLEN ;
	    int		fl ;
	    char	fbuf[BUFLEN + 1] ;

	    while ((fl = field_sharg(&fsb,terms,fbuf,flen)) > 0) {

	        i += 1 ;
	        rs = vecstr_add(alp,fbuf,fl) ;

		if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb)  ;
	} /* end if (field) */

ret:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("processargs: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (processargs) */


