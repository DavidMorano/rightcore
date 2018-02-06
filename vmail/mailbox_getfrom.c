/* mailbox_getfrom */

/* get a FROM address from a mail message */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets a FROM address from a mail message.

	Synopsis:

	int mailbox_getfrom(MAILBOX *mbp,char *rbuf,int rlen,cchar *fn,int mi)

	Arguments:

	mbp		pointer to MAILBOX object
	rbuf		buffer to hold result
	rlen		length of supplied result buffer
	fn		mailbox file-name
	mi		message-id (ID of message within mail-box)

	Returns:

	>=0		OK
	<0		some error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<mailbox.h>
#include	<mailmsg.h>
#include	<mailmsghdrs.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1w(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	pathadd(char *,cchar *,int) ;
extern int	sfsubstance(const char *,int,const char **) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	mkbestfrom(char *,int,const char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	mailmsg_loadfile(MAILMSG *,bfile *) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	mailmsg_fromer(MAILMSG *,char *,int) ;
static int	isNoMsg(int) ;


/* local variables */

static const int	rsnomsg[] = {
	SR_NOMSG,
	SR_NOENT,
	0
} ;


/* exported subroutines */


int mailbox_getfrom(MAILBOX *mbp,char *rbuf,int rlen,cchar *fn,int mi)
{
	MAILBOX_MSGINFO	msginfo ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("mailbox_getfrom: ent mi=%d\n",mi) ;
#endif

	if (mbp == NULL) return SR_FAULT ;
	if (fn == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (fn[0] == '\0') return SR_INVALID ;

	if (mi < 0) {
	    MAILBOX_INFO	mbinfo ;
	    rs = mailbox_info(mbp,&mbinfo) ;
	    if (mbinfo.nmsgs > 0) mi = (mbinfo.nmsgs - 1) ;
	} /* end if (default) */

	if ((rs >= 0) && (mi >= 0)) {
	    if ((rs = mailbox_msginfo(mbp,mi,&msginfo)) >= 0) {
		bfile		mf ;
	        if ((rs = bopen(&mf,fn,"r",0666)) >= 0) {
		    offset_t	moff = msginfo.moff ;
	            if ((rs = bseek(&mf,moff,SEEK_SET)) >= 0) {
			MAILMSG		m ;
	                if ((rs = mailmsg_start(&m)) >= 0) {
			    if ((rs = mailmsg_loadfile(&m,&mf)) >= 0) {
				rs = mailmsg_fromer(&m,rbuf,rlen) ;
			        len = rs ;
			    }
	                    rs1 = mailmsg_finish(&m) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (mailmsg) */
	            } /* end if (seek) */
	            rs1 = bclose(&mf) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (mail-file) */
	    } /* end if (msg-info) */
	} /* end if (positive MI) */

#if	CF_DEBUGS
	debugprintf("mailbox_getfrom: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mailbox_getfrom) */


/* local subroutines */


static int mailmsg_fromer(MAILMSG *mmp,char *rbuf,int rlen)
{
	int		rs ;
	int		vl = 0 ;
	int		len = 0 ;
	cchar		*vp ;
	cchar		*hn = HN_FROM ;
	if ((rs = mailmsg_hdrival(mmp,hn,0,&vp)) > 0) {
	    vl = rs ;
	} else if ((rs == 0) || isNoMsg(rs)) {
	    hn = HN_RETURNPATH ;
	    if ((rs = mailmsg_hdrival(mmp,hn,0,&vp)) > 0) {
	        vl = rs ;
	    } else if ((rs == 0) || isNoMsg(rs)) {
	        hn = HN_REPLYTO ;
	        if ((rs = mailmsg_hdrival(mmp,hn,0,&vp)) > 0) {
	            vl = rs ;
	        } else if ((rs == 0) || isNoMsg(rs)) {
	            hn = HN_SENDER ;
	            if ((rs = mailmsg_hdrival(mmp,hn,0,&vp)) > 0) {
	                vl = rs ;
	            } else if ((rs == 0) || isNoMsg(rs)) {
	                rs = mailmsg_envaddress(mmp,0,&vp) ;
	                vl = rs ;
		    }
		}
	    }
	}
	if ((rs >= 0) && (vl > 0)) {
	    rs = mkbestfrom(rbuf,rlen,vp,vl) ;
	    len = rs ;
	}
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mailmsg_fromer) */


static int isNoMsg(int rs)
{
	return isOneOf(rsnomsg,rs) ;
}
/* end subroutine (isNoMsg) */


