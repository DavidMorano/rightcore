/* cmd_save */

/* save a bulletin board article */


#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_UGETPW	1		/* use 'ugetpw(3uc)' */


/* revision history:

	= 1994-02-01, David A­D­ Morano
        I wrote this from scratch when I took over the code. The previous code
        was a mess (still is in many places!).

	= 1998-11-22, David A­D­ Morano
	Some clean-up.

*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Save a bulletin board article to either a mailbox or to standard
	output depending on the mode we are called with.

	mode=SMODE_MAILBOX	save article to mailbox
	mode=SMODE_OUT		write article to standard output


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bfile.h>
#include	<char.h>
#include	<dater.h>
#include	<getax.h>
#include	<localmisc.h>

#include	"artlist.h"
#include	"headerkeys.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	MAILFOLDER
#define	MAILFOLDER	"mail"
#endif

#ifndef	MAILNEW
#define	MAILNEW		"new"
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkmailname(char *,int,const char *,int) ;

extern int	bbcpy(char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;


/* external data */

extern struct proginfo	g ;


/* forward references */

static int	envelope() ;


/* local variables */


/* exported subroutines */


int cmd_save(pip,ap,ngdir,afname,mode,mailbox)
struct proginfo	*pip ;
ARTLIST_ENT	*ap ;
const char	ngdir[] ;
const char	afname[] ;
const int	mode ;
const char	mailbox[] ;
{
	struct ustat	stat_a ;
	struct ustat	sb ;
	struct tm	tms, *timep ;
	bfile		afile, *afp = &afile ;
	bfile		mfile, *mfp = &mfile ;
	offset_t	offset ;
	time_t		t ;

	long	flen ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	line ;
	int	clen, len ;
	int	cl ;
	int	f_bol, f_eol, f_clen = FALSE ;
	int	f_from = FALSE ;
	int	f_newsgroups = FALSE ;
	int	f_date = FALSE ;
	int	f_articleid = FALSE ;
	int	f_nofile = FALSE ;
	int	f_envesc = FALSE ;

	const char	*cp ;

	char	ngname[MAXPATHLEN + 1] ;
	char	maildname[MAXPATHLEN + 1] ;
	char	mfname[MAXPATHLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;
	char	hv_date[LINEBUFLEN + 1] ;
	char	env_from[LINEBUFLEN + 1] ;
	char	env_date[LINEBUFLEN + 1] ;
	char	from_username[USERNAMELEN + 1] ;
	char	from_realname[REALNAMELEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("save: mode=%d\n",mode) ;
#endif

	if (ngdir == NULL) return SR_FAULT ;
	if (afname == NULL) return SR_FAULT ;

	if (ngdir[0] == '\0') return SR_INVALID ;
	if (afname[0] == '\0') return SR_INVALID ;

	mfname[0] = '\0' ;
	if (mode == SMODE_MAILBOX) {

	    if (mailbox == NULL)
	        mailbox = "new" ;

	    mkpath2(maildname, pip->homedname,MAILFOLDER) ;

	    mkpath2(mfname,maildname,mailbox) ;

	} /* end if (save to mailbox) */

/* get the current time */

	pip->now.time = time(NULL) ;

/* get status of current file */

	flen = -1 ;
	rs1 = bopen(afp,afname,"r",0666) ;
	if (rs1 < 0)
	    goto ret0 ;

	rs1 = bcontrol(afp,BC_STAT,&stat_a) ;
	if (rs1 < 0)
	    goto ret1 ;

	flen = stat_a.st_size ;

/* OK, we start */

	if (mode == SMODE_MAILBOX) {

/* if the directory for mailboxes does not exist, create it */

	    f_nofile = FALSE ;
	    if ((rs = u_stat(maildname,&sb)) < 0) {

/* check if the directory is there or not */

	        if (rs == SR_NOENT) {

	            if (u_mkdir(maildname,0775) < 0) {

	                f_nofile = TRUE ;
	                bprintf(g.efp,
	                    "%s: couldn't create mail directory (%d)\n",
	                    g.progname,rs) ;

	            }

	        } else {

/* can't access it */

	            f_nofile = TRUE ;
	            bprintf(g.efp,
	                "%s: can't access mail directory,",
	                g.progname) ;

	            bprintf(g.efp," please check permissions\n") ;

	        }

	    } /* end if (creating missing user mail directory) */

	    if (! f_nofile) {

/* open mailbox and append new message */

	        rs = bopen(mfp,mfname,"wca",0664) ;
		if (rs < 0) mfp = NULL ;

	    } else
	        rs = BAD ;

	} else {

	    mfp = g.ofp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("save: ofp=%08X\n",mfp) ;
#endif

	} /* end if (mailbox or regular mode) */

	if (rs < 0)
	    goto ret1 ;

/* get information about this article */

	from_username[0] = '\0' ;
	from_realname[0] = '\0' ;
	{
	    struct passwd	pw ;
	    const uid_t		uid = stat_a.st_uid ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    cchar		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs = GETPW_UID(&pw,pwbuf,pwlen,uid)) >= 0) {
	    	    strwcpy(from_username,pw.pw_name,USERNAMELEN) ;
	    	    mkmailname(from_realname,REALNAMELEN,pw.pw_gecos,-1) ;
		} else {
	    	    snsd(from_username,USERNAMELEN,"G",uid) ;
	    	    strwcpy(from_realname,from_username,REALNAMELEN) ;
		} /* end if */
		uc_free(pwbuf) ;
	    } /* end if (memory-allocation) */
	}

	offset = 0 ;

