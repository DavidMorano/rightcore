/* main (bandate) */

/* whole program for BANDATE */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debugging memory allocations */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Write ("date") the current date on the banner read on STDIN.

	Synopsis:

	$ banner <string> | bandate 


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

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
extern int	bwriteblanks(bfile *,int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnprbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	process(cchar *,cchar *,cchar *,int) ;
static int	trailprint(bfile *,cchar *,cchar *,int) ;
static int	subprint(bfile *,cchar *,cchar *,int) ;


/* local variables */

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	TMTIME		tm ;
	time_t		daytime = time(NULL) ;

#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	const int	dlen = TIMEBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_OK ;
	int		wlen = 0 ;
	int		f_top = FALSE ;

	const char	*tspec = "%e %b %T" ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = BFILE_STDIN ;
	const char	*cp ;
	char		dbuf[TIMEBUFLEN+1] ;


#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((argc > 1) && (argv[1] != '\0')) {
	    cp = argv[1] ;
	    f_top = (cp[0] == 't') ;
	}

	if ((rs = tmtime_gmtime(&tm,daytime)) >= 0) {
	    rs = sntmtime(dbuf,dlen,&tm,tspec) ;
	}

#if	CF_DEBUGS
	debugprintf("main: dstr=%s\n",dbuf) ;
#endif

	if (rs >= 0) {
	    rs = process(NULL,ifname,dbuf,f_top) ;
	    wlen = rs ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	}

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int process(cchar *ofn,cchar *ifn,cchar *dstr,int f_top)
{
	BFILE		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    BFILE	ifile, *ifp = &ifile ;

	    if ((rs = bopen(ifp,ifn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
		int		line = 0 ;
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
	                rs = bprintln(ofp,lbuf,len) ;
	                wlen += rs ;
	            }

	            line += 1 ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (input-file-open) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (output-file-open) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int trailprint(bfile *ofp,cchar *dstr,cchar *lbuf,int llen)
{
	const int	cols = COLUMNS ;
	const int	dl = strlen(dstr) ;
	int		rs = SR_OK ;
	int		breaklen ;
	int		ml ;
	int		i = 0 ;
	int		wlen = 0 ;

	breaklen = (cols - dl) ;

	if ((rs >= 0) && (i < breaklen)) {
	    ml = MIN(llen,breaklen) ;
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


static int subprint(bfile *ofp,cchar *dstr,cchar *lbuf,int len)
{
	int		rs = SR_OK ;
	int		di = 0 ;
	int		ch ;
	int		i ;
	int		wlen = 0 ;

	for (i = 0 ; (i < len) && lbuf[i] ; i += 1) {
	    ch = MKCHAR(lbuf[i]) ;
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

