/* getbbopts */

/* get the BB options */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1995-05-01, David A­D­ Morano

	This code module was completely rewritten to 
	replace any original garbage that was here before.


*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine parses out options from the main PCS
	configuration file.

	Synopsis:

	int getpfopts(pip,setsp)
	struct proginfo	*pip ;
	vecstr		*setsp ;

	Arguments:

	hp		pointer to program information
	setsp		pointer to the PCS configuration SET variables

	Returns:

	<0		error
	>=0		number of configuration options processed


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<field.h>
#include	<vecstr.h>
#include	<pcsconf.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"dirlist.h"
#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;


/* external variables */


/* forward references */


/* local variables */

static const char *bbopts[] = {
	"newsdir",
	"fastscan",
	"extrascan",
	"popscreen",
	"readtime",
	"querytext",
	NULL
} ;

enum bbopts {
	bbopt_newsdir,
	bbopt_fastscan,
	bbopt_extrascan,
	bbopt_popscreen,
	bbopt_readtime,
	bbopt_querytext,
	bbopt_overlast
} ;


/* exported subroutines */


int getbbopts(pip,setsp)
struct proginfo	*pip ;
vecstr		*setsp ;
{
	FIELD	fb, *fbp = &fb ;

	int	rs = SR_OK ;
	int	i, oi, val ;
	int	fl ;
	int	c = 0 ;

	const char	*fp ;
	const char	*cp ;

	uchar	fterms[32] ;


/* system-wide options? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("getbbopts: scanning system options\n") ;
#endif

	for (i = 0 ; vecstr_get(setsp,i,&cp) >= 0 ; i += 1) {
	    const char	*cp2 ;

	    if (cp == NULL) continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("getbbopts: conf >%s<\n",cp) ;
#endif

	    if (strncmp(cp,"bb:",3) != 0) 
		continue ;

		cp += 3 ;
	    if ((cp2 = strchr(cp,'=')) == NULL) 
		continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("getbbopts: bbopt >%s<\n",cp) ;
#endif

	    if ((oi = matostr(bbopts,2,cp,(cp2 - cp))) >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("getbbopts: system valid option, oi=%d\n",
	                oi) ;
#endif

		cp2 += 1 ;
		c += 1 ;
	        switch (oi) {

		case bbopt_newsdir:
			if (cp2[0] != '\0') {
				if (pip->newsdname != NULL)
					uc_free(pip->newsdname) ;
				pip->newsdname = mallocstr(cp2) ;
			}
			break ;

	        case bbopt_fastscan:
	            if (cfdeci(cp2,-1,&val) >= 0)
	                pip->f.extrascan = (val == 0) ;
#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("getbbopts: set extrascan=%d\n",
	                    pip->f.extrascan) ;
#endif
	            break ;

	        case bbopt_extrascan:
	            if (cfdeci(cp2,-1,&val) >= 0)
	                pip->f.extrascan = (val != 0) ;
#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("getbbopts: set extrascan=%d\n",
	                    pip->f.extrascan) ;
#endif
	            break ;

	        case bbopt_popscreen:
	            if (cfdeci(cp2,-1,&val) >= 0)
	                pip->f.popscreen = (val != 0) ;
#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("getbbopts: set popscreen=%d\n",
	                    pip->f.popscreen) ;
#endif
	            break ;

	        case bbopt_readtime:
	            if (cfdeci(cp2,-1,&val) >= 0)
	                pip->f.readtime = (val != 0) ;
#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("getbbopts: set readtime=%d\n",
	                    pip->f.readtime) ;
#endif
	            break ;

		case bbopt_querytext:
			if (cp2[0] != '\0')
			pip->querytext = mallocstr(cp2) ;
			break ;

	        } /* end switch */

	    } /* end if (got a match) */

	} /* end for (PCS configuration) */

/* now check for local options */

	fieldterms(fterms,0,":") ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    int	f_opton ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("getbbopts: got some options >%s<\n",cp) ;
#endif

	    if ((rs = field_start(fbp,cp,-1)) >= 0) {

	    while (rs >= 0) {

	        if ((fl = field_get(fbp,fterms,&fp)) > 0) {

	            f_opton = TRUE ;
	            if ((*fp == '-') || 
	                (*fp == '+')) {

	                if (*fp == '-')
	                    f_opton = FALSE ;

	                fp += 1 ;
	                fl -= 1 ;
	            }

	            if ((oi = matostr(bbopts,2,fp,fl)) >= 0) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("getbbopts: valid option, oi=%d\n",oi) ;
#endif

			c += 1 ;
	                switch (oi) {

	                case bbopt_fastscan:
	                    pip->f.extrascan = (! f_opton) ;
	                    break ;

	                case bbopt_extrascan:
	                    pip->f.extrascan = f_opton ;
	                    break ;

	                case bbopt_popscreen:
	                    pip->f.popscreen = f_opton ;
	                    break ;

	                case bbopt_readtime:
	                    pip->f.readtime = f_opton ;
	                    break ;

	                } /* end switch */

	            } /* end if (got a match) */

	        } /* end if (non-zero length field) */

	    } /* end while */

	    field_finish(fbp) ;
	    } /* end if (field) */

	} /* end if (getting startup flags) */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("getbbopts: extrascan=%d\n",pip->f.extrascan) ;
	    debugprintf("getbbopts: readtime=%d\n",pip->f.readtime) ;
	}
#endif /* CF_DEBUG */

/* there is no more FASTSCAN option anymore, it is always slow now! */

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: FASTSCAN=%d\n",
	        pip->progname,(! pip->f.extrascan)) ;

	if (! pip->f.extrascan)
	    logfile_printf(&pip->lh,"FASTSCAN\n") ;
#endif /* COMMENT */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getbbopts) */



