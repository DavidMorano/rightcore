/* scanner */

/* scan the message for the primary information fields to be displayed */


#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_FIXFROM	1		/* the the FROM address */
#define	CF_FIXSUBJECT	1		/* fix subject in scan line */


/* revision history:

	= 94/01/06, David A­D­ Morano
	I modified from original to better parse the "from" 

	= 96/06/18, David A­D­ Morano
	I finally added Walter Pitio's algorithm for displaying
	line numbers larger than 999.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine scans the message for whatever.

        This subroutine creates the brief synopsis of the message, giving:
        number, sender, subject, date, length. We currently assume that the line
        is formatted into 80 characters (scanlong) or 50 characters (scanshort).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<curses.h>
#include	<stdio.h>

#include	<bfile.h>
#include	<mailmsg.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mb.h"
#include	"ds.h"


/* local defines */

#define	FROMWIDTH	18

#ifdef	NULL
#define	N	NULL
#else
#define	N	((char *) 0)
#endif


/* external subroutines */

extern int	getgecos() ;
extern int	fixfrom(struct proginfo *,char *,char *,int) ;
extern int	mkdisplayable(char *,int,const char *,int) ;

extern char	strwcpy(char *,const char *,int) ;
extern char	*strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_scandate(time_t,char *) ;


/* forward references */

int		fetchfrom(struct proginfo *,int,char *,int) ;

static int	scantitle(struct proginfo *,DS *) ;
static int	scanlong(struct proginfo *,DS *,struct mailbox *,int,int) ;
static int	scanshort(struct proginfo *,DS *,struct mailbox *,int,int) ;
static int	compressname(), compressdate() ;


/* external variables */

extern struct mailbox	mb ;


/* glabol variables */


/* local variables */

static const char	blanks[] = "          " ;

static const char	*days[] = {
	    "sun",
	    "mon",
	    "tue",
	    "wed",
	    "thu",
	    "fri",
	    "sat",
	    NULL
} ;


/* exported subroutines */


int scanner(pip,dsp,mbp,mn,line)
struct proginfo	*pip ;
DS		*dsp ;
struct mailbox	*mbp ;
int	mn ;
int	line ;
{
	int	rs = OK ;
	int	pv ;


#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf("scanner: ent w/ mn=%d line=%d\n",mn,line) ;

	    malloctest(26) ;

	}
#endif /* CF_DEBUG */

	if (mn < 0) {

	    scantitle(pip,dsp) ;		/* identify scanline fields */

	} else if ((mn < 0) || (mn >= mb.total)) {

	    wmove(dsp->w_header,line,0) ;

	    wclrtoeol(dsp->w_header) ;

	} else {

/* scan the mailbox */

	    pv = profile("long_scan") ;

	    if ((pv < 0) || (pv > 0)) {
	        rs = scanlong(pip,dsp,mbp,mn,line) ;

	    } else
	        rs = scanshort(pip,dsp,mbp,mn,line) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("scanner: exiting \n") ;
#endif

	return rs ;
}
/* end subroutine (scanner) */


/* local subroutines */


static int scanlong(pip,dsp,mbp,mn,wline)
struct proginfo	*pip ;
DS		*dsp ;
struct mailbox	*mbp ;
int	mn ;
int	wline ;
{
	time_t	envtime, msgtime ;

	MSG	m ;

	DATER	d ;

	int	rs, rs1 ;
	int	i, mi ;

	char	scanline[LINEBUFLEN + 1], *slp = scanline ;
	char	string[LINEBUFLEN + 1] ; /* getfield returns values here */ 
	char	work[LINEBUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1], timebuf2[TIMEBUFLEN + 1] ;
	char	*env_from = NULL ;
	char	*sp ;
	char	*cp, *cp2, *cp3 ;


	dater_start(&d,&pip->now,pip->zname,-1) ;

	mi = messord[mn] ;

	*slp = '\0' ;
	if (messdel[mi] == 1)
	    slp += sprintf(slp,"* ") ;

	else 
	    slp += sprintf(slp,"  ") ;


/* get information on this message */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("scanlong: get envelope information\n") ;
#endif

	bseek(mbp->mfp,messbeg[mi],SEEK_SET) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf("scanner: seeked\n") ;

	    malloctest(26) ;

	}
