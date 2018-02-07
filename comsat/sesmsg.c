/* sesmsg */

/* create and parse the internal messages */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2002-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the code to make and parse the internal messages
	that are used in this whole server facility.


*******************************************************************************/


#define	SESMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"sesmsg.h"


/* local defines */

#define	NDF		"sesmsg.deb"


/* external subroutines */


/* local structures */


/* forward references */

static int sesmsg_mbuf(SESMSG_MBUF *,int,int,char *,int) ;


/* local variables */


/* exported subroutines */


int sesmsg_exit(SESMSG_EXIT *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("sesmsg_exit: ent mlen=%d\n",mlen) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"sesmsg_exit: ent mlen=%d\n",mlen) ;
#endif
	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;
	    if (f) { /* read */
	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;
	        serialbuf_ruint(&mb,&sp->tag) ;
	        serialbuf_rstrw(&mb,sp->reason,SESMSG_REASONLEN) ;
	    } else { /* write */
	        sp->msgtype = sesmsgtype_exit ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;
	        serialbuf_wuint(&mb,sp->tag) ;
	        serialbuf_wstrw(&mb,sp->reason,SESMSG_REASONLEN) ;
	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }
	    } /* end if */
	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
	    debugprintf("sesmsg_exit: serialbuf_finish() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	    nprintf(NDF,"sesmsg_exit: serialbuf_finish() rs=%d\n",rs) ;
#endif
	} /* end if (serialbuf) */
#if	CF_DEBUGS
	debugprintf("sesmsg_exit: ret rs=%d\n",rs) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"sesmsg_exit: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sesmsg_exit) */


int sesmsg_noop(SESMSG_NOOP *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;
	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;
	    if (f) { /* read */
	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;
	        serialbuf_ruint(&mb,&sp->tag) ;
	    } else { /* write */
	        sp->msgtype = sesmsgtype_noop ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;
	        serialbuf_wuint(&mb,sp->tag) ;
	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }
	    } /* end if */
	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */
	return rs ;
}
/* end subroutine (sesmsg_noop) */


int sesmsg_gen(SESMSG_GEN *sp,int f,char *mbuf,int mlen)
{
	SESMSG_MBUF	*mp = (SESMSG_MBUF *) sp ;
	const int	mt = sesmsgtype_gen ;
	return sesmsg_mbuf(mp,mt,f,mbuf,mlen) ;
}
/* end subroutine (sesmsg_gen) */


int sesmsg_biff(SESMSG_BIFF *sp,int f,char *mbuf,int mlen)
{
	SESMSG_MBUF	*mp = (SESMSG_MBUF *) sp ;
	const int	mt = sesmsgtype_biff ;
	return sesmsg_mbuf(mp,mt,f,mbuf,mlen) ;
}
/* end subroutine (sesmsg_biff) */


int sesmsg_echo(SESMSG_ECHO *sp,int f,char *mbuf,int mlen)
{
	SESMSG_MBUF	*mp = (SESMSG_MBUF *) sp ;
	const int	mt = sesmsgtype_echo ;
	return sesmsg_mbuf(mp,mt,f,mbuf,mlen) ;
}
/* end subroutine (sesmsg_echo) */


int sesmsg_response(SESMSG_RESPONSE *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;
	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;
	    if (f) { /* read */
	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;
	        serialbuf_ruint(&mb,&sp->tag) ;
	        serialbuf_ruint(&mb,&sp->pid) ;
	        serialbuf_ruchar(&mb,&sp->rc) ;
	    } else { /* write */
	        sp->msgtype = sesmsgtype_response ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;
	        serialbuf_wuint(&mb,sp->tag) ;
	        serialbuf_wuint(&mb,sp->pid) ;
	        serialbuf_wuchar(&mb,sp->rc) ;
	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }
	    } /* end if */
	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */
	return rs ;
}
/* end subroutine (sesmsg_response) */


int sesmsg_passfd(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_passfd	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_rstrw(&mb,sp->svc,SESMSG_SVCLEN) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_passfd ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wchar(&mb,sp->msgtype) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wstrw(&mb,sp->svc,SESMSG_SVCLEN) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_passfd) */


int sesmsg_getsysmisc(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_getsysmisc	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_getsysmisc ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_getsysmisc) */


