/* mailmsg_enver */

/* MAILMSG get-envelope */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DIRECT	1		/* try the more direct approach */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine gets information about a MAILMSG envelope.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<mailmsg.h>
#include	<mailmsgmatenv.h>
#include	<localmisc.h>

#include	"mailmsg_enver.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	HDRNAMELEN
#define	HDRNAMELEN	80
#endif

#ifndef	MSGLINELEN
#define	MSGLINELEN	(2 * 1024)
#endif

#ifndef	MAXMSGLINELEN
#define	MAXMSGLINELEN	76
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;

extern int	sfsubstance(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

#if	CF_DIRECT
#else
static int	isNotField(int) ;
#endif


/* local variables */

#if	CF_DIRECT
#else /* CF_DIRECT */
static const int	rsnofield[] = {
	SR_NOENT,
	SR_NOMSG,
	0
} ;
#endif /* CF_DIRECT */


/* exported subroutines */


#if	CF_DIRECT
int mailmsg_enver(MAILMSG *msgp,int ei,MAILMSG_ENVER *mep)
{
	int		rs ;

	if (msgp == NULL) return SR_FAULT ;
	if (mep == NULL) return SR_FAULT ;
	if (msgp->magic != MAILMSG_MAGIC) return SR_NOTOPEN ;

	{
	    MAILMSG_ENV		*eop = &msgp->envs ;
	    MAILMSGMATENV	*ep ;
	    if ((rs = vecobj_get(&eop->insts,ei,&ep)) >= 0) {
	        mep->a.ep = ep->a.ep ;
	        mep->a.el = ep->a.el ;
	        mep->d.ep = ep->d.ep ;
	        mep->d.el = ep->d.el ;
	        mep->r.ep = ep->r.ep ;
	        mep->r.el = ep->r.el ;
	    }
	}

	return rs ;
}
/* end subroutine (mailmsg_enver) */
#else /* CF_DIRECT */
int mailmsg_enver(MAILMSG *msgp,int ei,MAILMSG_ENVER *mep)
{
	int		rs ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("mailmsg_enver: ei=%u\n",ei) ;
#endif

	memset(mep,0,sizeof(MAILMSG_ENVER)) ;

	if ((rs = mailmsg_envaddress(msgp,ei,&sp)) >= 0) {
	    mep->a.el = rs ;
	    mep->a.ep = sp ;
#if	CF_DEBUGS
	    debugprintf("mailmsg_enver: a=>%t<\n",sp,rs) ;
#endif
	    if ((rs = mailmsg_envdate(msgp,ei,&sp)) >= 0) {
	        mep->d.el = rs ;
	        mep->d.ep = sp ;
#if	CF_DEBUGS
	        debugprintf("mailmsg_enver: d=>%t<\n",sp,rs) ;
#endif
	        if ((rs = mailmsg_envremote(msgp,ei,&sp)) >= 0) {
	            mep->r.el = rs ;
	            mep->r.ep = sp ;
#if	CF_DEBUGS
	            debugprintf("mailmsg_enver: r=>%t<\n",sp,rs) ;
#endif
	        } else if (isNotField(rs)) {
	            rs = SR_OK ;
		}
	    } /* end if (mailmsg_envdate) */
	} /* end if (mailmsg_envaddress) */

#if	CF_DEBUGS
	debugprintf("mailmsg_enver: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsg_enver) */
#endif /* CF_DIRECT */


/* provate subroutines */


#if	CF_DIRECT
#else /* CF_DIRECT */
static int isNotField(int rs)
{
	return isOneOf(rsnofield,rs) ;
}
/* end subroutine (isNotField) */
#endif /* CF_DIRECT */


