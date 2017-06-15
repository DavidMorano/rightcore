/* getserial */

/* get the serial number for logging references */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-1, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to get a unique serial number from a specified
        file. These numbes are used for sequencing and other purposes in general
        code. An attempt is made to lock the SERIAL file and if the lock fails,
        the subroutine returns an error (negative number).

        Locking may indeed fail due to the very poorly written file locking code
        on the old SunOS 4.xxx version of the UNIX system. Remote file locking
        over NFS on the old SunOS 4.xxx systems **never** worked correctly!
        Other errors, like "couldn't create the file" are reported as such.

	Synopsis:

	int getserial(sfname)
	const char	sfname[] ;

	Arguments:

	sfname		sfname of file containing the serial number

	Returns:

	>0		the serial number
	==0		file was just created
	<0		could not get it!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	DEFSERIAL	"/tmp/serial"
#define	FILEMODE	0666

#define	TO_LOCK		4		/* seconds */

#if	defined(IRIX)
#ifndef	F_ULOCK
#define F_ULOCK	0	/* Unlock a previously locked region */
#endif
#ifndef	F_LOCK
#define F_LOCK	1	/* Lock a region for exclusive use */
#endif
#ifndef	F_TLOCK
#define F_TLOCK	2	/* Test and lock a region for exclusive use */
#endif
#ifndef	F_TEST
#define F_TEST	3	/* Test a region for other processes locks */
#endif
#endif /* defined(IRIX) */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

extern int	mkpath1w(char *,cchar *,int) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	cfnumui(const char *,int,int *) ;
extern int	ctdeci(char *,int,uint) ;
extern int	ctdecui(char *,int,uint) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	isNotPresent(int) ;
extern int	isNotValid(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	getserial_open(cchar *) ;
static int	getserial_read(int,char *,int) ;
static int	getserial_write(int,char *,int,int) ;


/* exported subroutines */


int getserial(cchar *sfname)
{
	int		rs ;
	int		rs1 ;
	int		serial = 0 ;

#if	CF_DEBUGS
	debugprintf("getserial: ent sfn=%s\n",sfname) ;
#endif

	if ((sfname == NULL) || (sfname[0] == '\0'))
	    sfname = DEFSERIAL ;

	if ((rs = getserial_open(sfname)) >= 0) {
	    const int	fd = rs ;
	    if ((rs = lockfile(fd,F_LOCK,0L,0L,TO_LOCK)) >= 0) {
		const int	dlen = DIGBUFLEN ;
		char		dbuf[DIGBUFLEN+1] ;
	        if ((rs = getserial_read(fd,dbuf,dlen)) >= 0) {
	            serial = rs ; /* result for return */
	            rs = getserial_write(fd,dbuf,dlen,serial) ;
	        }
	    } /* end if (lockfile) */
	    rs1 = u_close(fd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (opened successfully) */

#if	CF_DEBUGS
	debugprintf("getserial: ret rs=%d serial=%u\n",rs,serial) ;
#endif

	return (rs >= 0) ? serial : rs ;
}
/* end subroutine (getserial) */


/* local subroutines */


static int getserial_open(cchar *sfname)
{
	const mode_t	m = FILEMODE ;
	int		rs ;
	int		fd = -1 ;

	rs = uc_open(sfname,O_RDWR,m) ;
	fd = rs ;

#if	CF_DEBUGS
	debugprintf("getserial: u_open() rs=%d\n",rs) ;
#endif

	if (rs == SR_ACCESS) {
	    if ((rs = uc_unlink(sfname)) >= 0) {
	        rs = SR_NOENT ;
	    }
	}
	if (rs == SR_NOENT) {
	    const int	of = (O_RDWR|O_CREAT) ;
	    if ((rs = uc_open(sfname,of,m)) >= 0) {
	        fd = rs ;
	        if ((rs = uc_fminmod(fd,m)) >= 0) {
	            const int	n = _PC_CHOWN_RESTRICTED ;
	            if ((rs = u_fpathconf(fd,n,NULL)) == 0) {
	                USTAT	sb ;
	                int	cl ;
	                cchar	*cp ;
	                if ((cl = sfdirname(sfname,-1,&cp)) > 0) {
			    const int	plen = MAXPATHLEN ;
	                    char	*pbuf ;
			    if ((rs = uc_malloc((plen+1),&pbuf)) >= 0) {
	                        if ((rs = mkpath1w(pbuf,cp,cl)) >= 0) {
	                            if ((rs = uc_stat(pbuf,&sb)) >= 0) {
	                                rs = u_fchown(fd,sb.st_uid,sb.st_gid) ;
	                            }
	                        } /* end if (mkpath) */
				uc_free(pbuf) ;
			    } /* end if (m-a-f) */
	                } /* end if (sfdirname) */
	            } /* end if (u_pathconf) */
	        } /* end if (uc_fminmod) */
	    } /* end if (uc_open) */
	}
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (getserial_open) */


static int getserial_read(int fd,char *dbuf,int dlen)
{
	int		rs ;
	int		serial = 0 ;
	if ((rs = u_read(fd,dbuf,dlen)) > 0) {
	    int		len = rs ;
	    int		cl ;
	    cchar	*cp ;
	    if ((cl = nextfield(dbuf,len,&cp)) > 0) {
	        if ((rs = cfnumui(cp,cl,&serial)) >= 0) {
	            rs = u_rewind(fd) ;
	        } else if (isNotValid(rs)) {
	            serial = 0 ;
	            rs = SR_OK ;
	        }
	    } /* end if (nextfield) */
	} /* end if (u_read) */
	return (rs >= 0) ? serial : rs ;
}
/* end subroutine (getserial_read) */


static int getserial_write(int fd,char *dbuf,int dlen,int serial)
{
	const int	nserial = ((serial+1) & INT_MAX) ;
	int		rs ;
	    if ((rs = ctdeci(dbuf,dlen,nserial)) >= 0) {
	        dbuf[rs++] = '\n' ;
	        if ((rs = u_write(fd,dbuf,rs)) >= 0) {
	            offset_t	uoff = rs ;
	            rs = uc_ftruncate(fd,uoff) ;
	        }
	    } /* end if (ctdeci) */
	return rs ;
}
/* end subroutine (getserial_write) */


