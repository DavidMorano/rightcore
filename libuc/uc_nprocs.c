/* uc_nprocs */

/* get the number of processes on the system */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1997-11-18, David A­D­ Morano
        This little subroutine was put together to get the current number of
        processes executing on the system.

	= 2019-01-14, David A.D. Morano
	Enhanced (after all this time) to ignore special entries (now common on
	newer OSes).

*/

/* Copyright © 1998,2019 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We basically cheat and count the number of "files" in '/proc' that start
	with a leading digit character. The filename may have to return a good
	|stat(2)| also. Of course, the whole file-system might be unmounted
	also, in which case we return with failure.

	Synopsis:

	int uc_nprocs(int w)

	Arguments:

	w		which processes to count:
				0=all
				1=system-only
				2=current-user
				3=current-session

	Returns:

	<0		error
	>=0		number of processes on the machine


	Notes:

	Amazingly, the '/proc' file-system with file entries in it that look
	like process IDs is probably the most portable way invented so far to
	enumerate process IDs on the system. I think we can all thank AT&T for 
	this, for inventing the PROC FS back in the mid-1980s or so. It is 
	amazing how much stuff that AT&T invented in the 1980s for UNIX that has 
	passed the test of time. The only real problem with the PROC FS is that
	it is possible that it can be unmounted (not mounted), in which case
	everyone has nothing!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<localmisc.h>		/* |MKCHAR(3dam)| and other stuff */


/* local defines */

#ifndef	PROCDNAME
#define	PROCDNAME	"/proc"
#endif

#ifndef	UIDSYS
#define	UIDSYS		100		/* common but unofficial value */
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_nprocs(int w)
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;
	const char	*procdname = PROCDNAME ;

#if	CF_DEBUGS
	debugprintf("uc_nprocs: ent\n") ;
#endif

	if ((rs = fsdir_open(&d,procdname)) >= 0) {
	    int		plen = MAXPATHLEN ;
	    int		ch ;
	    char	*pbuf ;
	    switch (w) {
	    case 0: /* all processes */
	        {
	            while ((rs = fsdir_read(&d,&de)) > 0) {
			ch = MKCHAR(de.name[0]) ;
	                if (isdigitlatin(ch)) {
	                    n += 1 ;
	                }
	            } /* end while */
	        } /* end block */
	        break ;
	    case 1: /* system processes */
	        if ((rs = uc_libmalloc((plen+1),&pbuf)) >= 0) {
	            if ((rs = mkpath1(pbuf,procdname)) >= 0) {
	                struct ustat	sb ;
	                plen = rs ;
	                while ((rs = fsdir_read(&d,&de)) > 0) {
			    ch = MKCHAR(de.name[0]) ;
	                    if (isdigitlatin(ch)) {
	                        if ((rs = pathadd(pbuf,plen,de.name)) >= 0) {
	                            if (u_stat(pbuf,&sb) >= 0) {
	                                if (sb.st_uid < UIDSYS) n += 1 ;
	                            }
	                        }
	                    }
			    if (rs < 0) break ;
	                } /* end while */
	            } /* end if (mkpath) */
	            uc_libfree(pbuf) ;
	        } /* end if (m-a) */
	        break ;
	    case 2: /* user processes */
	        if ((rs = uc_libmalloc((plen+1),&pbuf)) >= 0) {
	            if ((rs = mkpath1(pbuf,procdname)) >= 0) {
	                struct ustat	sb ;
	                const uid_t	uid = getuid() ;
	                plen = rs ;
	                while ((rs = fsdir_read(&d,&de)) > 0) {
			    ch = MKCHAR(de.name[0]) ;
	                    if (isdigitlatin(ch)) {
	                        if ((rs = pathadd(pbuf,plen,de.name)) >= 0) {
	                            if (u_stat(pbuf,&sb) >= 0) {
	                                if (sb.st_uid == uid) n += 1 ;
	                            }
	                        }
	                    }
			    if (rs < 0) break ;
	                } /* end while */
	            } /* end if (mkpath) */
	            uc_libfree(pbuf) ;
	        } /* end if (m-a) */
	        break ;
	    case 3: /* session processes */
	        {
	            const pid_t	sid = getsid(0) ;
	            pid_t	cid, csid ;
	            int		v ;
	            while ((rs = fsdir_read(&d,&de)) > 0) {
			ch = MKCHAR(de.name[0]) ;
	                if (isdigitlatin(ch)) {
	                    if (cfdeci(de.name,rs,&v) >= 0) {
	                        cid = v ;
	                        if (cid > 0) {
	                            if ((csid = getsid(cid)) >= 0) {
	                                if (csid == sid) n += 1 ;
	                            }
	                        }
	                    }
	                }
	            } /* end while */
	        } /* end block */
	        break ;
	    default:
	        rs = SR_NOSYS ;
	        break ;
	    } /* end switch */
	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */

#if	CF_DEBUGS
	debugprintf("uc_nprocs: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (uc_nprocs) */


