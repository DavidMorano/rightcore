/* mailboxappend */

/* append some data (presumably a mail-message) to the end of a mailbox */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-01-20, David A­D­ Morano
	This is new code to replace some previous miscellaneous code used for
	the same purpose.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine appends some data to the end of a mailbox file.

	Synopsis:

	int mailboxappend(mbfname,sfd,slen,to)
	const char	mbfname[] ;
	int		sfd ;
	int		slen ;
	int		to ;

	Arguments:

	- mbfname	file-name of mailbox
	- sfd		source file-descriptor
	- slen		source length to write (starting from current offset)
	- to		timeout waiting for mailbox lock

	Returns:

	>=0		amount written
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<localmisc.h>


/* local defines */

#ifndef	TO_LOCK
#define	TO_LOCK		4
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	lockend(int,int,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	mailboxappender(cchar *,int,int,int) ;
static int	mblock(int,int,int) ;


/* local variables */


/* exported subroutines */


int mailboxappend(cchar *mbfname,int sfd,int slen,int to)
{
	const int	nrs = SR_ACCESS ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if (mbfname == NULL) return SR_FAULT ;

	if (mbfname[0] == '\0') return SR_INVALID ;

	if (sfd < 0) return SR_BADF ;

#if	CF_DEBUGS
	debugprintf("mailboxappend: slen=%d to=%d\n",slen,to) ;
#endif

#if	CF_DEBUGS
	{
	    struct ustat	sb ;
	    int	rs1 ;
	    rs1 = u_fstat(sfd,&sb) ;
	    debugprintf("mailboxappend: u_fstat() rs=%d size=%lu\n",
	        rs1,sb.st_size) ;
	}
#endif /* CF_DEBUGS */

	if ((rs = mailboxappender(mbfname,sfd,slen,to)) == nrs) {
	    if ((rs = u_access(mbfname,W_OK)) >= 0) {
	        const uid_t	uid = getuid() ;
	        const uid_t	euid = geteuid() ;
	        if (uid != euid) {
		    if ((rs = u_seteuid(uid)) >= 0) {
		        rs = mailboxappender(mbfname,sfd,slen,to) ;
		        wlen = rs ;
		        rs1 = u_seteuid(euid) ;
		        if (rs >= 0) rs = rs1 ;
		    } else
		        rs = nrs ;
	        } else
		    rs = nrs ;
	    } /* end if (u_access) */
	} else
	    wlen = rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mailboxappend) */


/* local subroutines */


static int mailboxappender(cchar *mbfname,int sfd,int slen,int to)
{
	const mode_t	om = 0666 ;
	const int	of = (O_CREAT|O_WRONLY|O_APPEND) ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = uc_open(mbfname,of,om)) >= 0) {
	    int		tfd = rs ;
	    if ((rs = mblock(tfd,TRUE,to)) >= 0) {
		SIGBLOCK	blocker ;
		offset_t	offend = 0 ;
		int		cmd ;

	        u_seeko(tfd,0L,SEEK_END,&offend) ;

#if	CF_DEBUGS
	        debugprintf("mailboxappend: seek-lock rs=%d offend=%lld\n",
	            rs,offend) ;
#endif

	        if ((rs = sigblock_start(&blocker,NULL)) >= 0) {

	            rs = uc_writedesc(tfd,sfd,slen) ;
	            wlen = rs ;

#if	CF_DEBUGS
	            debugprintf("mailboxappend: uc_writedesc() rs=%d\n",rs) ;
#endif

	            if (rs < 0)
	                uc_ftruncate(tfd,offend) ;

	            sigblock_finish(&blocker) ;
	        } /* end block (sigblock) */

	        cmd = F_UNLOCK ;
	        rs1 = lockfile(tfd,cmd,offend,0L,0) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (lock) */
	    rs1 = u_close(tfd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (open) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mailboxappender) */


/* Mail-Box Lock */
static int mblock(fd,f_lock,to)
int		fd ;
int		f_lock ;
int		to ;
{
	struct flock	fl ;
	int		rs = SR_OK ;
	int		i ;

	memset(&fl,0,sizeof(struct flock)) ;

	fl.l_type = (f_lock) ? F_WRLCK : F_UNLCK ;
	fl.l_whence = SEEK_END ;
	fl.l_start = 0L ;
	fl.l_len = 0 ;

	if (to < 0)
	    to = TO_LOCK ;

	for (i = 0 ; i < (to + 1) ; i += 1) {

	    rs = u_fcntl(fd,F_SETLK,&fl) ;

	    if ((rs != SR_AGAIN) && (rs != SR_ACCES))
	        break ;

	    if (i < to)
	        uc_safesleep(1) ;

	} /* end for */

	return rs ;
}
/* end subroutine (mblock) */


