/* extraunix */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<poll.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int readn(fd,buf,len)
int	fd ;
char	buf[] ;
int	len ;
{


	return uc_readn(fd,buf,len) ;
}


int writen(fd,buf,len)
int	fd ;
char	buf[] ;
int	len ;
{


	return uc_writen(fd,buf,len) ;
}


int reade(fd,buf,buflen,opts,timeout)
int	fd ;
char	buf[] ;
int	buflen ;
int	opts, timeout ;
{
	int	rs ;


	rs = uc_reade(fd,buf,buflen,timeout,opts) ;

	return rs ;
}



