/* main */

/* test the DATE object */


#define	CF_DEBUGS	0		/* run-time debugging */
#define	CF_TZSET	1
#define	CF_LOCALTIME	1
#define	CF_MKTIME	1


/* revision history:

	= 2000-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"date.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern char	*timestr_logz(time_t,char *) ;


/* external variables */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* local variables */

static unsigned char	tterms[] = {
	                0x00, 0x1B, 0x00, 0x00,
	                0x01, 0x10, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00
} ;

static unsigned char	dterms[] = {
	                0x00, 0x1B, 0x00, 0x00,
	                0x01, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00,
	                0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int main()
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	DATE	d ;

	FIELD	f ;

	time_t	daytime ;

	int		rs = SR_OK ;
	int		i, n ;
	int	len, sl ;
	int	itype, otype ;
	int	fd_debug ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	lineout[LINEBUFLEN + 1] ;
	char	timebuf[50] ;
	char	cname[DATE_ZNAMESIZE] ;
	char	*cp ;


	if ((cp = getenv(DEBUGFDVAR1)) == NULL)
	    cp = getenv(DEBUGFDVAR2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    esetfd(fd_debug) ;


	bopen(ifp,BIO_STDIN,"dr",0666) ;

	bopen(ofp,BIO_STDOUT,"dwct",0666) ;


	bprintf(ofp,"DATE object test program\n") ;


	u_time(&daytime) ;

/* initial stuff */

	date_init(&d,NULL,NULL,0) ;

	n = date_nzones(&d) ;

	bprintf(ofp,"date_nzones=%d\n",n) ;

	bprintf(ofp,"daylight=%d timezone=%d altzone=%d\n",
	    daylight,timezone,altzone) ;

#if	CF_TZSET
	bprintf(ofp,"tzset()\n") ;

	tzset() ;

	bprintf(ofp,"daylight=%d timezone=%d altzone=%d\n",
	    daylight,timezone,altzone) ;
#endif /* CF_TZSET */

#if	CF_LOCALTIME
	bprintf(ofp,"localtime()\n") ;

	localtime(&daytime) ;

	bprintf(ofp,"daylight=%d timezone=%d altzone=%d\n",
	    daylight,timezone,altzone) ;
#endif

#if	CF_MKTIME
	{
	    struct tm	*tsp, ts ;

	    time_t		t ;


	    bprintf(ofp,"mktime()\n") ;

	    tsp = (struct tm *) localtime_r(&daytime,&ts) ;

	    daylight = 0 ;
	    timezone = 0 ;

	    rs = uc_mktime(tsp,&t) ;

	    bprintf(ofp,"mktime() rs=%d\n",rs) ;

	    bprintf(ofp,"daylight=%d timezone=%d altzone=%d\n",
	        daylight,timezone,altzone) ;

	    bprintf(ofp,"made time=%s\n",
	        timestr_log(t,timebuf)) ;

	    bprintf(ofp,"daylight=%d timezone=%d altzone=%d\n",
	        daylight,timezone,altzone) ;

	} /* end block */
#endif /* CF_MKTIME */


/* loop stuff */

	while (TRUE) {

	    int		dlen ;

	    char	*dp ;


	    bprintf(ofp,"date types :\n") ;

	    for (i = 0 ; i < 3 ; i += 1) {

	        switch (i) {

	        case 0:
	            bprintf(ofp,"\t%d\tSTD\n",i) ;

	            break ;

	        case 1:
	            bprintf(ofp,"\t%d\tMSG\n",i) ;

	            break ;

	        case 2:
	            bprintf(ofp,"\t%d\tLOGZ\n",i) ;

	            break ;

	        } /* end switch */

	    } /* end for */

	    bprintf(ofp,
	        "enter type and date_string> ") ;

	    if ((len = breadline(ifp,linebuf,LINEBUFLEN)) <= 0)
	        break ;

	    field_start(&f,linebuf,len) ;

/* get the type */

	    field_get(&f,tterms) ;

	    itype = f.fp[0] - '0' ;

	    if (f.term == ',') {
	        field_get(&f,tterms) ;
	        otype = f.fp[0] - '0' ;
	    } /* end if (getting output type) */

/* get the date_string */

#ifdef	COMMENT
	    field_get(&f,dterms) ;
#else
	    dp = f.lp ;
	    dlen = f.rlen ;
	    if (dlen > 0) {
	        while (CHAR_ISWHITE(dp[dlen - 1])) dlen -= 1 ;
	    }
#endif

	    if (dlen > 0) {

	        bprintf(ofp,
	            "converting string=>%W<\n",dp,dlen) ;

	        switch (itype) {

	        case 0:
	            rs = date_setstd(&d,dp,dlen) ;
	            break ;

	        case 1:

#if	CF_DEBUGS
	            eprintf("main: str=>%W<\n",dp,dlen) ;
#endif

	            rs = date_setmsg(&d,dp,dlen) ;
	            break ;

	        case 2:
	            rs = date_setlogz(&d,dp,dlen) ;
	            break ;

	        default:
	            rs = SR_NOTSUP ;
	            bprintf(ofp,"unknown input type\n") ;
		    break ;

	        } /* end switch */

	        bprintf(ofp,"date_setxxx() rs=%d\n",rs) ;

	        bprintf(ofp,"daylight=%d timezone=%d altzone=%d\n",
	            daylight,timezone,altzone) ;

	        if (rs >= 0) {

	            bprintf(ofp,"f_zoff=%d\n",
	                d.f.zoff) ;

	            bprintf(ofp,"dstflag=%d\n",
	                d.b.dstflag) ;

	            bprintf(ofp,"local %s\n",
	                timestr_logz(d.b.time,timebuf)) ;

	            for (i = 0 ; i < DATE_DTSEND ; i += 1) {

	                sl = date_mkdatestr(&d,i,lineout,LINEBUFLEN) ;

#if	CF_DEBUGS
	                eprintf("main: daylight=%d timezone=%d altzone=%d\n",
	                    daylight,timezone,altzone) ;
#endif

	                bprintf(ofp,"date type=%d > %W <\n",
	                    i,lineout,sl) ;

	            } /* end for */

	        } else
	            bprintf(ofp,"bad conversion (rs %d)\n",rs) ;

	    } else
	        bprintf(ofp,"no string given to convert\n") ;

	    field_finish(&f) ;
	} /* end while */

	bprintf(ofp,"\n") ;



	date_free(&d) ;

	bclose(ofp) ;

	return EX_OK ;
}
/* end subroutine (main) */



