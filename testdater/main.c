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

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

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

static int	process(PROGINFO *,DATER *,cchar *) ;
static int	procin(PROGINFO *,DATER *,bfile *) ;
static int	procline(PROGINFO *,DATER *,bfile *,cchar *,int) ;

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


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	DATER		d ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_INFO ;
	cchar		*ofn = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->intpoll = -1 ;
	pip->daytime = time(NULL) ;

/* test 'sntmtime(3dam)' */

#if	CF_TESTSNTMTIME
	if (isatty(fd_stdout)) {
	    TMTIME	tmt ;
	    if ((rs = tmtime_localtime(&tmt,pip->daytime)) >= 0) {
	        char	tbuf[TIMEBUFLEN+1] ;
	        fmt = "%a %b %d %H%:%M%:%S %Z %Y %O %n" ;
	        if ((rs = sntmtime(tbuf,TIMEBUFLEN,&tmt,fmt)) >= 0) {
	            len = rs ;
	            rs = uc_writen(fd_stdout,tbuf,len) ;
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

/* other */

#ifdef	COMMENT

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

	if ((rs = dater_start(&d,NULL,NULL,0)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: dater_start() rs=%d\n",rs) ;
#endif

#if	CF_NZONES
	    n = dater_nzones(&d) ;
#if	CF_DEBUGS
	    debugprintf("main: dater_nzones() rs=%d\n",n) ;
#endif
#endif /* CF_NZONES */

#if	CF_DUMPZINFO
	    dumpzinfo(&d,ZINFOFNAME) ;
#endif

/* loop stuff */

#if	CF_DEBUGS
	    debugprintf("main: loop-about\n") ;
#endif

	    rs = process(pip,&d,ofn) ;

	    rs1 = dater_finish(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (dater) */

#if	CF_DEBUGS
	debugprintf("main: dater-out rs=%d\n",rs) ;
#endif

/* done */

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int process(PROGINFO *pip,DATER *dp,cchar *ofn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0')) ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    char	tbuf[TIMEBUFLEN+1] ;

	    bprintf(ofp,"DATER object test program\n") ;

#ifdef	COMMENT
	    bprintf(ofp,"daytime=%ld\n",pip->daytime) ;
#endif

	    bprintf(ofp,"current time=%s\n",
	        timestr_logz(pip->daytime,tbuf)) ;

/* initial stuff */

	    rs = procin(pip,dp,ofp) ;
	    wlen = rs ;

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile-output) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procin(PROGINFO *pip,DATER *dp,bfile *ofp)
{
	bfile		ifile, *ifp = &ifile ;
	int		rs ;
	int		rs1 ;
	cchar		*ifn = BFILE_STDIN ;
	cchar		*fmt ;

	if ((rs = bopen(ifp,ifn,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		i ;
	    int		cl ;
	    cchar	*cp ;
	    char	lbuf[LINEBUFLEN+1] ;

	    while (rs >= 0) {

	        bprintf(ofp,"date types:\n") ;
	        for (i = 0 ; i < type_overlast ; i += 1) {
	            bprintf(ofp,"\t%d\t%s\n",i,types[i]) ;
	        }
	        fmt = "enter type and dater_string> " ;
	        bprintf(ofp,fmt) ;

	        rs = breadline(ifp,lbuf,llen) ;
	        len = rs ;
	        if (rs <= 0) break ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;

	        if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	            if (cp[0] != '#') {
	                rs = procline(pip,dp,ofp,lbuf,cl) ;
	            }
	        }

	        bprintf(ofp,"\n") ;

	    } /* end while */

	    bprintf(ofp,"\n") ;

	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile-input) */

	return rs ;
}
/* end subroutine (procin) */


static int procline(PROGINFO *pip,DATER *dp,bfile *ofp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		itype = -1 ;
	    int		rlen ;
	    int		fl ;
	    cchar	*fmt ;
	    cchar	*fp ;
	    cchar	*rbuf ;

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

	        } else {
	            itype = (fp[0] - '0') ;
	        }

/* get the dater_string */

	        rlen = field_remaining(&fsb,&rbuf) ;

	        while ((rlen > 0) && CHAR_ISWHITE(rbuf[rlen - 1])) {
	            rlen -= 1 ;
	        }

	        if (rlen > 0) {

#ifdef	COMMENT
	            bprintf(ofp,"input=>%t<\n",
	                rbuf,((rbuf[rlen-1] == '\n') ? (rlen-1) : rlen)) ;
#endif

	            if ((itype >= 0) && (itype < type_overlast)) {
	                fmt = "converting type=%d(%s)\n" ;
	                bprintf(ofp,fmt,itype,types[itype]) ;
	                bprintf(ofp, "in=>%t<\n",rbuf,rlen) ;
	            } else {
	                bprintf(ofp,"unknown type=%d\n",itype) ;
	            }

	            switch (itype) {
	            case type_std:
	                rs1 = dater_setstd(dp,rbuf,rlen) ;
	                break ;
	            case type_msg:
	                rs1 = dater_setmsg(dp,rbuf,rlen) ;
	                break ;
	            case type_strdig:
	                rs1 = dater_setstrdig(dp,rbuf,rlen) ;
	                break ;
	            case type_logz:
	                rs1 = dater_setlogz(dp,rbuf,rlen) ;
	                break ;
	            case type_toucht:
	                rs1 = dater_settoucht(dp,rbuf,rlen) ;
	                break ;
	            case type_day:
	            default:
	                rs = SR_NOTSUP ;
	                bprintf(ofp,"unknown input type\n") ;
	                break ;
	            } /* end switch */

	            bprintf(ofp,"conversion result=%d\n",rs1) ;

	            if (rs1 >= 0) {
	                const int	olen = LINEBUFLEN ;
	                int		i ;
	                int		sl ;
	                char		obuf[LINEBUFLEN+1] ;
	                char		tbuf[TIMEBUFLEN+1] ;

	                bprintf(ofp,"f_zoff=%u zoff=%dm (west of GMT)\n",
	                    dp->f.zoff,
	                    dp->b.timezone) ;

	                bprintf(ofp,"f_zname=%u zname=%t\n",
	                    dp->f.zname,
	                    dp->zname,strnlen(dp->zname,DATER_ZNAMESIZE)) ;

	                bprintf(ofp,"dstflag=%d\n",
	                    dp->b.dstflag) ;

	                bprintf(ofp,"loctime=%s\n",
	                    timestr_log(dp->b.time,tbuf)) ;
	                bprintf(ofp,"gmttime=%s\n",
	                    timestr_gmlog(dp->b.time,tbuf)) ;

	                for (i = 0 ; i < DATER_DTSEND ; i += 1) {

	                    sl = dater_mkdatestr(dp,i,obuf,olen) ;

	                    bprintf(ofp,"date type=%u(%s) > %t <\n",
	                        i,types[i],obuf,sl) ;

	                } /* end for */

	            } else
	                bprintf(ofp,"bad conversion (%d)\n",rs1) ;

	        } else
	            bprintf(ofp,"no string given to convert\n") ;

	    } /* end if (field-get) */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return rs ;
}
/* end subroutine (procline) */


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


