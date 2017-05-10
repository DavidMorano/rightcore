/* output */

/* subroutine to write out the make time information */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine outputs the created string.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* local variables */


/* exported subroutines */


int output(PROGINFO *pip,cchar *ofname)
{
	bfile		ofile, *ofp = &ofile ;
	const time_t	clock = pip->daytime ;
	const int	olen = ORGLEN ;
	int		rs = SR_OK ;
	int		len = 0 ;
	int		wlen = 0 ;
	cchar		*org = pip->org ;
	cchar		*module = pip->module ;
	char		obuf[ORGLEN + 1], *bp ;
	char		timebuf[TIMEBUFLEN + 1] ;

/* we cannot allow any weirdo quotes from getting into our target string */

	if ((org != NULL) && (org[0] != '\0')) {
	    int		i ;
	    len = strnlen(org,olen) ;
	    bp = obuf ;
	    for (i = 0 ; (i < len) && org[i] ; i += 1) {
	        if (org[i] == '"') *bp++ = '\\' ;
	        *bp++ = org[i] ;
	    } /* end for */
	    *bp++ = '\0' ;
	} else {
	    obuf[0] = '\0' ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("output: opening output file\n") ;
#endif

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {
	    offset_t	offset ;

	    if ((module != NULL) && (module[0] != '\0')) {
	        rs = bprintf(ofp,"const char %s_makedate[] =\n\"",module) ;
	    } else
	        rs = bprintf(ofp,"const char makedate[] =\n\"") ;

	    wlen += rs ;
	    rs = bputc(ofp,'@') ;
	    wlen += rs ;

	    rs = bputc(ofp,CH_LPAREN) ;
	    wlen += rs ;

	    rs = bputc(ofp,'#') ;
	    wlen += rs ;

	    rs = bputc(ofp,CH_RPAREN) ;
	    wlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("output: outputting m=%\n",module) ;
#endif

	    if ((module != NULL) && (module[0] != '\0')) {
	        if (pip->f.main) {
	            rs = bprintf(ofp,"%-23s",module) ;
	    	    wlen += rs ;
	        } else {
	            rs = bprintf(ofp,"/%-22s",module) ;
	    	    wlen += rs ;
	   	}
	    } else {
	        rs = bprintf(ofp,"%-23s","") ;
	        wlen += rs ;
	    }

	    rs = bprintf(ofp," %s",timestr_logz(clock,timebuf)) ;
	    wlen += rs ;

	    if ((pip->org != NULL) && (pip->org[0] != '\0')) {
	        rs = bprintf(ofp," %s",obuf) ;
	        wlen += rs ;
	    }

	    rs = bprintf(ofp,"\" ;\n") ;
	    wlen += rs ;

/* truncate it to where we are (if STDOUT was already open -- usually) */

	    if ((rs = btell(ofp,&offset)) >= 0) {
	        rs = btruncate(ofp,offset) ;
	    } else if (rs == SR_NOTSEEK)
	        rs = SR_OK ;

	    bclose(ofp) ;
	} /* end if (writing the stuff) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (output) */


