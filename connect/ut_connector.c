/* ut_connector */

/* this is a higher level function that does a |ut_connect| */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_PUSHMOD	0		/* push TIRDWR */


/* revision history:

	= 1998-04-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We do a |ut_connect(3ut)| without all of the hassle.

	Synopsis:

	int uc_connector(fd,addr,alen)
	int		fd ;
	const char	addr[] ;
	int		alen ;

	Arguments:

	fd		file-descriptor
	addr		pointer to XTI address
	alen		length of XTI address

	Returns:

	<0		error
	>=0		success


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	mkhexstr(char *,int,const void *,int) ;
#endif


/* local variables */


/* exported subroutines */


int ut_connector(int fd,cchar *abuf,int alen)
{
	struct t_call	*sndcall ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("dialticotsord: ent\n") ;
#endif

	if (abuf == NULL) return SR_FAULT ;

	if ((rs = ut_alloc(fd,T_CALL,0,(void **) &sndcall)) >= 0) {
	    sndcall->addr.maxlen = alen ;
	    sndcall->addr.buf = (char *) abuf ;
	    sndcall->addr.len = alen ;

	    rs = ut_connect(fd,sndcall,NULL) ;

	    sndcall->addr.maxlen = 0 ;
	    sndcall->addr.buf = NULL ;
	    sndcall->addr.len = 0 ;
	    ut_free(sndcall,T_CALL) ;
	} /* end if (alloc) */

	return rs ;
}
/* end subroutine (ut_connector) */


