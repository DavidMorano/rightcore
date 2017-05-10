/* main (bandate) */

/* whole program for BANDATE */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Write ("date") the current date on the banner read on STDIN.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<tmtime.h>
#include	<sntmtime.h>
#include	<bfile.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif


/* external subroutines */

extern int	sfnext(const char *,int,const char **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnprbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int trailprint(bfile *,const char *,const char *,int) ;
static int subprint(bfile *,const char *,const char *,int) ;


/* local variables */


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	BFILE	ofile, *ofp = &ofile ;

	TMTIME	tm ;

	time_t	daytime = time(NULL) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	rs = SR_OK ;
	int	rs1 ;
	int	pan = 0 ;
	int	ex = EX_OK ;
	int	line = 0 ;
	int	wlen = 0 ;
	int	f_top = TRUE ;

	const char	*tspec = "%e %b %T" ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = BFILE_STDIN ;
	const char	*cp ;

	char	dstr[TIMEBUFLEN+1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	        cp = getourenv(envv,VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((argc > 1) && (argv[1] != '\0')) {
	    cp = argv[1] ;
	    f_top = (cp[0] == 't') ;
	}

	rs = tmtime_gmtime(&tm,daytime) ;

	if (rs >= 0)
	    rs = sntmtime(dstr,TIMEBUFLEN,&tm,tspec) ;

#if	CF_DEBUGS
	debugprintf("main: dstr=%s\n",dstr) ;
#endif

	if (rs < 0) goto badtime ;

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {
	    BFILE	ifile, *ifp = &ifile ;

	    if ((rs = bopen(ifp,ifname,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN+1] ;

	        while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len-1] == '\n') lbuf[--len] = '\0' ;

	            if (f_top && (line == 0)) {
	                rs = trailprint(ofp,dstr,lbuf,len) ;
	            } else if ((! f_top) && (line == 5)) {
	                rs = subprint(ofp,dstr,lbuf,len) ;
	                wlen += rs ;
	            } else {
	                rs = bprintline(ofp,lbuf,len) ;
	                wlen += rs ;
	            }

	            line += 1 ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(ifp) ;
	    } /* end if (input-file-open) */

	    bclose(ofp) ;
	} /* end if (output-file-open) */

badtime:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int trailprint(bfile *ofp,const char *dstr,const char *lbuf,int len)
{
	const int	cols = COLUMNS ;
	const int	dl = strlen(dstr) ;

	int	rs = SR_OK ;
	int	breaklen ;
	int	ml ;
	int	i = 0 ;
	int	wlen = 0 ;

	breaklen = (cols - dl) ;

	if ((rs >= 0) && (i < breaklen)) {
	    ml = MIN(len,breaklen) ;
	    rs = bwrite(ofp,(lbuf+i),ml) ;
	    wlen += rs ;
	    i += rs ;
	}

	if ((rs >= 0) && (i < breaklen)) {
	    ml = (breaklen-i) ;
	    rs = bwriteblanks(ofp,ml) ;
	    wlen += rs ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = bwrite(ofp,dstr,dl) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bputc(ofp,'\n') ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (trailprint) */


static int subprint(bfile *ofp,const char *dstr,const char *lbuf,int len)
{
	int	rs = SR_OK ;
	int	di = 0 ;
	int	ch ;
	int	i ;
	int	wlen = 0 ;

	for (i = 0 ; (i < len) && lbuf[i] ; i += 1) {
	    ch = lbuf[i] & 0xff ;
	    if ((! CHAR_ISWHITE(ch)) && (dstr[di] != '\0')) {
	        if (dstr[di] != ' ') ch = dstr[di] ;
	        di += 1 ;
	    }
	    rs = bputc(ofp,ch) ;
	    wlen += rs ;
	} /* end for */

	if (rs >= 0) {
	    rs = bputc(ofp,'\n') ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subprint) */


