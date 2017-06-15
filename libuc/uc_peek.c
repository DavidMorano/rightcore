/* uc_peek */

/* "peek" at any waiting input */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We peek ahead into the input buffer.

	Synosis:

	int uc_peek(fd,dbuf,dlen)
	int		fd ;
	void		*dbuf ;
	int		dlen ;

	Arguments:

	fd		file-descriptor
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


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */


/* forward references */

static int	peek_socket(int,void *,int) ;
static int	peek_stream(int,void *,int) ;
static int	peek_regular(int,void *,int) ;


/* exported subroutines */


int uc_peek(int fd,void *dbuf,int dlen)
{
	USTAT		sb ;
	int		rs ;

	if (dbuf == NULL) return SR_FAULT ;

	if (fd < 0) return SR_NOTOPEN ;

	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    if (S_ISSOCK(sb.st_mode)) {
		rs = peek_socket(fd,dbuf,dlen) ;
	    } else if (S_ISCHR(sb.st_mode)) {
		rs = peek_stream(fd,dbuf,dlen) ;
	    } else {
		rs = peek_regular(fd,dbuf,dlen) ;
	    }
	} /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("uc_peek: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_peek) */


/* local subroutines */


static int peek_socket(int fd,void *dbuf,int dlen)
{
	const int	mopts = MSG_PEEK ;
	return u_recv(fd,dbuf,dlen,mopts) ;
}
/* end subroutine (peek_socket) */


static int peek_stream(int fd,void *dbuf,int dlen)
{
	int		rs ;
	int		len = 0 ;

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
/* end subroutine (peek_stream) */


static int peek_regular(int fd,void *dbuf,int dlen)
{
	offset_t	fo ;
	int		rs ;
	if ((rs = u_tell(fd,&fo)) >= 0) {
	    rs = u_pread(fd,dbuf,dlen,fo) ;
	} /* end if (u_tell) */
	return rs ;
}
/* end subroutine (peek_regular) */


