/* emit_header */

/* print out a header value from the article */


#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_UGETPW	1		/* use 'ugetpw(3uc)' */


/* revision history:

	= 1994-11-01, David A­D­ Morano
	1) added a mode to intercept for mailbox use

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	IMPORTANT: This subroutine is one of the "EMIT" subroutines used for 
	"emitting" articles in different ways.

        This subroutine is used to print out the "subject" header (if present)
        of a article.

	Synopsis:

	int emit_header(pip,dsp,ai,ap,ngdir,af)
	struct proginfo	*pip ;
	MKDIRLIST_ENT	*dsp ;
	int		ai ;
	ARTLIST_ENT	*ap ;
	char		ngdir[] ;
	char		af[] ;

	Arguments:

	pip		program information pointer
	dsp		user structure pointer
	ai		article index within newsgroup
	ap		article ARTLIST_ENT pointer
	ngdir		directory (relative to spool directory) of article
	af		article base file name

	Returns:

	-		??


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bfile.h>
#include	<dater.h>
#include	<mailmsghdrs.h>
#include	<getax.h>
#include	<localmisc.h>

#include	"headerkeys.h"
#include	"config.h"
#include	"defs.h"
#include	"mkdirlist.h"
#include	"artlist.h"


/* local defines */

#define	HEADBUFLEN	2048

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#define	MAXFIELDLEN	77

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */


/* external subroutines */

extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkmailname(char *,int,const char *,int) ;
extern int	sfcenter(const char *,int,const char *,const char **) ;
extern int	hdrextid(char *,int,const char *,int) ;

extern int	bbcpy(char *,const char *) ;
extern int	mm_getfield() ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* exported subroutines */


