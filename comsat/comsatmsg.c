/* comsatmsg */

/* create and parse COMSAT messages */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the code to make and parse Comsat messages.

	A Comsat message looks like:

		<user>@<offset>[:<file>]

	Where:

		<user>		is the user who received the message
		<offset>	is the file-offset to this message
		<file>		associated filename (If present)

	Example:

		dam@251:bobby


*******************************************************************************/


#define	COMSATMSG_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"comsatmsg.h"


/* local defines */


/* external subroutines */

extern int	mkpath1w(char *,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	cfdecl(const char *,int,long *) ;
extern int	cfdecul(const char *,int,ulong *) ;

extern char	*strnchr(const char *,int,int) ;
extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* local variables */


/* exported subroutines */


int comsatmsg_mo(COMSATMSG_MO *msp,int f_read,char *mbuf,int mlen)
{
	SBUF		msgbuf ;
	ulong		ulv ;
	int		rs ;
	int		rs1 ;

	if (f_read) { /* read */
	    int		cl ;
	    int		sl = mlen ;
	    const char	*tp, *cp ;
	    const char	*sp = mbuf ;

	    msp->offset = 0 ;
	    msp->username[0] = '\0' ;
	    msp->fname[0] = '\0' ;

	    while ((sl > 0) && CHAR_ISWHITE(sp[sl - 1])) {
	        sl -= 1 ;
	    }

	    if ((tp = strnchr(sp,sl,'@')) != NULL) {
	        const int	ulen = USERNAMELEN ;
	        char		*ubuf = msp->username ;

	        if ((rs = sncpy1w(ubuf,ulen,sp,(tp-sp))) >= 0) {

	            sl -= ((tp + 1) - mbuf) ;
	            sp = (tp + 1) ;

	            cp = sp ;
	            cl = sl ;
	            if ((tp = strnchr(sp,sl,':')) != NULL) {
	                cl = (tp - sp) ;
	                if (rs >= 0) {
	                    char	*fbuf = msp->fname ;
	                    rs = mkpath1w(fbuf,(tp+1),((sp+sl)-(tp+1))) ;
	                }
	            }

	            if (rs >= 0) {
	                rs = cfdecul(cp,cl,&ulv) ;
	                msp->offset = ulv ;
	            }

	        } /* end if */

	    } else
	        rs = SR_BADMSG ;

	} else { /* write */

	    if ((rs = sbuf_start(&msgbuf,mbuf,mlen)) >= 0) {

	        sbuf_strw(&msgbuf,msp->username,-1) ;

	        sbuf_char(&msgbuf,'@') ;

	        ulv = msp->offset ;
	        sbuf_decul(&msgbuf,ulv) ;

	        if (msp->fname[0] != '\0') {

	            sbuf_char(&msgbuf,':') ;

	            sbuf_strw(&msgbuf,msp->fname,-1) ;

	        } /* end if */

	        rs1 = sbuf_finish(&msgbuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sbuf) */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("comsatmsg_mo: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (comsatmsg_mo) */


