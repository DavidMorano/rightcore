/* bufstr */

/* string buffer object */


#define	CF_FASTGROW	1		/* grow faster */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module can be used to construct strings or messages in buffers
	WITHOUT using the 'sprint' subroutine.

	This module is useful when the user does NOT supply a buffer to be used
	as the working store.  Rather, an internally, dynamically grown and
	managed buffer is maintained.

	This module uses an object, that must be initialized and eventually
	freed, to track the state of the dynamically used internal buffer.


*******************************************************************************/


#define	BUFSTR_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bufstr.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		bufstr_strw(BUFSTR *,const char *,int) ;

static int	bufstr_extend(BUFSTR *,int,char **) ;


/* exported subroutines */


int bufstr_start(BUFSTR	 *op)
{

	if (op == NULL) return SR_FAULT ;

	op->len = 0 ;
	op->dlen = 0 ;
	op->dbuf = NULL ;
	op->sbuf[0] = '\0' ;
	return SR_OK ;
}
/* end subroutine (bufstr_start) */


int bufstr_finish(BUFSTR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->dbuf != NULL) {
	    rs1 = uc_free(op->dbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbuf = NULL ;
	}

	if (rs >= 0) rs = op->len ;
	op->len = 0 ;
	op->dlen = 0 ;
	return rs ;
}
/* end subroutine (bufstr_finish) */


int bufstr_char(BUFSTR *op,int ch)
{
	int		rs ;
	char		buf[2] ;

	buf[0] = (char) ch ;
	rs = bufstr_strw(op,buf,1) ;

	return rs ;
}
/* end subroutine (bufstr_char) */


int bufstr_strw(BUFSTR *op,cchar *sp,int sl)
{
	int		rs ;
	char		*bp ;

	if (op == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

	if ((rs = bufstr_extend(op,sl,&bp)) >= 0) {
	    strwcpy(bp,sp,sl) ;
	    op->len += sl ;
	}

	return (rs >= 0) ? op->len : rs ;
}
/* end subroutine (bufstr_strw) */


int bufstr_buf(BUFSTR *op,cchar sp[],int sl)
{
	int		rs ;
	char		*bp ;

	if (op == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

	if ((rs = bufstr_extend(op,sl,&bp)) >= 0) {
	    memcpy(bp,sp,sl) ;
	    op->len += sl ;
	}

	return (rs >= 0) ? op->len : rs ;
}
/* end subroutine (bufstr_buf) */


/* get the pointer the object string */
int bufstr_get(BUFSTR *op,cchar **spp)
{

	if (op == NULL) return SR_FAULT ;

	if (spp != NULL)
	    *spp = (op->dbuf != NULL) ? op->dbuf : op->sbuf ;

	return op->len ;
}
/* end subroutine (bufstr_get) */


/* private subroutines */


static int bufstr_extend(BUFSTR *op,int nlen,char **rpp)
{
	const int	slen = BUFSTR_LEN ;
	int		rs = SR_OK ;
	int		dlen ;
	int		rlen ;
	char		*dp ;

#if	CF_DEBUGS
	debugprintf("buffer_extend: nlen=%d\n",nlen) ;
	debugprintf("buffer_extend: e=%u l=%d\n",op->dlen,op->len) ;
#endif

	if (op->dbuf == NULL) {
	    rlen = (slen-op->len) ;
	    if (nlen > rlen) {
	    	dlen = MAX((slen + nlen),(slen * 2)) ;
		if ((rs = uc_malloc((dlen+1),&dp)) >= 0) {
		    op->dlen = dlen ;
		    op->dbuf = dp ;
		    dp = strwcpy(dp,op->sbuf,op->len) ;
		}
	    } else {
		dp = (op->sbuf+op->len) ;
	    }
	} else {
	    rlen = (op->dlen-op->len) ;
	    if (nlen > rlen) {
		dlen = (op->dlen+nlen+slen) ;
		if ((rs = uc_realloc(op->dbuf,(dlen+1),&dp)) >= 0) {
		    op->dbuf = dp ;
		    op->dlen = dlen ;
		}
	    } else {
		dp = (op->dbuf+op->len) ;
	    }
	} /* end if (extension needed) */

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? dp : NULL ;

#if	CF_DEBUGS
	debugprintf("buffer_extend: ret rs=%d e=%u\n",rs,op->dlen) ;
#endif

	return rs ;
}
/* end subroutine (bufstr_extend) */


