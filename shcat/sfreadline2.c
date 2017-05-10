/* sfreadline */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#if	defined(SFIO) || defined(KSHBUILTIN)
#undef	CF_SFIO
#define	CF_SFIO	1
#endif

#if	CF_SFIO
#include	<sfio.h>
#endif

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int sfreadlinetimed(fp,buf,buflen,timeout,opts)
Sfio_t	*fp ;
char	buf[] ;
int	buflen ;
int	timeout ;
int	opts ;
{
	Sfio_t		*streams[2] ;
	int		rs = 0 ;
	int		i = 0 ;

	streams[0] = fp ;
	streams[1] = NULL ;
	while (i < buflen) {

	    if ((rs = sfread(fp,(buf + i),1)) < 0)
		rs = SR_HANGUP ;

	    if (rs <= 0)
	        break ;

	    if (buf[i++] == '\n')
	        break ;

	} /* end while */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sfreadlinetimed) */


int sfreadline(fp,buf,buflen)
Sfio_t	*fp ;
char	buf[] ;
int	buflen ;
{

	return sfreadlinetimed(fp,buf,buflen,-1,0) ;
}
/* end subroutine (sfreadline) */


