/* tcpeek */

/* terminal "peek" */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We peek ahead in the terminal input buffer.

	Synosis:

	int tcpeek(fd,dbuf,dlen)
	int	fd ;
	char	dbuf[] ;
	int	dlen ;

	Arguments:

	fd		file-descriptor of terminal
	dbuf		data buffer
	dlen		data buffer size provided

	Return:

	<0		error
	>=0		length of characters peeked at


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
#define	CF_STREAM	1
#else
#define	CF_STREAM	0
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<termios.h>
#include	<stdlib.h>
#include	<string.h>

#if	CF_STREAM
#include	<stropts.h>
#endif

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	CBUFLEN
#define	CBUFLEN		LINEBUFLEN
#endif


/* exported subroutines */


int tcpeek(int fd,char *dbuf,int dlen)
{
	int		rs ;
	int		len = 0 ;

	if (dbuf == NULL) return SR_FAULT ;

	if (fd < 0) return SR_NOTOPEN ;

#if	CF_STREAM
	{
	    const int	clen = CBUFLEN ;
	    char	*cbuf ;
	    if ((rs = uc_libmalloc((clen+1),&cbuf)) >= 0) {
	        struct strpeek	pd ;
	        memset(&pd,0,sizeof(struct strpeek)) ;
	        pd.flags = 0 ;
	        pd.ctlbuf.buf = cbuf ;
	        pd.ctlbuf.maxlen = clen ;
	        pd.databuf.buf = dbuf ;
	        pd.databuf.maxlen = dlen ;
	        rs = u_ioctl(fd,I_PEEK,&pd) ;
	        len = pd.databuf.len ;
		uc_libfree(cbuf) ;
	    } /* end if (m-a-f) */
	} /* end block */
#else
	dbuf[0] = '\0' ;
	rs = SR_NOSYS ;
#endif /* CF_STREAM */

	return (rs > 0) ? len : rs ;
}
/* end subroutine (tcpeek) */


