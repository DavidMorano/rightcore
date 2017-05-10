/* process */

/* process a name */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module just provides optional expansion of directories.
	The real work is done by the 'checkname' module.



******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	cfdecmfui(const char *,int,int *) ;
extern int	cfdecmfull(const char *,int,ULONG *) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	uc_ftruncate(int,offset_t) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* external variables */


/* global variables */


/* local variables */


/* exported subroutines */


int process(pip,namespec,trunclen)
struct proginfo	*pip ;
const char	namespec[] ;
ULONG		trunclen ;
{
	struct ustat	sb ;

	ULONG	flen ;

	int	rs = SR_OK ;
	int	fd ;
	int	f_flen ;

	const char	*fname ;
	const char	*cp ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if (namespec == NULL)
	    return SR_FAULT ;

	if (namespec[0] == '\0')
	    return SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: namespec=\"%s\"\n",namespec) ;
#endif

	flen = trunclen ;
	f_flen = pip->f.trunclen ;

	if (! pip->f.literal) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: not literal\n") ;
#endif

	    fname = namespec ;
	    if ((cp = strchr(namespec,'=')) != NULL) {

	        fname = tmpfname ;
	        strwcpy(tmpfname,namespec,(cp - namespec)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: fname=%s\n",fname) ;
#endif

	        rs = cfdecmfull((cp + 1),-1,&flen) ;
	        f_flen = (rs >= 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: f_flen=%d flen=%llu\n",f_flen,flen) ;
#endif

	    }

	} else
	    fname = namespec ;

/* do it */

	fd = FD_STDIN ;
	if (strcmp(fname,"-") != 0) {

		rs = u_open(fname,O_WRONLY,0666) ;
		fd = rs ;

	} else
		rs = fperm(fd,-1,-1,NULL,W_OK) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: u_open()/fperm() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* continue */

	rs = u_fstat(fd,&sb) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("process: u_fstat() rs=%d\n",rs) ;
	    debugprintf("process: isreg=%d\n",
			S_ISREG(sb.st_mode)) ;
	}
#endif

	if ((rs < 0) || (! S_ISREG(sb.st_mode)))
	    goto done ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: filesize=%lld flen=%llu\n",
		sb.st_size,flen) ;
#endif

	if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: len=%llu file=%s\n",
			pip->progname,flen,fname) ;

	if (f_flen && (sb.st_size > flen)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: truncating to len=%llu\n",flen) ;
#endif

	    rs = uc_ftruncate(fd,(offset_t) flen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process: uc_ftruncate() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) rs = 1 ;

	} /* end if (file needed truncating) */

done:
	u_close(fd) ;

ret0:
	return rs ;
}
/* end subroutine (process) */



