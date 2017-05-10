/* egs */

/* entropy gathering and manipulation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module provides a convenient way to perform some operations with
	the Entropy Gathering Daemon (EGD) that may be running on the current
	system.

	Commands to the Entropy Gathering Daemon (EGD):

 0x00 (get entropy level)
  0xMM (msb) 0xmm 0xll 0xLL (lsb)
 0x01 (read entropy nonblocking) 0xNN (bytes requested)
  0xMM (bytes granted) MM bytes
 0x02 (read entropy blocking) 0xNN (bytes desired)
  [block] NN bytes
 0x03 (write entropy) 0xMM 0xLL (bits of entropy) 0xNN (bytes of data) NN bytes
 0x04 (report PID)
  0xMM (length of PID string, not null-terminated) MM chars


*******************************************************************************/


#define	EGS_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<netorder.h>
#include	<localmisc.h>

#include	"egs.h"


/* local defines */

#ifndef	PID_MAX
#define	PID_MAX			(6*10)
#endif

#define	EGS_CMDGETLEVEL		0
#define	EGS_CMDREADSHORT	1
#define	EGS_CMDREAD		2
#define	EGS_CMDWRITE		3
#define	EGS_CMDGETPID		4

#undef	CMDBUFLEN
#define	CMDBUFLEN		100

#define	TO_READ			10	/* seconds */


/* external subroutines */

extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;


/* forward references */

static int egs_opencheck(EGS *) ;


/* exported subroutines */


int egs_open(eop,filename)
EGS		*eop ;
const char	filename[] ;
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("egs_start: ent\n") ;
#endif

	if (eop == NULL) return SR_FAULT ;

	eop->magic = 0 ;
	eop->pid = -1 ;
	if (filename == NULL) filename = EGS_DEFFILE ;

/* do this to be in accord with the latest BSD rules */

	if ((rs = perm(filename,-1,-1,NULL,(R_OK | W_OK))) >= 0) {
	    if ((rs = dialuss(filename,5,0)) >= 0) {
	        eop->fd = rs ;
	        if ((uc_closeonexec(eop->fd,TRUE)) >= 0) {
		    if ((rs = egs_opencheck(eop)) >= 0) {
		        eop->magic = EGS_MAGIC ;
		    }
	        } /* end if (closeonexec) */
	        if (rs < 0) {
		    u_close(eop->fd) ;
		    eop->fd = -1 ;
		}
	    } /* end if (dialuss) */
	} /* end if (perm) */

	return rs ;
}
/* end subroutine (egs_open) */


/* free up the entire vector string data structure object */
int egs_close(eop)
EGS		*eop ;
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("egs_close: ent\n") ;
#endif

	if (eop == NULL) return SR_FAULT ;

	if (eop->magic != EGS_MAGIC) return SR_NOTOPEN ;

	if (eop->fd >= 0) {
	    rs1 = u_close(eop->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    eop->fd = -1 ;
	}

	eop->magic = 0 ;
	return rs ;
}
/* end subroutine (egs_close) */


