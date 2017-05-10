/* bminmod */

/* "Basic I/O" package similiar to some other thing whose initials is "stdio" */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_MEMCPY	1		/* use 'memcpy(3c)' */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Sset a minimum mode on the opened file.


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

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int bminmod(bfile *fp,mode_t om)
{
	int	rs ;

	if (fp == NULL) return SR_FAULT ;
	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	rs = uc_fminmod(fp->fd,om) ;

	return rs ;
}
/* end routine (bminmod) */


