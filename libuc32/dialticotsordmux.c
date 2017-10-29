/* dialticotsordmux */

/* dial to the server listening on USSMUX */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This object dials out to a UNIX®-Socket-Stream (USS) that implements
	a multiplexor on the server side.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<buffer.h>
#include	<sigblock.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define	DBUFLEN		(8 * 1024)

#ifndef	PORTSPEC_USSMUX
#define	PORTSPEC_USSMUX	"/tmp/ussmux"
#endif


/* external subroutines */

extern int	dialticotsord(const char *,int,int,int) ;
extern int	mkquoted(char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	dialer(BUFFER *,cchar *,int,char *,int,int,int) ;


/* local variables */

static const int	sigblocks[] = {
	SIGPIPE,
	0
} ;


/* exported subroutines */


int dialticotsordmux(abuf,alen,svcspec,srvargv,to,opts)
const char	abuf[] ;
int		alen ;
const char	svcspec[] ;
const char	*srvargv[] ;
int		to ;
int		opts ;
{
	BUFFER		srvbuf ;
	int		rs ;
	int		rs1 ;
	int		svclen ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("dialticotsordmux: ent to=%d opts=%04x\n",
	    to,opts) ;
#endif

	if (abuf == NULL) return SR_FAULT ;
	if (svcspec == NULL) return SR_FAULT ;

	if (svcspec[0] == '\0') return SR_INVALID ;

	while (CHAR_ISWHITE(*svcspec)) {
	    svcspec += 1 ;
	}

	svclen = strlen(svcspec) ;

	while (svclen && CHAR_ISWHITE(svcspec[svclen - 1])) {
	    svclen -= 1 ;
	}

	if (svclen <= 0)
	    return SR_INVAL ;

#if	CF_DEBUGS
	debugprintf("dialticotsordmux: final svcspec=%t\n",svcspec,svclen) ;
#endif

/* format the service string to be transmitted */

	if ((rs = buffer_start(&srvbuf,100)) >= 0) {
	    const int	dlen = DBUFLEN ;
	    char	*dbuf ;
	    if ((rs = uc_malloc((dlen+1),&dbuf)) >= 0) {

/* format the service code and arguments for transmission */

	        buffer_strw(&srvbuf,svcspec,svclen) ;

	        if (srvargv != NULL) {
	            int		i ;
	            for (i = 0 ; (rs >= 0) && (srvargv[i] != NULL) ; i += 1) {
	                if ((rs = mkquoted(dbuf,dlen,srvargv[i],-1)) >= 0) {
	                    buffer_char(&srvbuf,' ') ;
	                    buffer_buf(&srvbuf,dbuf,rs) ;
	                }
	            } /* end for */
	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("dialticotsordmux: finishing service format\n") ;
#endif

	        if (rs >= 0) {
	            buffer_char(&srvbuf,'\n') ;
	        }

	        if (rs >= 0) {
	            rs = dialer(&srvbuf,abuf,alen,dbuf,dlen,to,opts) ;
	            fd = rs ;
	        } /* end if (positive) */

	        uc_free(dbuf) ;
	    } /* end if (m-a-f) */
	    rs1 = buffer_finish(&srvbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */
	if ((rs >= 0) && (fd >= 0)) u_close(fd) ;

#if	CF_DEBUGS
	debugprintf("dialticotsordmux: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialticotsordmux) */


/* local subroutines */


static int dialer(BUFFER *sbp,cchar *abuf,int alen,char *dbuf,int dlen,
int to,int opts)
{
	SIGBLOCK	ss ;
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	cchar		*bp ;
	if ((rs = buffer_get(sbp,&bp)) >= 0) {
	    const int	blen = rs ;
	    if ((rs = sigblock_start(&ss,sigblocks)) >= 0) {

	        if ((rs = dialticotsord(abuf,alen,to,opts)) >= 0) {
	            fd = rs ;

	            if ((rs = uc_writen(fd,bp,blen)) >= 0) {
	                dbuf[0] = '\0' ;
	                if ((rs = uc_readlinetimed(fd,dbuf,dlen,to)) >= 0) {
	                    if ((rs == 0) || (dbuf[0] != '+')) {
	                        rs = SR_BADREQUEST ;
	                    }
	                }
	            } /* end if (wrote service code) */

#if	CF_DEBUGS
	            debugprintf("dialticotsordmux: recv rs=%d\n",rs) ;
#endif

	            if (rs < 0) {
	                u_close(fd) ;
	                fd = -1 ;
	            }
	        } /* end if (opened) */

	        rs1 = sigblock_finish(&ss) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sigblock) */
	} /* end if (buffer_get) */
	return (rs >= 0) ? fd : rs ;
}
/* end subroutiner (dialer) */


