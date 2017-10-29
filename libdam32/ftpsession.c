/* ftpsession */

/* way to access remote files with FTP */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This module supplies a subset of the features of the LibFTP
	library.  This library currently only has enough stuff to
	"read" a remote file.  Remarkably, that is all that is needed
	right now.


**************************************************************************/


#define	FTPSESSION_MASTER	1


#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<getxusername.h>
#include	<localmisc.h>

#include	"ftpsession.h"


/* local defines */

#define	FTPSESSION_MAGIC	0x93447561


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* exported subroutines */


int ftpsession_open(fsp,hostname,username,password,account)
FTPSESSION	*fsp ;
const char	hostname[] ;
const char	username[] ;
const char	password[] ;
const char	account[] ;
{
	int	rs = SR_OK ;
	int	sl = 0 ;

	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	passbuf[LOGNAMELEN + 1 + MAXHOSTNAMELEN + 1] ;


	if (fsp == NULL)
	    return SR_FAULT ;

	fsp->magic = 0 ;
	if ((hostname == NULL) || (hostname[0] == '\0'))
	    return SR_INVALID ;

	if ((username == NULL) || (username[0] == '\0'))
	    username = "anonymous" ;

	if ((password == NULL) || (password[0] == '\0')) {

	    rs = getnodedomain(NULL,domainname) ;

	    if (rs >= 0)
	        rs = getusername(passbuf,USERNAMELEN,-1) ;

	    if (rs >= 0) {
	        passbuf[sl++] = '@' ;
	        strwcpy((passbuf + sl),domainname,(MAXHOSTNAMELEN - 1)) ;
	        password = passbuf ;
	    }

	} /* end if (NULL password) */

	if (rs >= 0) {
	    rs = FtpLogin(&fsp->sp,hostname,username,password,account) ;
	    if (rs < 0) rs = (- errno) ;
	    sl = rs ;
	}

	if (rs >= 0)
	    fsp->magic = FTPSESSION_MAGIC ;

ret0:
	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (ftpsession_open) */


/* free up the data structure object */
int ftpsession_close(fsp)
FTPSESSION	*fsp ;
{
	int	rs ;


	if (fsp == NULL)
	    return SR_FAULT ;

	if (fsp->magic != FTPSESSION_MAGIC)
	    return SR_NOTOPEN ;

	rs = FtpBye(fsp->sp) ;

	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (ftpsession_close) */


/* set the transfer mode */
int ftpsession_type(fsp,type)
FTPSESSION	*fsp ;
int		type ;
{
	int	rs ;


	if (fsp == NULL)
	    return SR_FAULT ;

	if (fsp->magic != FTPSESSION_MAGIC)
	    return SR_NOTOPEN ;

	rs = FtpType(fsp->sp,type) ;

	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (ftpsession_setmode) */


/* set BINARY mode */
int ftpsession_binary(fsp)
FTPSESSION	*fsp ;
{
	int	rs ;


	if (fsp == NULL)
	    return SR_FAULT ;

	if (fsp->magic != FTPSESSION_MAGIC)
	    return SR_NOTOPEN ;

	rs = FtpBinary(fsp->sp) ;

	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (ftpsession_binary) */


/* set ASCII mode */
int ftpsession_ascii(fsp)
FTPSESSION	*fsp ;
{
	int	rs ;


	if (fsp == NULL)
	    return SR_FAULT ;

	if (fsp->magic != FTPSESSION_MAGIC)
	    return SR_NOTOPEN ;

	rs = FtpAscii(fsp->sp) ;

	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (ftpsession_ascii) */


/* remove a remote file */
int ftpsession_rm(fsp,filename)
FTPSESSION	*fsp ;
char		filename[] ;
{
	int	rs ;


	if (fsp == NULL)
	    return SR_FAULT ;

	if (fsp->magic != FTPSESSION_MAGIC)
	    return SR_NOTOPEN ;

	if ((filename == NULL) || (filename[0] == '\0'))
		return SR_INVALID ;

	rs = FtpRm(fsp->sp,filename) ;

	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (ftpsession_setmode) */


/* return the level of ftpsession available */
int ftpsession_fileread(fsp,filename)
FTPSESSION	*fsp ;
char		filename[] ;
{
	int	rs, len ;


	if (fsp == NULL)
	    return SR_FAULT ;

	if (fsp->magic != FTPSESSION_MAGIC)
	    return SR_NOTOPEN ;

	if ((filename == NULL) || (filename[0] == '\0'))
	    return SR_INVALID ;

	rs = FtpOpenRead(fsp->sp,filename) ;

	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (ftpsession_level) */


/* read some data butt */
int ftpsession_read(fsp,buf,buflen)
FTPSESSION	*fsp ;
char		buf[] ;
int		buflen ;
{
	int	rs ;


	if (fsp == NULL)
	    return SR_FAULT ;

	if (fsp->magic != FTPSESSION_MAGIC)
	    return SR_NOTOPEN ;

	if (buf == NULL)
	    return SR_FAULT ;

	rs = FtpReadBlock(fsp->sp,buf,buflen) ;

	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (ftpsession_read) */


/* write some data butt to the file */
int ftpsession_write(fsp,buf,buflen)
FTPSESSION	*fsp ;
char	buf[] ;
int	buflen ;
{
	int	rs ;


	if (fsp == NULL)
	    return SR_FAULT ;

	if (fsp->magic != FTPSESSION_MAGIC)
	    return SR_NOTOPEN ;

	if (buf == NULL)
	    return SR_FAULT ;

	rs = FtpWriteBlock(fsp->sp,buf,buflen) ;

	if (rs < 0)
	    rs = (- errno) ;

	return rs ;
}
/* end subroutine (ftpsession_write) */



