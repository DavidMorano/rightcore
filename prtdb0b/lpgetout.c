/* lpgetout */

/* capture output from LPGET program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-01-10, Dave morano
	Originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine uses the LPGET program to retrieve a key from the
        PRINTERS database.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	TO_CHILD	5


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */

#if	CF_DEBUG
extern char	**environ ;
#endif


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int lpgetout(pip,printer,vbuf,vlen,key)
struct proginfo	*pip ;
const char	printer[] ;
char		vbuf[] ;
int		vlen ;
const char	key[] ;
{
	bfile	ofile, *ofp = &ofile  ;

	const int	llen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	len, cl, ml ;
	int	vl = 0 ;
	int	f_gotit ;
	int	f_first ;

	const char	*progfname = pip->prog_lpget ;
	const char	*cp, *tp ;
	const char	*av[6] ;

	char	progname[MAXNAMELEN + 1] ;
	char	lbuf[LINEBUFLEN + 1] ;


	if (printer == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("lpgetout: printer=%s key=%s\n",printer,key) ;
	debugprintf("lpgetout: progfname=%s\n",progfname) ;
	}
#endif

	if ((cl = sfbasename(progfname,-1,&cp)) > 0) {
		ml = MIN(MAXNAMELEN,cl) ;
		strwcpy(progname,cp,ml) ;
	} else
		sncpy1(progname,MAXNAMELEN,progfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("lpgetout: progname=%s\n",progname) ;
#endif

/* prepare arguments */

	i = 0 ;
	av[i++] = progname ;
	av[i++] = "-k" ;
	av[i++] = key ;
	av[i++] = printer ;
	av[i] = NULL ;

	if ((rs = bopenprog(ofp,progfname,"r",av,pip->envv)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("lpgetout: opened-prog rs=%d\n",rs) ;
#endif

	    f_gotit = FALSE ;
	    f_first = TRUE ;
	    while ((rs = breadline(ofp,lbuf,llen)) > 0) {
	        len = rs ;

	        if ((tp = strnchr(lbuf,len,':')) == NULL)
	            continue ;

	        if (strncmp(lbuf,printer,(tp - lbuf)) == 0) {

		    cl = (lbuf + len - (tp + 1)) ;
		    cp = (tp + 1) ;
			while ((cl > 0) && isspace(cp[cl - 1]))
				cl -= 1 ;

	            f_gotit = TRUE ;
	            break ;
	        }

	    } /* end while (readling lines) */

	    if ((rs >= 0) && f_gotit) {

	        while (f_first ||
			((rs = breadline(ofp,lbuf,llen)) > 0)) {

		    if (! f_first) {

	                len = rs ;
	                if (lbuf[len - 1] == '\n')
	                    len -= 1 ;

	                cl = sfshrink(lbuf,len,&cp) ;

		    } else
			f_first = FALSE ;

	            tp = strnpbrk(cp,cl,"=-") ;

		    if ((tp != NULL) && (tp[0] == '=')) {

	                vl = (cp + cl) - (tp + 1) ;

	                rs = (vl <= vlen) ? SR_OK : SR_OVERFLOW ;

	                if (rs >= 0)
	                    rs = strwcpy(vbuf,(tp + 1),MIN(vlen,vl)) - vbuf ;

	                break ;
	            }

	        } /* end while */

		if (rs == 0)
			rs = SR_NOTFOUND ;

	    } /* end if (extracting value) */

	    bclose(ofp) ;
	} /* end if (reading child output) */

#if	CF_DEBUG
	debugprintf("lpgetout: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (lpgetout) */



