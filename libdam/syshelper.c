/* syshelper */

/* syshelper gathering and manipulation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This modules provide a convenient way to perform some
	operations with the Entropy Gathering Daemon (EGD) that
	may be running on the current system.

	Commands to the Entropy Gathering Daemon (EGD) :

 0x00 (get syshelper level)
  0xMM (msb) 0xmm 0xll 0xLL (lsb)
 0x01 (read syshelper nonblocking) 0xNN (bytes requested)
  0xMM (bytes granted) MM bytes
 0x02 (read syshelper blocking) 0xNN (bytes desired)
  [block] NN bytes
 0x03 (write syshelper) 0xMM 0xLL (bits of syshelper) 0xNN (bytes of data) NN bytes
 0x04 (report PID)
  0xMM (length of PID string, not null-terminated) MM chars


**************************************************************************/


#define	SYSHELPER_MASTER	1


#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<netorder.h>
#include	<localmisc.h>

#include	"syshelper.h"


/* local defines */

#define	SYSHELPER_DEFFILE	"/tmp/syshelper"
#define	SYSHELPER_MAGIC		0x93847561
#define	SYSHELPER_MAXPID	(2 << 15)

#define	SYSHELPER_CMDGETLEVEL	0
#define	SYSHELPER_CMDREADSHORT	1
#define	SYSHELPER_CMDREAD	2
#define	SYSHELPER_CMDWRITE	3
#define	SYSHELPER_CMDGETPID	4

#undef	CMDBUFLEN
#define	CMDBUFLEN		100

#define	TO_READ			10	/* seconds */


/* external subroutines */

extern int	cfdeci(char *,int,int *) ;
extern int	dialuss(char *,int,int) ;


/* forward references */


/* exported subroutines */