/* add some of our own entropy to the mix (is this a security problem?) */
int egs_write(eop,buf,buflen)
EGS		*eop ;
char		buf[] ;
int		buflen ;
{
	const int	tlen = MIN(255,CMDBUFLEN) ;
	int		rs = SR_OK ;
	int		rlen, mlen ;
	int		bits ;
	int		i ;
	uchar		cmdbuf[CMDBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("egs_write: ent\n") ;
#endif

	if (eop == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (eop->magic != EGS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("egs_write: ent\n") ;
#endif

	if (buflen < 0)
	    buflen = strlen(buf) ;

	i = 0 ;
	rlen = buflen ;
	while ((rs >= 0) && (rlen > 0)) {

	    mlen = MIN(tlen,rlen) ;

		bits = mlen * 8 ;
	    cmdbuf[0] = EGS_CMDWRITE ;
		cmdbuf[1] = (uchar) (bits >> 8) ;
		cmdbuf[0] = (uchar) (bits & 255) ;
	    cmdbuf[3] = (uchar) mlen ;
	    rs = uc_writen(eop->fd,(void *) cmdbuf,4) ;

	    if (rs >= 0)
	    rs = uc_writen(eop->fd,(void *) (buf + i),mlen) ;

	    rlen -= mlen ;
	    i += mlen ;

	} /* end while */

	if (rs >= 0)
	    rs = buflen ;

#if	CF_DEBUGS
	debugprintf("egs_write: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (egs_write) */


/* read some entropy */
int egs_read(eop,buf,buflen)
EGS		*eop ;
char		buf[] ;
int		buflen ;
{
	const int	tlen = MIN(255,CMDBUFLEN) ;
	int		rs = SR_OK ;
	int		i ;
	int		len, rlen, mlen ;
	uchar		cmdbuf[CMDBUFLEN + 1] ;

	if (eop == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (eop->magic != EGS_MAGIC) return SR_NOTOPEN ;

	i = 0 ;
	rlen = buflen ;
	while (rlen > 0) {

	    mlen = MIN(tlen,rlen) ;

	    cmdbuf[0] = EGS_CMDREAD ;
	    cmdbuf[1] = (uchar) mlen ;
	    rs = uc_writen(eop->fd,(void *) cmdbuf,2) ;
	    if (rs < 0) break ;

	    rs = uc_reade(eop->fd,buf + i,mlen, TO_READ, FM_EXACT) ;
	    len = rs ;
	    if (rs < 0) break ;

#if	CF_DEBUGS
	debugprintf("egs_read: mlen=%d len=%d\n",mlen,len) ;
#endif

	    if (len == 0)
		break ;

	    rlen -= len ;
	    i += len ;

	} /* end while */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (egs_read) */


/* return the level of entropy available */
int egs_level(eop)
EGS		*eop ;
{
	int		rs ;
	int		len = 0 ;
	char		cmdbuf[CMDBUFLEN + 1] ;

	if (eop == NULL) return SR_FAULT ;

	if (eop->magic != EGS_MAGIC) return SR_NOTOPEN ;

	cmdbuf[0] = EGS_CMDGETLEVEL ;
	if ((rs = uc_writen(eop->fd,cmdbuf,1)) >= 0) {
	    if ((rs = uc_reade(eop->fd,cmdbuf,4, TO_READ, FM_EXACT)) >= 0) {
	        uint	uiw ;
	        netorder_ruint(cmdbuf,&uiw) ;
	        len = (uiw & INT_MAX) ;
	    } /* end if (uc_reade) */
	} /* end if (uc_writen) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (egs_level) */


/* return the daemon PID */
int egs_getpid(eop,pidp)
EGS	*eop ;
pid_t	*pidp ;
{
	int		rs = SR_OK ;

	if (eop == NULL) return SR_FAULT ;

	if (eop->magic != EGS_MAGIC) return SR_NOTOPEN ;

	if (pidp != NULL)
	    *pidp = (pid_t) eop->pid ;

	rs = (int) eop->pid ;
	return rs ;
}
/* end subroutine (egs_getpid) */


/* private subroutines */


/* get the EGD program PID and check it */
static int egs_opencheck(EGS *eop)
{
	int		rs ;
	int		len ;
	char		cmdbuf[CMDBUFLEN + 1] ;

	cmdbuf[0] = EGS_CMDGETPID ;
	if ((rs = uc_writen(eop->fd,cmdbuf,1)) >= 0) {
	    const int	to = TO_READ ;
	    const int	fm = FM_EXACT ;
	    if ((rs = uc_reade(eop->fd,cmdbuf,1,to,fm)) >= 0) {
		len = (int) cmdbuf[0] ;
		if (len <= CMDBUFLEN) {
		    if ((rs = uc_reade(eop->fd,cmdbuf,len,to,fm)) >= 0) {
	    		int	v ;
	    		if ((rs = cfdeci(cmdbuf,len,&v)) >= 0) {
	    		    eop->pid = (v & INT_MAX) ;
			    if ((eop->pid < 0) || (eop->pid > PID_MAX)) {
				rs = SR_PROTO ;
			    }
			} /* end if (cfdeci) */
		    } /* end if (uc_reade) */
		} else
		    rs = SR_PROTO ;
	    } /* end if (uc_reade) */
	} /* end if (uc_writen) */
	
#if	CF_DEBUGS
	debugprintf("egs_opencheck: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (egs_opencheck) */


