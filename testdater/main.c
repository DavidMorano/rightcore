/* main */

/* test the DATER object */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_TZSET	1		/* play w/ |tzset(3c)| */
#define	CF_FTIME	0		/* play w/ |uc_ftime(3dam)| */
#define	CF_INITNULL	0		/* initialize |dater()| w/ NULL */
#define	CF_DUMPZINFO	0		/* dump zone-info */
#define	CF_NZONES	0		/* 'dater_nzones(3dam)' */
#define	CF_TESTSNTMTIME	0		/* test |sntmtime(3dam)| */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little subroutine tests the DATER object.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<tmtime.h>
#include	<sntmtime.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"dater.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	ZINFOFNAME	"zinfo"


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_gmlog(time_t,char *) ;


/* external variables */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* local structures */


/* forward references */

#if	CF_DUMPZINFO
static int	dumpzinfo(DATER *,const char *) ;
#endif /* CF_DUMPZINFO */


/* local variables */

static unsigned char	titerms[] = {
	0x00, 0x1B, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*types[] = {
	"std",
	"msg",
	"strdig",
	"logz",
	"gmlogz",
	"toucht",
	NULL
} ;

enum types {
	type_std,
	type_msg,
	type_strdig,
	type_logz,
	type_day,
	type_toucht,
	type_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	DATER		d ;
	FIELD		fsb ;
	time_t		daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		len, sl ;
	int		itype ;
	int		ex = EX_INFO ;
	const char	*cp ;
	char		linebuf[LINEBUFLEN + 1] ;
	char		lineout[LINEBUFLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	daytime = time(NULL) ;

/* test 'sntmtime(3dam)' */

#if	CF_TESTSNTMTIME
	if (isatty(fd_stdout)) {
	    TMTIME	tmt ;
	    if ((rs = tmtime_localtime(&tmt,daytime)) >= 0) {
	        fmt = "%a %b %d %H%:%M%:%S %Z %Y %O %n" ;
	        if ((rs = sntmtime(timebuf,TIMEBUFLEN,&tmt,fmt)) >= 0) {
	            len = rs ;
	            rs = uc_writen(fd_stdout,timebuf,len) ;
		}
	    }
	}
#endif /* CF_TESTSNTMTIME */

/* tzset(3c) */

#if	CF_TZSET
	tzset() ;
#if	CF_DEBUGS
	debugprintf("main: tzset() daylight=%d\n",daylight) ;
	debugprintf("main: tzset() timezone=%d(%um) altzone=%d(%um)\n",
		timezone,(timezone/60),altzone,(altzone/60)) ;
#endif
#endif /* CF_TZSET */

/* ftime(3c) */

#if	CF_FTIME
	{
		struct timeb	b ;
		uc_ftime(&b) ;
#if	CF_DEBUGS
	    debugprintf("main: uc_ftime() dstflag=%d timezone=%d\n",
		b.dstflag,b.timezone) ;
#endif
	}
#endif /* CF_FTIME */

	if (rs < 0) {	
	    ex = EX_DATAERR ;
	    goto ret0 ;
	}

/* other */

	rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto ret0 ;
	}

	rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;
	if (rs < 0) {
	    ex = EX_NOINPUT ;
	    goto ret1 ;
	}

	bprintf(ofp,"DATER object test program\n") ;

#ifdef	COMMENT
	bprintf(ofp,"daytime=%ld\n",daytime) ;
#endif

	bprintf(ofp,"current time=%s\n",
	    timestr_logz(daytime,timebuf)) ;

/* initial stuff */

#if	CF_INITNULL

	rs1 = dater_start(&d,NULL,NULL,0) ;

#else /* CF_INITNULL */

/* get the current time-of-day */

	{
	    struct timeb	now ;
	    TMTIME		tmt ;
	    char		zname[DATER_ZNAMESIZE + 1] ;

	    uc_ftime(&now) ;

	    rs1 = SR_INVALID ;
	    if ((rs = tmtime_localtime(&tmt,now.time)) >= 0) {
	        now.timezone = (tmt.gmtoff / 60) ;
	        now.dstflag = tmt.isdst ;
	        strncpy(zname,tmt.zname,DATER_ZNAMESIZE) ;
	        rs1 = dater_start(&d,&now,zname,-1) ;
	    }

	} /* end block (getting some current time stuff) */

#endif /* CF_INITNULL */

#if	CF_DEBUGS
	debugprintf("main: dater_start() rs=%d\n",rs1) ;
#endif

#if	CF_NZONES
	n = dater_nzones(&d) ;
#if	CF_DEBUGS
	debugprintf("main: dater_nzones() rs=%d\n",n) ;
#endif
#endif /* CF_NZONES */

#if	CF_DUMPZINFO
	dumpzinfo(&d,ZINFOFNAME) ;
	bprintf(ofp,"dater_nzones=%d\n",n) ;
#endif

/* loop stuff */

#if	CF_DEBUGS
	debugprintf("main: loop-about\n") ;
#endif

	len = 1 ;
	while (rs >= 0) {
	    int		dlen ;
	    int		fl ;
	    const char	*dp ;
	    const char	*fp ;

	    if (len > 0) {

	        bprintf(ofp,"date types:\n") ;

	        for (i = 0 ; i < type_overlast ; i += 1)
	            bprintf(ofp,"\t%d\t%s\n",i,types[i]) ;

	        bprintf(ofp,
	            "enter type and dater_string> ") ;

	    }

#ifdef	OPTIONAL
	    bflush(ofp) ;
#endif

	    rs = breadline(ifp,linebuf,LINEBUFLEN) ;
	    len = rs ;
	    if (rs <= 0)
	        break ;

	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;
	    cp = linebuf ;
	    while (CHAR_ISWHITE(*cp)) {
	        cp += 1 ;
	        len -= 1 ;
	    }

	    if (*cp == '\0')
	        continue ;

	    bprintf(ofp,"\n") ;

	    if ((rs = field_start(&fsb,linebuf,len)) >= 0) {

/* get the type */

	        if ((fl = field_get(&fsb,titerms,&fp)) > 0) {
		    const int	fch = MKCHAR(fp[0]) ;

#if	CF_DEBUGS
	        debugprintf("main: field=>%t<\n",
	            fp,MIN(fl,TIMEBUFLEN)) ;
#endif

	        if (! isdigitlatin(fch)) {
#if	CF_DEBUGS
	            debugprintf("main: raw type=>%t<\n",
	                fp,strlinelen(fp,fl,20)) ;
#endif

	            itype = matpcasestr(types,1,fp,fl) ;

#if	CF_DEBUGS
	            debugprintf("main: type=%d(%s)\n",
	                itype,
	                ((itype >= 0) && (itype < type_overlast)) ? 
	                types[itype] : "X") ;
#endif

	        } else
	            itype = (fp[0] - '0') ;

/* get the dater_string */

	   	dlen = field_remaining(&fsb,&dp) ;

	            while ((dlen > 0) && CHAR_ISWHITE(dp[dlen - 1]))
	                dlen -= 1 ;

	        if (dlen > 0) {

#ifdef	COMMENT
	            bprintf(ofp,"input=>%t<\n",
	                dp, ((dp[dlen - 1] == '\n') ? (dlen - 1) : dlen)) ;
#endif

	            if ((itype >= 0) && (itype < type_overlast)) {

	                bprintf(ofp,
	                    "converting type=%d(%s)\n",
	                    itype,types[itype]) ;

	                bprintf(ofp,
	                    "in=>%t<\n",
	                    dp,dlen) ;

	            } else
	                bprintf(ofp,"unknown type=%d\n",itype) ;

	            switch (itype) {

	            case type_std:
	                rs1 = dater_setstd(&d,dp,dlen) ;
	                break ;

	            case type_msg:
	                rs1 = dater_setmsg(&d,dp,dlen) ;
	                break ;

	            case type_strdig:
	                rs1 = dater_setstrdig(&d,dp,dlen) ;
	                break ;

	            case type_logz:
	                rs1 = dater_setlogz(&d,dp,dlen) ;
	                break ;

	            case type_toucht:
	                rs1 = dater_settoucht(&d,dp,dlen) ;
	                break ;

	            case type_day:
	            default:
	                rs = SR_NOTSUP ;
	                bprintf(ofp,"unknown input type\n") ;
			break ;

	            } /* end switch */

	            bprintf(ofp,"conversion result=%d\n",rs1) ;

	            if (rs1 >= 0) {

	                bprintf(ofp,"f_zoff=%u zoff=%dm (west of GMT)\n",
	                    d.f.zoff,
	                    d.b.timezone) ;

	                bprintf(ofp,"f_zname=%u zname=%t\n",
	                    d.f.zname,
	                    d.zname,strnlen(d.zname,DATER_ZNAMESIZE)) ;

	                bprintf(ofp,"dstflag=%d\n",
	                    d.b.dstflag) ;

	                bprintf(ofp,"loctime=%s\n",
	                    timestr_log(d.b.time,timebuf)) ;
	                bprintf(ofp,"gmttime=%s\n",
	                    timestr_gmlog(d.b.time,timebuf)) ;

	                for (i = 0 ; i < DATER_DTSEND ; i += 1) {

	                    sl = dater_mkdatestr(&d,i,lineout,LINEBUFLEN) ;

	                    bprintf(ofp,"date type=%u(%s) > %t <\n",
	                        i,types[i],
	                        lineout,sl) ;

	                } /* end for */

	            } else
	                bprintf(ofp,"bad conversion (%d)\n",rs1) ;

	        } else
	            bprintf(ofp,"no string given to convert\n") ;

		} /* end if (field-get) */

	        field_finish(&fsb) ;
	    } /* end if (field) */

	} /* end while */

	bprintf(ofp,"\n") ;

	dater_finish(&d) ;

	bclose(ifp) ;

ret1:
	bclose(ofp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


#if	CF_DUMPZINFO
static int dumpzinfo(DATER *dp,cchar *fname)
{
	bfile		zfile, *zfp = &zfile ;
	DATER_ZINFO	zi ;
	int		rs ;
	int		i ;
	int		wlen = 0 ;

	if (dp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = bopen(zfp,fname,"wct",0666)) >= 0) {

	    for (i = 0 ; dater_zinfo(dp,&zi,i) >= 0 ; i += 1) {

	        rs = bprintf(zfp,"%8s %6d %2d\n",zi.zname,zi.zoff,zi.isdst) ;
	        wlen += rs ;
	        if (rs < 0) break ;

	    } /* end for */

	    bclose(zfp) ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (dumpzinfo) */
#endif /* CF_DUMPZINFO */