int sesmsg_sysmisc(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_sysmisc	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_ruint(&mb,&sp->la_1min) ;

	        serialbuf_ruint(&mb,&sp->la_5min) ;

	        serialbuf_ruint(&mb,&sp->la_15min) ;

	        serialbuf_ruint(&mb,&sp->boottime) ;

	        serialbuf_ruint(&mb,&sp->nproc) ;

	        serialbuf_ruchar(&mb,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_sysmisc ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wuint(&mb,sp->la_1min) ;

	        serialbuf_wuint(&mb,sp->la_5min) ;

	        serialbuf_wuint(&mb,sp->la_15min) ;

	        serialbuf_wuint(&mb,sp->boottime) ;

	        serialbuf_wuint(&mb,sp->nproc) ;

	        serialbuf_wuchar(&mb,sp->rc) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_sysmisc) */


int sesmsg_getloadave(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_getloadave	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_getloadave ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_getloadve) */


int sesmsg_loadave(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_loadave	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_ruint(&mb,&sp->la_1min) ;

	        serialbuf_ruint(&mb,&sp->la_5min) ;

	        serialbuf_ruint(&mb,&sp->la_15min) ;

	        serialbuf_ruchar(&mb,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_loadave ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wuint(&mb,sp->la_1min) ;

	        serialbuf_wuint(&mb,sp->la_5min) ;

	        serialbuf_wuint(&mb,sp->la_15min) ;

	        serialbuf_wuchar(&mb,sp->rc) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_loadave) */


int sesmsg_reploadave(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_reploadave	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_ruint(&mb,&sp->utag) ;

	        serialbuf_ruint(&mb,&sp->duration) ;

	        serialbuf_ruint(&mb,&sp->interval) ;

	        serialbuf_rushort(&mb,&sp->addrfamily) ;

	        serialbuf_rushort(&mb,&sp->addrport) ;

	        serialbuf_ruint(&mb,&sp->addrhost[0]) ;

	        serialbuf_ruint(&mb,&sp->addrhost[1]) ;

	        serialbuf_ruint(&mb,&sp->addrhost[2]) ;

	        serialbuf_ruint(&mb,&sp->addrhost[3]) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_reploadave ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wuint(&mb,sp->utag) ;

	        serialbuf_wuint(&mb,sp->duration) ;

	        serialbuf_wuint(&mb,sp->interval) ;

	        serialbuf_wushort(&mb,sp->addrfamily) ;

	        serialbuf_wushort(&mb,sp->addrport) ;

	        serialbuf_wuint(&mb,sp->addrhost[0]) ;

	        serialbuf_wuint(&mb,sp->addrhost[1]) ;

	        serialbuf_wuint(&mb,sp->addrhost[2]) ;

	        serialbuf_wuint(&mb,sp->addrhost[4]) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_reploadave) */


int sesmsg_getlistener(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_getlistener	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_ruint(&mb,&sp->idx) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_getlistener ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wuint(&mb,sp->idx) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_getlistener) */


int sesmsg_listener(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_listener	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_ruint(&mb,&sp->idx) ;

	        serialbuf_ruint(&mb,&sp->pid) ;

	        serialbuf_ruchar(&mb,&sp->rc) ;

	        serialbuf_ruchar(&mb,&sp->ls) ;

	        serialbuf_rstrw(&mb,sp->name,SESMSG_LNAMELEN) ;

	        serialbuf_rstrw(&mb,sp->addr,SESMSG_LADDRLEN) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_listener ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wuint(&mb,sp->idx) ;

	        serialbuf_wuint(&mb,sp->pid) ;

	        serialbuf_wuchar(&mb,sp->rc) ;

	        serialbuf_wuchar(&mb,sp->ls) ;

	        serialbuf_wstrw(&mb,sp->name,SESMSG_LNAMELEN) ;

	        serialbuf_wstrw(&mb,sp->addr,SESMSG_LADDRLEN) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_listener) */


int sesmsg_mark(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_mark	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_mark ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_mark) */


int sesmsg_unknown(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_unknown	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_unknown ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_unknown) */


int sesmsg_gethelp(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_gethelp	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_ruint(&mb,&sp->idx) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_gethelp ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wuint(&mb,sp->idx) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_gethelp) */


int sesmsg_help(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_help	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_ruint(&mb,&sp->idx) ;

	        serialbuf_ruint(&mb,&sp->pid) ;

	        serialbuf_ruchar(&mb,&sp->rc) ;

	        serialbuf_rstrw(&mb,sp->name,SESMSG_LNAMELEN) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_help ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wuint(&mb,sp->idx) ;

	        serialbuf_wuint(&mb,sp->pid) ;

	        serialbuf_wuchar(&mb,sp->rc) ;

	        serialbuf_wstrw(&mb,sp->name,SESMSG_LNAMELEN) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_help) */


int sesmsg_cmd(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sesmsg_cmd	*sp ;
{
	SERIALBUF	mb ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&mb,&sp->tag) ;

	        serialbuf_rstrw(&mb,sp->cmd,SESMSG_CMDLEN) ;

	    } else { /* write */

	        sp->msgtype = sesmsgtype_cmd ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;

	        serialbuf_wuint(&mb,sp->tag) ;

	        serialbuf_wstrw(&mb,sp->cmd,SESMSG_CMDLEN) ;

	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sesmsg_cmd) */


/* local subroutines */


static int sesmsg_mbuf(sp,mt,f,mbuf,mlen)
SESMSG_MBUF	*sp ;
int		mt ;
int		f ;
char		mbuf[] ;
int		mlen ;
{
	SERIALBUF	mb ;
	const int	nlen = SESMSG_NBUFLEN ;
	const int	ulen = SESMSG_USERLEN ;
	int		rs ;
	int		rs1 ;
	if ((rs = serialbuf_start(&mb,mbuf,mlen)) >= 0) {
	    ulong	lw ;
	    uint	hdr ;
	    if (f) { /* read */
	        serialbuf_ruint(&mb,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;
	        serialbuf_rulong(&mb,&lw) ;
		sp->stime = (time_t) lw ;
	        serialbuf_ruint(&mb,&sp->tag) ;
	        serialbuf_ruchar(&mb,&sp->rc) ;
	        serialbuf_rstrw(&mb,sp->user,ulen) ;
	        serialbuf_rstrw(&mb,sp->nbuf,nlen) ;
	    } else { /* write */
	        sp->msgtype = (uchar) (mt&UCHAR_MAX) ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&mb,hdr) ;
		lw = (ulong) sp->stime ;
	        serialbuf_wulong(&mb,lw) ;
	        serialbuf_wuint(&mb,sp->tag) ;
	        serialbuf_wuchar(&mb,sp->rc) ;
	        serialbuf_wstrw(&mb,sp->user,ulen) ;
	        serialbuf_wstrw(&mb,sp->nbuf,nlen) ;
	        if ((sp->msglen = serialbuf_getlen(&mb)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }
	    } /* end if */
	    rs1 = serialbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */
	return rs ;
}
/* end subroutine (sesmsg_mbuf) */


