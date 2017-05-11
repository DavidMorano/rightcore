/* uc_nprocs */

/* get the number of processes on the system */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1997-11-18, David A­D­ Morano
        This little subroutine was put together to get the current number of
        processes executing on the system.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We basically cheat and count the number of "files" in '/proc', if it
        exists and has a non-zero value (the FS might be unmounted).

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


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<localmisc.h>


/* local defines */

#ifndef	PROCDNAME
#define	PROCDNAME	"/proc"
#endif

#ifndef	UIDSYS
#define	UIDSYS		100
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;


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
	int		n = 0 ;
	const char	*procdname = PROCDNAME ;

#if	CF_DEBUGS
	debugprintf("uc_nprocs: ent\n") ;
#endif

	if ((rs = fsdir_open(&d,procdname)) >= 0) {
	    int		plen = MAXPATHLEN ;
	    char	*pbuf ;
	    switch (w) {
	    case 0: /* all processes */
	        {
	            while ((rs = fsdir_read(&d,&de)) > 0) {
	                if (de.name[0] != '.') {
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
	                    if (de.name[0] != '.') {
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
	                    if (de.name[0] != '.') {
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
	                if (de.name[0] != '.') {
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
	    fsdir_close(&d) ;
	} /* end if (fsdir) */

#if	CF_DEBUGS
	debugprintf("uc_nprocs: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (uc_nprocs) */


