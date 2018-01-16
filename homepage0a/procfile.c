/* procfile */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_FOOTER	1		/* include a footer? */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


	= 2004-02-09, David A­D­ Morano

	No it wasn't (referring to the above).	I snarfed this from
	another program with that previous change note! :-)  Almost every
	subroutine starts from a previous one.	Why do we keep the old
	change notes?

	Also, what was the previous program by the same name?
	Never mind, it doesn't really matter since I have gutted
	this subroutine almost to the bone! :-)


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine processes one file at a time.  


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<char.h>
#include	<field.h>
#include	<sbuf.h>
#include	<realname.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"procfile.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		(2 * 1024)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	MAXLINELEN
#define	MAXLINELEN	76
#endif

#ifndef	FNAMESUF
#define	FNAMESUF	"htm"
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	bprintlns(bfile *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t, char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	sihyphen(const char *,int) ;
static int	artdate(int,const char *,int,time_t *) ;
static int	mkrealname(char *,int,const char *,int) ;


/* local variables */

static const char	*fulldays[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday", NULL
} ;

static const char	*fullmonths[] = {
	"January", "February", "March", "April", "May", "June", 
	"July", "August", "September", "October", "November", "December", 
	NULL
} ;

static const char	*days[] = {
	"sun", "mon", "tue", "wed", "thu", "fri", "sat", NULL
} ;

static const char	*months[] = {
	"jan", "feb", "mar", "apr", "may", "jun", 
	"jul", "aug", "sep", "oct", "nov", "dec", NULL
} ;

enum states {
	state_begin,
	state_title,
	state_author,
	state_date,
	state_publisher,
	state_body,
	state_overlast
} ;


/* exported subroutines */


int procfile(pip,pfp,fname)
struct proginfo	*pip ;
struct procfile	*pfp ;
const char	fname[] ;
{
	struct ustat	sb ;

	SBUF	b ;

	bfile	ifile, ofile ;

	time_t	stime, dtime ;

	int	rs, rs1 ;
	int	si, len, ml, cl, bl ;
	int	pstate, state = state_begin ;
	int	lines ;
	int	c ;
	int	f_bol, f_eol ;
	int	f_write = FALSE ;
	int	f_par = FALSE ;
	int	f_blockquote = FALSE ;
	int	f_artinfo = FALSE ;

	const char	*cp ;

	char	buf[BUFLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (pfp == NULL)
	    return SR_FAULT ;

	memset(pfp,0,sizeof(struct procfile)) ;

	if (strcmp(fname,"-") != 0) {
	    rs = bopen(&ifile,fname,"r",0666) ;
	} else
	    rs = bopen(&ifile,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    return rs ;

/* see if we have to make the HTM version */

	cl = -1 ;
	if ((cp = strrchr(fname,'.')) != NULL)
	    cl = (cp - fname) ;

	ml = (cl > 0) ? MIN(cl,MAXNAMELEN) : MAXNAMELEN ;
	strwcpy(buf,fname,ml) ;

	mkfnamesuf1(pfp->htmfname,buf,FNAMESUF) ;

	rs = u_stat(pfp->htmfname,&sb) ;
	f_write = (rs < 0) ;
	if (! f_write) {

	    dtime = sb.st_mtime ;
	    bcontrol(&ifile,BC_STAT,&sb) ;

	    stime = sb.st_mtime ;
	    f_write = (stime > dtime) ;
	}

/* open any necessary destination file */

	if (f_write) {

	    rs = bopen(&ofile,pfp->htmfname,"wct",0666) ;

	    f_write = (rs >= 0) ;

	}

/* if writing a destination put out any header information */

	if (f_write) {

	    bprintf(&ofile,"<html>\n") ;

	    bprintf(&ofile,"<head>\n") ;

	    bprintf(&ofile,"<title>\n") ;

	    if (pip->article_title[0] != '\0')
	        bprintf(&ofile,"%s\n",pip->article_title) ;

	    else if (pip->site_title[0] != '\0')
	        bprintf(&ofile,"%s\n",pip->site_title) ;

	    bprintf(&ofile,"</title>\n") ;

	    bprintf(&ofile,"</head>\n") ;

	    bprintf(&ofile,"<body>\n") ;

	} /* end if (writing) */

/* start reading through the file */

	lines = 0 ;
	f_bol = TRUE ;
	while ((rs = breadline(&ifile,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    f_eol = (linebuf[len - 1] == '\n') ;
	    if (f_eol)
	        len -= 1 ;

	    linebuf[len] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("procfile: line=>%t<\n",linebuf,MIN(len,40)) ;
#endif

	    cl = sfshrink(linebuf,len,&cp) ;

	    switch (state) {

	    case state_begin:
	        if (linebuf[0] == '\0')
	            break ;

	        pstate = state ;
	        state = state_title ;

/* FALLTHROUGH */
	    case state_title:
	        if (pstate != state) {

	            c = 0 ;
	            sbuf_start(&b,buf,BUFLEN) ;

	        }

	        pstate = state ;
	        if (cl > 0) {

	            if (c++ > 0)
	                sbuf_char(&b,' ') ;

	            sbuf_strw(&b,cp,cl) ;

	        } else {

	            bl = sbuf_getlen(&b) ;

	            si = sihyphen(buf,bl) ;

	            if (si < 0)
	                si = bl ;

	            snwcpy(pfp->title,PROCFILE_LITEM,buf,si) ;

	            if (f_write && (bl > 0)) {

	                bprintf(&ofile,"<h1>\n") ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("procfile: title=>%t<\n",buf,bl) ;
#endif

	                bprintlns(&ofile,LINEFOLDLEN,buf,bl) ;

	                bprintf(&ofile,"</h1>\n") ;

	                f_artinfo = TRUE ;
	                bprintf(&ofile,"<p>\n") ;

	            }

	            sbuf_finish(&b) ;

	            state = state_author ;

	        }

	        break ;

	    case state_author:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	            debugprintf("procfile: author?\n") ;
#endif

	        if (cp[0] == '.')
	            break ;

	        while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	            cp += 1 ;
	            cl -= 1 ;
	        }

	        if (strncasecmp(cp,"by",2) == 0) {
	            cp += 2 ;
	            cl -= 2 ;
	        } else
	            pfp->f.noauthor = TRUE ;

	        while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	            cp += 1 ;
	            cl -= 1 ;
	        }

	        if (cl > 0) {

		    char	namebuf[REALNAMELEN + 1] ;


	            if (! pfp->f.noauthor) {
	
			rs1 = mkrealname(namebuf,REALNAMELEN,cp,cl) ;

			if (rs1 >= 0) {
			    cp = namebuf ;
			    cl = rs1 ;
			}

		    }

		    snwcpy(pfp->author,PROCFILE_LITEM,cp,cl) ;

	            if (f_write && (cl > 0)) {

	                if (! pfp->f.noauthor)
	                    bprintf(&ofile,"by ") ;

	                bprintlns(&ofile,LINEFOLDLEN,cp,cl) ;

	            }

	        }

	        pstate = state ;
	        state = state_date ;
	        break ;

	    case state_date:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	            debugprintf("procfile: date?\n") ;
#endif

	        if (cp[0] == '.')
	            break ;

	        rs1 = artdate(pip->cyear,cp,cl,&pfp->date) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	            debugprintf("procfile: artdate() rs=%d\n",rs) ;
#endif

	        if (rs1 < 0)
	            pfp->date = 0 ;

	        if (f_write && (pfp->date != 0)) {

	            struct tm	ts ;

	            uint	year ;


	            rs1 = uc_localtime(&pfp->date,&ts) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	            debugprintf("procfile: uc_localtime() rs=%d\n",rs) ;
	            debugprintf("procfile: date=>%s<\n",
	                timestr_log(pfp->date,timebuf)) ;
	}
#endif

	            if (rs1 >= 0) {

	                bprintf(&ofile,"<br>\n") ;

	                year = TM_YEAR_BASE + ts.tm_year ;
	                bprintf(&ofile,"%s %s %u, %u\n",
	                    fulldays[ts.tm_wday],
	                    fullmonths[ts.tm_mon],
	                    ts.tm_mday,year) ;

	            }
	        }

	        pstate = state ;
	        state = state_publisher ;
	        break ;

	    case state_publisher:
	        if (cp[0] == '.')
	            break ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	        debugprintf("procfile: publisher s=%u ps=%u\n",
	            state,pstate) ;
#endif

	        if (pstate != state) {

	            c = 0 ;
	            sbuf_start(&b,buf,BUFLEN) ;

	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	        debugprintf("procfile: publisher cl=%d\n",cl) ;
#endif

	        pstate = state ;
	        if (cl > 0) {

	            if (c++ > 0)
	                sbuf_char(&b,' ') ;

	            sbuf_strw(&b,cp,cl) ;

	        } else {

	            bl = sbuf_getlen(&b) ;

		    snwcpy(pfp->publisher,PROCFILE_LITEM,buf,bl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	            debugprintf("procfile: publisher=%s\n",
	                pfp->publisher) ;
#endif

	            if (f_write && (bl > 0)) {

	                bprintf(&ofile,"<br>\n") ;

	                bprintlns(&ofile,LINEFOLDLEN,buf,bl) ;

	                f_artinfo = FALSE ;
	                bprintf(&ofile,"</p>\n") ;

	            }

	            sbuf_finish(&b) ;

	            state = state_body ;

	        }

	        break ;

	    case state_body:
	        if (f_artinfo) {

	            f_artinfo = FALSE ;
	            bprintf(&ofile,"</p>\n") ;

	        }

	        if (pstate != state)
	            bprintf(&ofile,"<pre>\n\n</pre>\n") ;

	        if (cl > 0) {

	            if (f_bol && (strcmp(cp,".QS") == 0)) {

	                f_blockquote = TRUE ;
	                bprintf(&ofile,"<blockquote>\n") ;

	            } else if (f_bol && (strcmp(cp,".QE") == 0)) {

	                if (f_par) {
	                    f_par = FALSE ;
	                    bprintf(&ofile,"</p>\n") ;
	                }

			if (f_blockquote) {
	                    f_blockquote = FALSE ;
	                    bprintf(&ofile,"</blockquote>\n") ;
			}

	            } else if (cp[0] != '.') {

	                if (! f_par) {
	                    f_par = TRUE ;
	                    bprintf(&ofile,"<p>\n") ;
	                }

	                bwrite(&ofile,cp,cl) ;

	                bputc(&ofile,'\n') ;

	            }

	        } else {

	            if (f_par) {
	                f_par = FALSE ;
	                bprintf(&ofile,"</p>\n") ;
	            }

	        }

	        pstate = state ;
	        break ;

	    } /* end switch */

	    if ((! f_write) && (state == state_body))
	        break ;

	    lines += 1 ;
	    f_bol = f_eol ;

	} /* end while (reading lines) */

	if (f_write) {

	    bprintf(&ofile,"<hr>\n") ;

	    bprintf(&ofile,"<p>\n") ;

	    bprintf(&ofile,"The raw text of this article is\n") ;

	    bprintf(&ofile,"<a href=\"%s\"> also available. </a>\n",
	        fname) ;

	    bprintf(&ofile,"</p>\n") ;

#if	CF_FOOTER
	    if (pip->site_title[0] != '\0') {

	        bprintf(&ofile,"<hr>\n") ;

	        bprintf(&ofile,"<center>\n") ;

	        bprintf(&ofile,"_ <a href=\"../index.htm\">home</a> _ \n") ;

	        bprintf(&ofile,"_ <a href=\"index.htm\">articles</a> _ \n") ;

	        bprintf(&ofile,"</center>\n") ;

	        bprintf(&ofile,"<br>\n") ;

	        bprintf(&ofile,"<center>\n") ;

	        bprintf(&ofile,"%s\n",pip->site_title) ;

	        bprintf(&ofile,"</center>\n") ;

	    }
#endif /* CF_FOOTER */

	    bprintf(&ofile,"</body>\n") ;

	    bprintf(&ofile,"</html>\n") ;

	    bclose(&ofile) ;

	} /* end if (needed to write a file) */

	bclose(&ifile) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procfile) */



/* LOCAL SUBROUTINES */



/* find string index of a hyphen (minus) character */
static int sihyphen(s,sl)
const char	s[] ;
int		sl ;
{
	int	i ;


	for (i = 0 ; (i < sl) && s[i] ; i += 1) {

	    if ((s[i] == '-') && ((i + 1) < sl) && (s[i + 1] == '-'))
	        break ;

	} /* end for */

	return ((i < sl) && s[i]) ? i : -1 ;
}
/* end subroutine (sihyphen) */


/* find a date for the article */
static int artdate(cy,s,slen,rtp)
int		cy ;
const char	s[] ;
int		slen ;
time_t		*rtp ;
{
	struct tm	ts ;

	int	rs, cl ;
	int	mon, day, year ;

	char	*tp, *cp ;


#if	CF_DEBUGS
	debugprintf("artdate: s=>%t<\n",s,slen) ;
#endif

	while ((slen > 0) && CHAR_ISWHITE(*s)) {
	    s += 1 ;
	    slen -= 1 ;
	}

/* is there a day?, if so ignore it */

	if (slen > 3) {

	    if (matcasestr(days,s,3) >= 0) {
	        s += 3 ;
	        slen -= 3 ;
	        while ((slen > 0) && (! CHAR_ISWHITE(*s))) {
	            s += 1 ;
	            slen -= 1 ;
	        }
	    }

	}

	while ((slen > 0) && CHAR_ISWHITE(*s)) {
	    s += 1 ;
	    slen -= 1 ;
	}

/* is there a month, grab it */

	mon = -1 ;
	if (slen > 3) {

	    if ((mon = matcasestr(months,s,3)) >= 0) {
	        s += 3 ;
	        slen -= 3 ;
	        while ((slen > 0) && (! CHAR_ISWHITE(*s))) {
	            s += 1 ;
	            slen -= 1 ;
	        }
	    }

#if	CF_DEBUGS
	    debugprintf("artdate: mon=%d\n",mon) ;
#endif

	}

	while ((slen > 0) && CHAR_ISWHITE(*s)) {
	    s += 1 ;
	    slen -= 1 ;
	}

/* is there a day of month (numeric) */

	day = -1 ;
	if (slen > 1) {

	    if ((tp = strnpbrk(s,slen," \t,")) != NULL) {

	        if (cfdeci(s,(tp - s),&day) < 0)
	            day = -1 ;

#if	CF_DEBUGS
	        debugprintf("artdate: mday=%d\n",day) ;
#endif

	        slen -= (tp - s) ;
	        s = (const char *) tp ;
	        if ((slen > 0) && (! CHAR_ISWHITE(s[0]))) {
	            s += 1 ;
	            slen -= 1 ;
	        }
	    }

	}

	while ((slen > 0) && CHAR_ISWHITE(*s)) {
	    s += 1 ;
	    slen -= 1 ;
	}

/* get the year */

#if	CF_DEBUGS
	debugprintf("artdate: rstr=%t\n",s,slen) ;
#endif

	year = -1 ;
	if (slen > 1) {

	    cp = (char *) s ;
	    cl = slen ;
	    if ((tp = strnpbrk(s,slen," (")) != NULL) {
		cl = (tp - s) ;
		slen -= cl ;

#if	CF_DEBUGS
	debugprintf("artdate: new rstr=%t\n",cp,cl) ;
#endif

	    }

	    if (cfdeci(cp,cl,&year) < 0)
	        year = -1 ;

	}

#if	CF_DEBUGS
	debugprintf("artdate: year=%d\n",year) ;
#endif

/* make the date */

	memset(&ts,0,sizeof(struct tm)) ;

	if (year >= 0) {

	    if (year >= 2000) {

#if	CF_DEBUGS
	        debugprintf("artdate: year GT 2000\n") ;
#endif

	        year -= TM_YEAR_BASE ;

#if	CF_DEBUGS
	        debugprintf("artdate: GT-200 year=%d\n",year) ;
#endif

	    } else if (year < 70)
	        year += 100 ;

	} else
	    year = cy ;

#if	CF_DEBUGS
	debugprintf("artdate: adjusted year=%d\n",year) ;
#endif

	ts.tm_year = year ;
	ts.tm_mon = mon ;
	ts.tm_mday = day ;
	ts.tm_hour = 12 ;

	rs = uc_mktime(&ts,rtp) ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("artdate: uc_mktime() rs=%d\n",rs) ;
	    debugprintf("artdate: date=>%s<\n",
	        timestr_log(*rtp,timebuf)) ;
	}
	    debugprintf("artdate: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (artdate) */


static int mkrealname(namebuf,namelen,sp,sl)
char		namebuf[] ;
int		namelen ;
const char	*sp ;
int		sl ;
{
	REALNAME	rn ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("mkrealname: name=>%t<\n",sp,sl) ;
#endif

	if ((rs = realname_start(&rn,sp,sl)) >= 0) {

		rs = realname_name(&rn,namebuf,namelen) ;

		realname_finish(&rn) ;

	}

#if	CF_DEBUGS
	debugprintf("mkrealname: ret rs=%d name=>%t<\n",
		rs,namebuf,rs) ;
#endif

	return rs ;
}
/* end subroutine (mkrealname) */