int emit_header(pip,dsp,ai,ap,ngdir,af)
struct proginfo	*pip ;
MKDIRLIST_ENT	*dsp ;
int		ai ;
ARTLIST_ENT	*ap ;
const char	ngdir[] ;
const char	af[] ;
{
	struct ustat	mmsb ;
	struct passwd	ps, *pp ;
	bfile		afile, *afp = &afile ;
	const int	hlen = HEADBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i, j ;
	int		sl, len ;
	int		cl ;
	int		f_messageid ;
	const char	*nd = pip->newsdname ;
	const char	*idp ;
	const char	*nbp ;
	const char	*hk ;
	const char	*cp ;
	char		hbuf[HEADBUFLEN + 1] ;
	char		fname[MAXPATHLEN + 1] ;
	char		buf[BUFLEN + 1] ;
	char		buf1[BUFLEN + 1] ;
	char		buf2[BUFLEN + 1] ;

	if ((ngdir == NULL) || (*ngdir == '\0'))
	    return EMIT_OK ;

	if (af == NULL)
	    return EMIT_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("emit_header: entered\n") ;
#endif

/* create the full path to this article file */

	mkpath3(fname,nd,ngdir,af) ;

/* this is a hack to only print out the newsgroup name when we have articles */

	if (pip->f.header) {
	    bbcpy(hbuf,ngdir) ;
	    pip->f.header = FALSE ;
	    bprintf(pip->ofp,"newsgroup> %s\n",hbuf) ;
	}

/* open the article file and get its status */

	if ((rs = bopen(afp,fname, "r",0666)) < 0)
	    goto done1 ;

	if ((rs = bcontrol(afp,BC_STAT,&mmsb)) < 0)
	    goto done2 ;

/* do whatever! */

	switch (pip->header) {

	case HI_ARTICLEID:
	    bprintf(pip->ofp,"  %s\n",af) ;
	    break ;

	case HI_SUBJECT:
#ifdef	COMMENT
	    sl = mm_getfield(afp,0L,mmsb.st_size,HK_SUBJECT, hbuf,hlen) ;
	    if (sl <= 0)
	        sl = mm_getfield(afp,0L,mmsb.st_size,HK_TITLE,hbuf,hlen) ;
	    if (sl > 0) {
	        if (sl > MAXFIELDLEN) sl = MAXFIELDLEN ;
	        bprintf(pip->ofp,"  %t\n",hbuf,sl) ;
	    } else {
		hk = HK_MESSAGEID ;
	        if ((sl = mm_getfield(afp,0L,mmsb.st_size,hk,hbuf,hlen)) > 0) {
	            if (sl > 37) sl = 37 ;
	            bprintf(pip->ofp,
	                "  ** no subject on message ID \"%s\"\n",hbuf) ;
	        } else {
		    hk = HK_ARTICLEID ;
	            sl = mm_getfield(afp,0L,mmsb.st_size,hk,hbuf,hlen) ;
	            if (sl > 37) sl = 37 ;
	            bprintf(pip->ofp,
	                "  ** no subject on article ID \"%s\"\n",
	                (sl > 0) ? hbuf : af) ;
	        }
	    }
#else /* COMMENT */
		if (ap->subject != NULL) {
		    sl = strnlen(ap->subject,MAXFIELDLEN) ;
	            bprintf(pip->ofp,"  %t\n",ap->subject,sl) ;
		}
#endif /* COMMENT */
	    break ;

/* try to read out a "from" header from the posted article */
	case HI_FROM:
	    sl = mm_getfield(afp,0L,mmsb.st_size,HK_FROM,hbuf,hlen) ;
	    if (sl > 0) {
	        if (sl > MAXFIELDLEN) sl = MAXFIELDLEN ;
	        bprintf(pip->ofp,"  %t\n",hbuf,sl) ;
	    } else {
		const int	pwlen = getbufsize(getbufsize_pw) ;
		struct passwd	pw ;
		uid_t		uid = mmsb.st_uid ;
		char		*pwbuf ;
		if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
		    if ((rs1 = GETPW_UID(&pw,pwbuf,pwlen,uid)) >= 0) {
		        nbp = buf1 ;
		        rs1 = mkmailname(buf1,BUFLEN,pw.pw_gecos,-1) ;
		        if (rs1 < 0)
			    nbp = pw.pw_name ;
	            } else {
	                nbp = "*unknown*" ;
		    }
		    uc_free(pwbuf) ;
		} /* end if (memory-allocation) */
	        sl = mm_getfield(afp,0L,mmsb.st_size,HK_MESSAGEID,hbuf,hlen) ;
	        f_messageid = FALSE ;
	        if (sl > 0) {
	            if (sl > 57) {
	                sl = 57 ;
	                hbuf[sl] = '\0' ;
	                f_messageid = TRUE ;
	            }
	        } else {
	            sl = mm_getfield(afp,0L,mmsb.st_size,HK_ARTICLEID,
	                hbuf,hlen) ;
	            if (sl > 0) {
	                if (sl > 57) sl = 57 ;
	                hbuf[sl] = '\0' ;
	            }
	        } /* end if */
	        if (f_messageid) {
	            idp = hbuf ;
	        } else {
	            idp = (sl > 0) ? hbuf : af ;
		}
	        bprintf(pip->ofp,
	            "  ** posting user on %s ID \"%s\" was '%s'\n",
	            (f_messageid) ? "message" : "article",
	            idp,
	            nbp) ;
	    } /* end if (no "from" header) */
	    break ;

/* try to get the date out of the article */
	case HI_DATE:
	    {
	        time_t	t ;
	        char	timebuf[TIMEBUFLEN + 1] ;
	        sl = mm_getfield(afp,0L,mmsb.st_size,HK_DATE,hbuf,hlen) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	            debugprintf("emit_header: hdate=>%t<\n",
	                hbuf,sl) ;
#endif
	        if (sl > 0) {
	            rs = dater_setmsg(&pip->tmpdate,hbuf,sl) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("emit_header: dater_setmsg() rs=%d\n", rs) ;
#endif
	            if (rs >= 0) {
	                dater_gettime(&pip->tmpdate,&t) ;
	                bprintf(pip->ofp,"  %s\n",
	                    timestr_edate(t,timebuf)) ;
	            } else {
	                bprintf(pip->ofp, "  %s (arrival)\n",
	                    timestr_edate(mmsb.st_mtime,timebuf)) ;
		    }
	        } else {
	            bprintf(pip->ofp, "  %s (arrival)\n",
	                timestr_edate(mmsb.st_mtime,timebuf)) ;
	        }
	    } /* end block */
	    break ;

/* message-ID */
	case HI_MSGID:
	    sl = mm_getfield(afp,0L,mmsb.st_size,HK_MESSAGEID,
	        hbuf,hlen) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("emit_header: hl=%d hb=%s\n",sl,hbuf) ;
#endif
	    if (sl > 0) {
		const int	ilen = MAXNAMELEN ;
		char		ibuf[MAXNAMELEN+1] ;
		cp = ibuf ;
		if ((rs = hdrextid(ibuf,ilen,hbuf,sl)) == 0) {
		    const char	*tp ;
		    const char	*sp = hbuf ;
		    int		sl = hlen ;
		    if ((tp = strnchr(sp,sl,',')) != NULL) sl = (tp-hbuf) ;
		    if ((cl = sfcenter(sp,sl,"<>",&cp)) <= 0) {
			cp = sp ;
			cl = sl ;
		    }
		}
#ifdef	COMMENT
	        if ((cp = strchr(hbuf,'<')) != NULL) {
	            char	*cp2 ;
	            cp += 1 ;
	            if ((cp2 = strchr(cp,'>')) != NULL)
	                *cp2 = '\0' ;
	        } else
	            cp = hbuf ;
	        cp[MAXFIELDLEN] = '\0' ;
#endif /* COMMENT */
	        if (cl > MAXFIELDLEN) cl = MAXFIELDLEN ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
		debugprintf("emit_header: mid=%t\n",cp,cl) ;
#endif
	        bprintf(pip->ofp,"  %t\n",cp,cl) ;
	    } else {
	        bprintf(pip->ofp,
	            "  ** article ID - %s\n",af) ;
	    }
	    break ;

	} /* end switch */

/* we are done, let's get out of here */
done2:
	bclose(afp) ;

done1:
	return (rs >= 0) ? EMIT_DONE : rs ;
}
/* end subroutine (emit_header) */


