/* mailmsg_envaddrfold */

/* MAILMSG create a folded envelope address */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine creates a folded envelope address from the various
        envelope components.

	Synopsis:

	int mailmsg_envaddrfold(MAILMSG *mmp,char *rbuf,int rlen)

	Arguments:

	mmp		pointer to MAILMSG object
	rbuf		buffer to receive result
	rlen		length of result buffer

	Returns:

	>=0		length of result (in bytes)
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<mailmsg.h>
#include	<emainfo.h>
#include	<sbuf.h>
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
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;

extern int	sfsubstance(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mailmsg_envaddrfold(MAILMSG *mmp,char *rbuf,int rlen)
{
	const int	alen = MAILADDRLEN ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		*abuf ;

	if (mmp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsg_envaddrfold: ent\n") ;
#endif

	if ((rs = uc_malloc((alen+1),&abuf)) >= 0) {
	    SBUF	b ;
	    if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	        MAILMSG_ENVER	me, *mep = &me ;
	        int		i ;
	        int		cl ;
	        const char	*cp ;

	        for (i = 0 ; mailmsg_enver(mmp,i,mep) >= 0 ; i += 1) {
	            EMAINFO	ai ;
	            int		atype ;

	            if ((mep->r.ep != NULL) && (mep->r.el > 0)) {
	                cp = mep->r.ep ;
	                cl = mep->r.el ;
	                if (c > 0) rs = sbuf_char(&b,'!') ;
	                if (rs >= 0) {
	                    c += 1 ;
	                    rs = sbuf_strw(&b,cp,cl) ;
	                }
	            } /* end if (remote) */

	            if (rs >= 0) {
	                const int	at = EMAINFO_TUUCP ;
	                int		al ;
	                cp = mep->a.ep ;
	                cl = mep->a.el ;
	                atype = emainfo(&ai,cp,cl) ;
	                if ((al = emainfo_mktype(&ai,at,abuf,alen)) > 0) {
	                    if (c > 0) rs = sbuf_char(&b,'!') ;
	                    if (rs >= 0) {
	                        c += 1 ;
	                        rs = sbuf_strw(&b,abuf,al) ;
	                    }
	                }
	            } /* end if (address) */

	            if (rs < 0) break ;
	        } /* end for (looping through envelopes) */

	        rs1 = sbuf_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sbuf) */
	    uc_free(abuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("mailmsg_envaddrfold: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsg_envaddrfold) */


