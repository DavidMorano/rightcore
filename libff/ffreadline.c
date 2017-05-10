/* ffreadline */

/* read a coded line from the STDIO stream */


#define	CF_FGETS	1	/* faster or not? */


/* revision history:

	= 1986-01-17, David A­D­ Morano

	This subroutine was originally written.

	= 2001-09-10, David A­D­ Morano

	I discovered that on the SGI Irix systems, 'getc()' does
	not seem to work properly so I hacked it out for that
	system.


*/

/* Copyright © 1986-2001 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This routine will only read at most 'len' number of bytes
	from the file.

	Note that the sematics of this call are not the same as
	'fgets(3c)'.  This call will write a NULLCHAR into the user
	buffer after the supplied length of the buffer is used up.
	With 'fgets(3c)', it will never write more than the user's
	supplied length of bytes.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>

#include	<vsystem.h>

#include	"ffile.h"


/* local defines */

#if	defined(OSNAME_IRIX)		/* IRIX screws up! (what else is new) */
#define	mygetc(p)	fgetc(p)
#else
#define	mygetc(p)	getc(p)
#endif


/* forward references */

#define	TO_AGAIN	(5*60)

#if	CF_FGETS
static int ffreadline_gets(FFILE *,char *,int) ;
#else
static int ffreadline_getc(FFILE *,char *,int) ;
#endif /* CF_FGETS */


/* exported subroutines */


int ffreadline(fp,buf,len)
FFILE	*fp ;
char	buf[] ;
int	len ;
{
	int	rs = SR_OK ;
	int	to = TO_AGAIN ;


	if (fp == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;
	if (len < 1) return SR_INVALID ;

again:
	if (len > 0) {
#if	CF_FGETS
	    rs = ffreadline_gets(fp,buf,len) ;
#else
	    rs = ffreadline_getc(fp,buf,len) ;
#endif /* CF_FGETS */
	}

	if (rs < 0) {
	    switch (rs) {
	    case SR_INTR:
		goto again ;
	    case SR_AGAIN:
	        if (to-- > 0) {
		    sleep(1) ;
		    goto again ;
		}
		break ;
	    } /* end switch */
	} /* end if */

	return rs ;
}
/* end subroutine (ffreadline) */


/* local subroutines */


#if	CF_FGETS

static int ffreadline_gets(fp,buf,len)
FFILE		*fp ;
char		*buf ;
int		len ;
{
	FILE	*sfp = fp->sfp ;

	int	rs = 0 ;
	char	*bp ;

	bp = fgets(buf,(len + 1),sfp) ;
	if (bp == NULL) {
		if (ferror(sfp)) {
		    rs = (- errno) ;
		} else if (feof(sfp)) {
		    rs = 0 ;
		} else {
		    rs = SR_NOANODE ;
		}
	    clearerr(sfp) ;
	} else
	    rs = strlen(bp) ;

	return rs ;
}
/* end subroutine (ffreadline_gets) */

#else /* CF_FGETS */

static int ffreadline_getc(fp,buf,len)
FFILE		*fp ;
char		*buf ;
int		len ;
{
	int	rs ;
	int	len = 0 ;
	char	*bp = buf ;

	while ((rs = ffgetc(fp)) >= 0) {
	    *bp++ = rs ;
	    len += 1 ;
	    if (rs == '\n') break ;
	} /* end while */

	if (rs == SR_EOF) rs = SR_OK ;

	*bp = '\0' ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ffreadline_getc) */

#endif /* CF_FGETS */



