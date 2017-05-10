/* bopenmod */

/* "Basic I/O" package similiar to some other thing whose initials is "stdio" */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We open with a minimum mode equal to the mode passed to the
	open call.  Otherwise everything is identical to |bopen(3b)|.


*******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */

extern int	hasallof(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int bopenmod(bfile *fp,const char *fname,const char *of,mode_t om)
{
	int	rs ;

#if	CF_DEBUGS
	debugprintf("bopenmod: fname=%s\n",fname) ;
	debugprintf("bopenmod: of=%s\n",of) ;
#endif

	if ((rs = bopen(fp,fname,of,om)) >= 0) {
	    if (strchr(of,'M') == NULL) {
	        if ((rs = hasallof(of,-1,"wc")) > 0) {
	            fp->oflags |= O_MINMODE ;
	            rs = uc_fminmod(fp->fd,om) ;
	        }
	    }
	    if (rs < 0)
	        bclose(fp) ;
	} /* end if (bopen) */

#if	CF_DEBUGS
	debugprintf("bopenmod: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end routine (bopenmod) */


