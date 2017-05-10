/* sms */

/* sms gathering and manipulation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 95/12/01, David A­D­ Morano

	This module was originally written.


*/


/**************************************************************************

	This module provides a client-side implementation of the
	SMS facility.



**************************************************************************/


#define	SMS_MASTER		1


#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>

#include	"netorder.h"

#include	"localmisc.h"
#include	"sms.h"



/* local defines */

#define	SMS_MAGIC		0x93897561

#define	SMS_CMDGETLEVEL		0
#define	SMS_CMDREADSHORT	1
#define	SMS_CMDREAD		2
#define	SMS_CMDWRITE		3
#define	SMS_CMDGETPID		4

#undef	CMDBUFLEN
#define	CMDBUFLEN		100

#define	TO_READ			10	/* seconds */



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	dialudp(const char *,const char *,int,int) ;


/* forward references */







int sms_open(op,pr,hostname,portname,svc,to)
SMS	*op ;
char	pr[] ;
char	hostname[] ;
char	portname[] ;
char	svc[] ;
int	to ;
{
	int	rs, len ;

	char	cmdbuf[CMDBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("sms_init: entered\n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

	op->magic = 0 ;
	if (portname == NULL)
		return SR_FAULT ;

	if (svc == NULL)
		return SR_FAULT ;


/* local or remote ? */

	if ((hostname == NULL) || hostname[0] == '\0')) {

/* do this to be in accord with the latest BSD rules */

	if ((rs = perm(filename,-1,-1,NULL,R_OK | W_OK)) < 0)
	    return rs ;

	rs = dialuss(filename,5,0) ;

	if (rs < 0)
	    return rs ;

	op->fd = rs ;

	} else {



	} /* end if (loca/remote) */


	rs = u_fcntl(op->fd,F_GETFD,0) ;

	if ((rs & FD_CLOEXEC) == 0) {

	    rs |= FD_CLOEXEC ;
	    (void) u_fcntl(op->fd,F_SETFD,rs) ;

	} /* end if (setting CLOSE-on-EXEC) */

/* establish that the correct server is on the other end ! */

	op->pid = -1 ;

/* get the EGD program PID and check it */

	cmdbuf[0] = SMS_CMDGETPID ;
	rs = uc_writen(op->fd,cmdbuf,1) ;

	if (rs < 0)
	    goto badret ;

	rs = uc_reade(op->fd,cmdbuf,1,TO_READ,FM_EXACT) ;

	if (rs < 0)
	    goto badret ;

	len = (int) cmdbuf[0] ;
	if (len > CMDBUFLEN) {

	    rs = SR_TOOBIG ;
	    goto badret ;
	}

	rs = uc_reade(op->fd,cmdbuf,len,TO_READ,FM_EXACT) ;

	if (rs < 0)
	    goto badret ;

	if ((rs = cfdeci(cmdbuf,len,&op->pid)) < 0)
	    goto badret ;

	if ((op->pid < 0) || (op->pid >= SMS_MAXPID))
		goto badret ;
	
#if	CF_DEBUGS
	debugprintf("sms_init: we have an EGD at %W\n",cmdbuf,len) ;
#endif

	op->magic = SMS_MAGIC ;
	return rs ;

/* bad things */
badret:
	(void) u_close(op->fd) ;

	return rs ;
}
/* end subroutine (sms_open) */


/* free up the entire vector string data structure object */
int sms_close(op)
SMS	*op ;
{
	int	i ;


#if	CF_DEBUGS
	debugprintf("sms_free: entered\n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SMS_MAGIC)
	    return SR_NOTOPEN ;

	if (op->fd >= 0)
	    (void) u_close(op->fd) ;

	return SR_OK ;
}
/* end subroutine (sms_close) */


/* add some of our own sms to the mix (is this a security problem ?) */
int sms_write(op,buf,buflen)
SMS	*op ;
char	buf[] ;
int	buflen ;
{
	int	rlen, tlen, mlen ;
	int	bits ;
	int	rs, i ;

	uchar	cmdbuf[CMDBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("sms_write: entered\n") ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

	if (op->magic != SMS_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("sms_write: entered\n") ;
#endif

	if (buflen < 0)
	    buflen = strlen(buf) ;

	tlen = MIN(255,CMDBUFLEN) ;

	i = 0 ;
	rlen = buflen ;
	while (rlen > 0) {

	    mlen = MIN(tlen,rlen) ;

		bits = mlen * 8 ;
	    cmdbuf[0] = SMS_CMDWRITE ;
		cmdbuf[1] = (uchar) (bits >> 8) ;
		cmdbuf[0] = (uchar) (bits & 255) ;
	    cmdbuf[3] = (uchar) mlen ;
	    rs = uc_writen(op->fd,cmdbuf,4) ;

	    if (rs < 0)
	        return rs ;

	    rs = uc_writen(op->fd,buf + i,mlen) ;

	    if (rs < 0)
	        return rs ;

	    rlen -= mlen ;
	    i += mlen ;

	} /* end while */

	if (rs >= 0)
		rs = buflen ;

#if	CF_DEBUGS
	debugprintf("sms_write: exiting, rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sms_write) */



/* CONSTRUCTORS + DESTRUCTORS */



SMS obj_sms(filename)
char	filename[] ;
{
	SMS	temp ;


	(void) sms_init(&temp,filename) ;

	return temp ;
}


SMS *new_sms(filename)
char	filename[] ;
{
	SMS	*sop = NULL ;


	if (uc_malloc(sizeof(SMS),&sop) >= 0)
	    (void) sms_init(sop,filename) ;

	return sop ;
}


void free_sms(sop)
SMS	*sop ;
{


	(void) sms_free(sop) ;

	free(sop) ;

}



/* PRIVATE SUBROUTINES */