#endif /* CF_DEBUG */

	envtime = 0 ;
	rs = msg_init(&m,mbp->mfp,-1L,NULL,0) ;

	if (rs >= 0) {
	    MSG_ENV	*ep ;
	    time_t	tmptime ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("scanlong: 'init'ed a message object\n") ;
#endif

	    env_from = NULL ;
	    for (i = 0 ; msg_getenv(&m,i,&ep) >= 0 ; i += 1) {
	        if (ep == NULL) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("scanlong: got an envelope\n") ;
#endif

/* from */

	        if ((ep->from != NULL) && (env_from == NULL))
	            env_from = ep->from ;

/* from */

	        if (ep->date != NULL) {

#if	CF_DEBUG
	            if (pip->debuglevel >= 4) {
	                debugprintf("scanlong: non-NULL envelope date\n") ;
	                debugprintf("scanlong: >%s<\n",ep->date) ;
	            }
#endif

	            rs = dater_setstd(&d,ep->date,-1) ;

#if	CF_DEBUG
	            if (pip->debuglevel >= 4)
	                debugprintf("scanlong: dater_setstd() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {

	                dater_gettime(&d,&tmptime) ;

#if	CF_DEBUG
	                if (pip->debuglevel >= 4)
	                    debugprintf("scanlong: tmptime=>%s<\n",
	                        timestr_log(tmptime,timebuf)) ;
#endif

	                if (envtime != 0) {

	                    if (tmptime < envtime)
	                        envtime = tmptime ;

	                } else
	                    envtime = tmptime ;

	            }

	        } /* end if (had a date) */

	    } /* end for (looping through envelopes) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {

	        if (envtime != ((time_t) -1))
	            debugprintf("scanlong: envelope date %s\n",
	                timestr_scandate(envtime,timebuf)) ;

	        else
	            debugprintf("scanlong: no envelope date\n") ;

	    }
#endif /* CF_DEBUG */

/* get who it was from */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("scanlong: FROM\n") ;
#endif

	    rs = msg_headervalue(&m,"from",&sp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("scanlong: msg_headervalue() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        sp = env_from ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("scanlong: fixfrom() sp=>%s<\n",sp) ;
#endif

	string[0] = '\0' ;
	if (sp != NULL) {

	    rs1 = fixfrom(pip,sp,work,LINEBUFLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
#ifdef	COMMENT
		debugprintf("scanlong: fixfrom() rs=%d\n",rs1) ;
#endif
		debugprintf("scanlong: mkdislpayable() work=%t\n",
			work,strnlen(work,LINEBUFLEN)) ;
	}
#endif /* CF_DEBUG */

	    mkdisplayable(string,26,work,LINEBUFLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("scanlong: string=%s\n",string) ;
#endif

	}

	    string[26] = '\0' ;
	    slp += sprintf(slp,"%-26s",string) ;

/* get a subject */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("scanlong: SUBJECT\n") ;
#endif

	    rs = msg_headervalue(&m,"subject",&sp) ;

	    string[0] = '\0' ;
	    if (rs >= 0) {

#if	CF_FIXSUBJECT
	        mkdisplayable(string,30,sp,MIN(LINEBUFLEN,rs)) ;
#else
		strwcpy(string,sp,30) ;
#endif

	    }

	    string[30] = '\0' ;
	    slp += sprintf(slp," %-30s", string) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("scanlong: subject=>%s<\n",string) ;
#endif

/* date */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("scanlong: msg_headervalue() DATE\n") ;
#endif

	    rs = msg_headervalue(&m,"date",&sp) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("scanlong: msg_headervalue() rs=%d date=>%t<\n",
			rs,sp,MAX(0,rs)) ;
#endif

	    if (rs >= 0)
	        rs = dater_setmsg(&d,sp,rs) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("scanlong: dater_setmsg() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

		char	zname[100] ;


	        dater_gettime(&d,&msgtime) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

		int	zoff ;


		dater_getzonename(&d,zname,99) ;

		dater_getzoneoff(&d,&zoff) ;

	        debugprintf("scanlong: date=%s zone=%s off=%d\n",
			timestr_log(msgtime,timebuf),
			zname,zoff) ;

	}
#endif /* CF_DEBUG */

	        (void) timestr_scandate(msgtime,string) ;

	    } else
	        (void) timestr_scandate(envtime,string) ;

	    slp += sprintf(slp," %-15s",string) ;


/* done extracting what we want */

	    msg_free(&m) ;

	} /* end if (opened the message object) */


#ifdef	COMMENT

/* start with the "from" header */

	string[0] = '\0' ;
	fetchfield(mi,"FROM:",string,LINEBUFLEN) ;

	if ((*string == '\0') || (strncmp(string,blanks,3) == 0)) {

	    fetchfrom(pip,mi,string,LINEBUFLEN) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("scanlong: envelope FROM=\"%s\"\n",string) ;
#endif

	} /* end if (getting envelope FROM header) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("scanlong: of=>%s<\n",string) ;
#endif

#if	CF_FIXFROM
	fixfrom(pip,string,work,LINEBUFLEN) ;
#else
	strcpy(work,string) ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf("scanlong: returned from 'fixfrom'\n") ;

	    debugprintf("scanlong: fixfrom result of=>%s< mf=>%s<\n",
	        string,work) ;
	}
#endif /* CF_DEBUG */

	work[26] = '\0' ;
	slp += sprintf(slp,"%-26s",work) ;

/* continue with the rest of it */

	fetchfield(mi,"SUBJECT:",string,30) ;

	string[30] = '\0' ;
	slp += sprintf(slp," %-30s", string) ;

/* do some minor date processing */

	fetchfield(mi,"DATE:",string,LINEBUFLEN) ;

	if ((string[0] != '\0') &&
	    (dater_setmsg(&d,string,-1) >= 0)) {

	    dater_gettime(&d,&msgtime) ;

	    (void) timestr_scandate(msgtime,work) ;

	} else
	    (void) timestr_scandate(envtime,work) ;

	slp += sprintf(slp," %-15s",work) ;

#endif /* COMMENT */


/* handle the number of lines in this message */

	if (messlen[mi] <= 0) {

	    slp += sprintf(slp,"   ?") ;

	} else {

	    *slp++ = ' ' ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("lognscanner: before digit3\n") ;
#endif

	    digit3(slp,messlen[mi]) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("scanlong: after digit3\n") ;
#endif

	    slp += 3 ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("scanlong: scanline\n%s\n",scanline) ;
#endif

	mvwprintw(dsp->w_header,wline,0,
	    "%s",scanline) ;

	wclrtoeol(dsp->w_header) ;

	dater_finish(&d) ;

	return OK ;
}
/* end subroutine (scanlong) */


/* short version of scan line */
static int scanshort(pip,dsp,mbp,mn,wline)
struct proginfo	*pip ;
DS		*dsp ;
struct mailbox	*mbp ;
int		mn ;
int		wline ;
{
	int	mi ;

	char	string[LINEBUFLEN + 1] ;	/* getfield returns values here	*/


	mi = messord[mn] ;
	if (messdel[mi] == 1) 
		wprintw(dsp->w_header,"* ") ;

	else 
		wprintw(dsp->w_header,"  ") ;

	string[0] = '\0' ;
	fetchfield(mi,"FROM:",string,20) ;

	if ((*string == '\0') || (strncmp(string,blanks,3) == 0))
	    fetchfrom(pip,mi,string,20) ;

	compressname(string,10) ;

	wprintw(dsp->w_header," %10.10s", string) ;

	fetchfield(mi,"SUBJECT:",string,20) ;

	wmove(dsp->w_header,wline, 15) ;

	wprintw(dsp->w_header,"  %20.20s", string) ;

	fetchfield(mi,"DATE:",string,18) ;

	if (strncmp(string,blanks,3) == 0)
	    fetchfield(mi,"Date:",string,10) ;

	compressdate(string,6) ;

	wmove(dsp->w_header,wline, 37) ;

	wprintw(dsp->w_header,"  %6.6s", string) ;

	wmove(dsp->w_header,wline, 45) ;

	wprintw(dsp->w_header," %4ld", MIN(messlen[mi],9999)) ;

	wclrtoeol(dsp->w_header) ;

	return OK ;
}
/* end subroutine (scanshort) */


/* compress name into last name only & pad to right len */
static int compressname(str,len)
char	str[] ;
int	len ;
{
	int	k ;

	char	*name, *sp, tempstr[LINEBUFLEN + 1], *tname ;


/* eliminate initials */

	sp = strrchr(str,'.') ;

	if (sp == NULL)
	    name = str ;

	else 
	    name = sp + 1 ;

/* eliminate first name(s) */

	strcpy(tempstr,name) ;

	tname = tempstr ;
	sp = strtok(tempstr," ") ;

	while (sp != NULL) {

	    tname = sp ;
	    sp = strtok(0," ") ;

	} /* end while */

/* pad */

	strcpy(str,tname) ;

	for (k = strlen(str) ; k < len ; k += 1)
	    str[k] = ' ' ;

	str[len] = '\0' ;
	return len ;
}
/* end subroutine (compressname) */


/* compress date into only mm/dd if is in that form & pad to right len */
static int compressdate(str,len)
char	str[] ;
int	len ;
{
	int	k ;

	char	*sp ;


	k=0 ;
	for (sp = str ; *sp != '\0' ; sp += 1) {

	    if (*sp == '/') {

	        if (k == 1)
	            break ;

	        else 
	            k += 1 ;

	    }

	} /* end for */

	for (k = sp - str ; k < len ; k += 1)
	    str[k] = ' ' ;

	str[len] = '\0' ;
	return len ;
}
/* end subroutine (compressdate) */


/* print a line identifying the fields in the scanline */
static int scantitle(pip,dsp)
struct proginfo	*pip ;
DS		*dsp ;
{

	int	pv ;


	pv = profile("long_scan") ;

	if ((pv < 0) || (pv > 0)) {

	    char	buf[40] ;


	    if (dsp->tzname[0] != '\0')
	        sprintf(buf,"DATE (%s)", dsp->tzname) ;

	    else
	        strcpy(buf,"DATE") ;

	    wprintw(dsp->w_root,"%-30s%-31s%-14s%-5s\n",
	        "   FROM","SUBJECT",buf,"LINES") ;

	} else 
	    wprintw(dsp->w_root,"%-18s%-21s%-7s%-5s\n",
	        "   FROM","SUBJECT","DATE","LINES") ;

	return 0 ;
}
/* end subroutine (scantitle) */