int syshelper_start(eop,filename)
SYSHELPER	*eop ;
const char	filename[] ;
{
	int	rs ;
	int	len ;

	char	cmdbuf[CMDBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("syshelper_start: ent\n") ;
#endif

	if (eop == NULL)
	    return SR_FAULT ;

	eop->magic = 0 ;
	if (filename == NULL)
	    filename = SYSHELPER_DEFFILE ;

/* do this to be in accord with the latest BSD rules */

	if ((rs = perm(filename,-1,-1,NULL,R_OK | W_OK)) < 0)
	    return rs ;

	rs = dialuss(filename,5,0) ;
	eop->fd = rs ;
	if (rs < 0)
	    return rs ;

	rs = u_fcntl(eop->fd,F_GETFD,0) :

	if ((rs & FD_CLOEXEC) == 0) {

	    rs |= FD_CLOEXEC ;
	    (void) u_fcntl(eop->fd,F_SETFD,rs) ;

	} /* end if (setting CLOSE-on-EXEC) */

/* establish that the correct server is on the other end ! */

	eop->pid = -1 ;

/* get the EGD program PID and check it */

	cmdbuf[0] = SYSHELPER_CMDGETPID ;
	rs = uc_writen(eop->fd,cmdbuf,1) ;
	if (rs < 0)
	    goto badret ;

	rs = uc_reade(eop->fd,cmdbuf,1, TO_READ, FM_EXACT) ;
	if (rs < 0)
	    goto badret ;

	len = (int) cmdbuf[0] ;
	if (len > CMDBUFLEN) {

	    rs = SR_TOOBIG ;
	    goto badret ;
	}

	rs = uc_reade(eop->fd,cmdbuf,len, TO_READ, FM_EXACT) ;
	if (rs < 0)
	    goto badret ;

	if ((rs = cfdeci(cmdbuf,len,&eop->pid)) < 0)
	    goto badret ;

	if ((eop->pid < 0) || (eop->pid >= SYSHELPER_MAXPID))
		goto badret ;
	
#if	CF_DEBUGS
	debugprintf("syshelper_start: we have an EGD at %W\n",cmdbuf,len) ;
#endif

	eop->magic = SYSHELPER_MAGIC ;
	return rs ;

/* bad things */
badret:
	u_close(eop->fd) ;

	return rs ;
}
/* end subroutine (syshelper_start) */


/* free up the entire vector string data structure object */
int syshelper_finish(eop)
SYSHELPER	*eop ;
{
	int	i ;


#if	CF_DEBUGS
	debugprintf("syshelper_finish: ent\n") ;
#endif

	if (eop == NULL)
	    return SR_FAULT ;

	if (eop->magic != SYSHELPER_MAGIC)
	    return SR_NOTOPEN ;

	if (eop->fd >= 0) {
	    u_close(eop->fd) ;
	    eop->fd = -1 ;
	}

	return SR_OK ;
}
/* end subroutine (syshelper_finish) */


/* add some of our own syshelper to the mix (is this a security problem?) */
int syshelper_write(eop,buf,buflen)
SYSHELPER	*eop ;
char		buf[] ;
int		buflen ;
{
	int	rs ;
	int	rlen, tlen, mlen ;
	int	bits ;
	int	i ;

	uchar	cmdbuf[CMDBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("syshelper_write: ent\n") ;
#endif

	if (eop == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

	if (eop->magic != SYSHELPER_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("syshelper_write: ent\n") ;
#endif

	if (buflen < 0)
	    buflen = strlen(buf) ;

	tlen = MIN(255,CMDBUFLEN) ;

	i = 0 ;
	rlen = buflen ;
	while (rlen > 0) {

	    mlen = MIN(tlen,rlen) ;

		bits = mlen * 8 ;
	    cmdbuf[0] = SYSHELPER_CMDWRITE ;
		cmdbuf[1] = (uchar) (bits >> 8) ;
		cmdbuf[0] = (uchar) (bits & 255) ;
	    cmdbuf[3] = (uchar) mlen ;
	    rs = uc_writen(eop->fd,cmdbuf,4) ;

	    if (rs < 0)
	        return rs ;

	    rs = uc_writen(eop->fd,buf + i,mlen) ;

	    if (rs < 0)
	        return rs ;

	    rlen -= mlen ;
	    i += mlen ;

	} /* end while */

	if (rs >= 0)
		rs = buflen ;

#if	CF_DEBUGS
	debugprintf("syshelper_write: exiting, rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (syshelper_write) */


/* read some syshelper */
int syshelper_read(eop,buf,buflen)
SYSHELPER	*eop ;
char		buf[] ;
int		buflen ;
{
	int	rs ;
	int	i ;
	int	rlen, tlen, mlen ;

	uchar	cmdbuf[CMDBUFLEN + 1] ;


	if (eop == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

	if (eop->magic != SYSHELPER_MAGIC)
	    return SR_NOTOPEN ;

	tlen = MIN(255,CMDBUFLEN) ;

	i = 0 ;
	rlen = buflen ;
	while (rlen > 0) {

	    mlen = MIN(tlen,rlen) ;

	    cmdbuf[0] = SYSHELPER_CMDREAD ;
	    cmdbuf[1] = (uchar) mlen ;
	    rs = uc_writen(eop->fd,cmdbuf,2) ;

	    if (rs < 0)
	        return rs ;

	    rs = uc_reade(eop->fd,buf + i,mlen, TO_READ, FM_EXACT) ;

	    if (rs < 0)
	        return rs ;

	    rlen -= mlen ;
	    i += mlen ;

	} /* end while */

	return mlen ;
}
/* end subroutine (syshelper_read) */


/* return the level of syshelper available */
int syshelper_level(eop)
SYSHELPER	*eop ;
{
	int	rs ;
	int	len ;
	int	niw ;

	char	cmdbuf[CMDBUFLEN + 1] ;


	if (eop == NULL)
	    return SR_FAULT ;

	if (eop->magic != SYSHELPER_MAGIC)
	    return SR_NOTOPEN ;

	cmdbuf[0] = SYSHELPER_CMDGETLEVEL ;
	rs = uc_writen(eop->fd,cmdbuf,1) ;

	if (rs < 0)
	    return rs ;

	rs = uc_reade(eop->fd,cmdbuf,4, TO_READ, FM_EXACT) ;

	if (rs < 0)
	    return rs ;

	netorder_rint(cmdbuf,&niw) ;

	rs = niw & INT_MAX ;

	return rs ;
}
/* end subroutine (syshelper_level) */


/* return the daemon PID */
int syshelper_getpid(eop,pidp)
SYSHELPER	*eop ;
pid_t		*pidp ;
{
	int	rs = SR_OK ;


	if (eop == NULL)
	    return SR_FAULT ;

	if (eop->magic != SYSHELPER_MAGIC)
	    return SR_NOTOPEN ;

	if (pidp != NULL)
	    *pidp = (pid_t) eop->pid ;

	rs = (int) eop->pid ;
	return rs ;
}
/* end subroutine (syshelper_getpid) */