/* process the header lines */

	f_bol = TRUE ;
	line = 0 ;
	while ((rs = breadline(afp,linebuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    f_eol = (linebuf[len - 1] == '\n') ? TRUE : FALSE ;
	    linebuf[len] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("save: LINE> %s",linebuf) ;
#endif

	    offset += len ;
	    if (f_bol) {

	        if (linebuf[0] == '\n')
	            break ;

/* check for the envelope header line */

	        if ((line == 0) && pip->f.addenv) {
	            const char	*envfromstr ;

	            t = pip->now.time ;
	            if (pip->f.envdate) {

	                switch (pip->whichenvdate) {

	                case SORTMODE_MTIME:
	                    t = stat_a.st_mtime ;
	                    break ;

	                case SORTMODE_ATIME:
	                    if ((t = ap->atime) == 0)
	                        t = ap->mtime ;

	                    break ;

	                case SORTMODE_PTIME:
	                    if ((t = ap->ptime) == 0)
	                        if ((t = ap->atime) == 0)
	                            t = ap->mtime ;

	                    break ;

	                case SORTMODE_CTIME:
	                    if ((t = ap->ctime) == 0)
	                        if ((t = ap->ptime) == 0)
	                            if ((t = ap->atime) == 0)
	                                t = ap->mtime ;

	                    break ;

	                case SORTMODE_NOW:
	                default:
	                    t = pip->now.time ;
	                    break ;

	                case SORTMODE_SUPPLIED:
	                    dater_gettime(&pip->envdate,&t) ;

	                    break ;

	                } /* end switch */

	            } /* end if (whichenvdate) */

	            if (! pip->f.envfrom) {

	                envfromstr = env_from ;
	                sncpy3(env_from,LINEBUFLEN,
	                    g.mailhost,"!",from_username) ;

	            } else
	                envfromstr = pip->envfrom ;

	            bprintf(mfp,
	                "From %s %s\n",
	                envfromstr,
	                timestr_edate(t,env_date)) ;

	            f_envesc = TRUE ;

	        } /* end if (adding an envelope) */

/* write an envelope if it does not already have one */

	        if ((line == 0) && (! f_envesc) &&
	            (! matthingenv(linebuf,-1))) {

#if	CF_DEBUG
	            if (g.debuglevel > 2)
	                debugprintf("save: writing an envelope header\n") ;
#endif

	            bprintf(mfp,
	                "From %s!%s %s\n",
	                g.mailhost,from_username,
	                timestr_edate(stat_a.st_mtime,env_date)) ;

	            f_envesc = FALSE ;

	        } /* end if (writing an envelope header) */

/* possibly escape all other envelopes (since we added one) */

	        if (f_envesc && matthingenv(linebuf,-1) &&
	            (linebuf[0] != '>')) {

	            bputc(mfp,'>') ;

	        } /* end if (escaping extra envelopes) */

/* all other headers */

	        if (strncasecmp(linebuf,"content-length",14) == 0) {

	            f_clen = TRUE ;

	        } else if ((strncasecmp(linebuf,HK_NEWSGROUPS,10)) == 0) {

	            f_newsgroups = TRUE ;

	        } else if ((strncasecmp(linebuf,HK_ARTICLEID,10)) == 0) {

	            f_articleid = TRUE ;

	        } else if ((strncasecmp(linebuf,HK_DATE,4)) == 0) {

	            f_date = TRUE ;

	        } else if ((strncasecmp(linebuf,HK_BOARD, 5)) == 0) {

	            cp = linebuf + 5 ;
	            while (CHAR_ISWHITE(*cp))
	                cp += 1 ;

	            if (*cp == ':') {

	                cp += 1 ;
	                while (CHAR_ISWHITE(*cp))
	                    cp += 1 ;

	                bprintf(mfp,"%s: %s",HK_NEWSGROUPS,cp) ;

	            } /* end if */

	            len = 0 ;
	            f_newsgroups = TRUE ;

	        } else if (strncasecmp(linebuf,"from",4) == 0) {

	            cp = linebuf + 4 ;
	            while (CHAR_ISWHITE(*cp))
	                cp += 1 ;

	            if (*cp == ':') cp += 1 ;

	            while (CHAR_ISWHITE(*cp))
	                cp += 1 ;

	            if (*cp == '\n')
	                len = 0 ;

	                else
	                f_from = TRUE ;

	        } else if ((strncasecmp(linebuf,HK_TITLE,5)) == 0) {

	            cp = linebuf + 5 ;
	            while (CHAR_ISWHITE(*cp))
	                cp += 1 ;

	            if (*cp == ':')
	                cp += 1 ;

	            while (CHAR_ISWHITE(*cp))
	                cp += 1 ;

	            bprintf(mfp,"%s:    %s\n",HK_SUBJECT,cp) ;

	            len = 0 ;

	        } /* end if */

	    } /* end if (BOL processing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	        linebuf[len] = '\0' ;
	        debugprintf("save: %d W> %s",len,linebuf) ;
	    }
#endif

	    rs = bwrite(mfp,linebuf,len) ;
	    if (rs < 0)
		break ;

	    if (f_eol)
	        line += 1 ;

	    f_bol = f_eol ;

	} /* end while (header lines) */

	if (rs < 0)
	    goto ret2 ;

/* end of reading the original header lines for this article */


/* add header lines that may not have been there! */

/* article ID length */

	if (! f_articleid)
	    bprintf(mfp,"%s: %s\n",HK_ARTICLEID,strbasename(afname)) ;

/* content length */

	if (flen >= 0) {

	    clen = flen - offset ;
	    if (! f_clen)
	        bprintf(mfp,"%s:  %d\n",HK_CLEN,clen) ;

	}

/* "from" person */

	if (! f_from) {

	    if (from_realname != NULL) {

	        if (from_username != NULL) {
	            bprintf(mfp, "%s:       %s <%s!%s>\n",
	                HK_FROM,from_realname,
	                g.mailhost,from_username) ;

	        } else
	            bprintf(mfp, "%s:       %s <%s!pcs>\n",
	                HK_FROM,from_realname,
	                g.mailhost) ;

	    } else {

	        bprintf(mfp, "%s:       %s\n",
	            HK_FROM,from_username) ;

	    }

	} /* end if */

/* which newsgroup */

	if (! f_newsgroups) {

	    bbcpy(ngname,ngdir) ;

	    bprintf(mfp,"%s: %s\n",HK_NEWSGROUPS,ngname) ;

	}

/* date of posting */

	if ((! f_date) &&
	    (bcontrol(afp,BC_STAT,&sb) >= 0)) {

	    timestr_hdate(sb.st_mtime,hv_date) ;

	    bprintf(mfp,"%s:       %s\n",HK_DATE,hv_date) ;

	} /* end if (posting date) */

/* to whom! (not needed) */

#ifdef	COMMENT
	bprintf(mfp,"%s:       %s\n",HK_TO,pip->username) ;
#endif

/* write out the end-of-header marker (a blank line) */

	bprintf(mfp,"\n") ;

/* copy the bulletin over to the mailbox */

	rs = bcopyblock(afp,mfp,clen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    if (rs < 0)
	        debugprintf("cmd_save: bad file copy rs=%d\n", rs) ;
#endif

/* close up stuff */
bad:
ret2:
	if ((mode == SMODE_MAILBOX) && (mfp != NULL))
	    bclose(mfp) ;

ret1:
	bclose(afp) ;

ret0:
	return rs ;
}
/* end subroutine (cmd_save) */


/* local subroutines */


/* determine if the line is an envelope header line or not */
static int envelope(linebuf,env_from,env_date)
const char	linebuf[] ;
char		env_from[] ;
char		env_date[] ;
{
	int		f_colon ;
	const char	*cp, *cp2 ;

#if	CF_DEBUG
	if (g.debuglevel > 2)
	    debugprintf("envelope: ent\n") ;
#endif

	if (strncmp(linebuf,"From",4) != 0)
	    return FALSE ;

#if	CF_DEBUG
	if (g.debuglevel > 2)
	    debugprintf("envelope: 1\n") ;
#endif

	cp = linebuf + 4 ;
	if (! CHAR_ISWHITE(*cp))
	    return FALSE ;

	cp += 1 ;
	while (CHAR_ISWHITE(*cp))
	    cp += 1 ;

	if (*cp == ':')
	    return FALSE ;

#if	CF_DEBUG
	if (g.debuglevel > 2)
	    debugprintf("envelope: 2\n") ;
#endif

/* we should be at the start of the envelope FROM field */

	cp2 = cp ;
	while (*cp && (! CHAR_ISWHITE(*cp)))
	    cp += 1 ;

	if (env_from != NULL)
	    strwcpy(env_from,cp2,cp - cp2) ;

	while (CHAR_ISWHITE(*cp))
	    cp += 1 ;

/* we should be at the start of the envelope DATE field */

	cp2 = cp ;
	f_colon = FALSE ;
	while (*cp) {

	    if (*cp == ':')
	        f_colon = TRUE ;

	    cp += 1 ;
	}

	if (! f_colon)
	    return FALSE ;

#if	CF_DEBUG
	if (g.debuglevel > 2)
	    debugprintf("envelope: 3\n") ;
#endif

	if (env_date != NULL)
	    strwcpy(env_date,cp2,cp - cp2) ;

	return TRUE ;
}
/* end subroutine (envelope) */



