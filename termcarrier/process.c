/* process */

/* process this device */


#define	CF_DEBUG 	1		/* compile-time */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The program was copied very substantially from 'hangup'.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int process(pip,basedir,name)
	PROGINFO	*pip ;
	char		basedir[] ;
	char		name[] ;

	Arguments:

	gp		global data pointer
	basedir		directory at top of tree
	name		device name to pop off !

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<termios.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	"localmisc.h"

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_F1		(O_RDWR | O_NDELAY | O_NOCTTY)
#define	O_F2		(O_RDONLY | O_NDELAY | O_NOCTTY)


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;


/* external variables */


/* local variables */

static cchar	*turnons[] = {
	"1",
	"yes",
	"y",
	"on",
	NULL
} ;






int process(pip,basedir,name)
PROGINFO	*pip ;
char		basedir[] ;
char		name[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	f_softcar ;
	int	f_softcar_on = 1 ;
	int	f_softcar_off = 0 ;
	int	f_tell = FALSE ;

	char	dfname[MAXPATHLEN + 1] ;
	char	*device, *state ;
	char	*bdp, *cp ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("process: entered\n") ;
#endif

	if (name == NULL)
	    return SR_FAULT ;

/* parse out the device string from the switch */

	device = name ;
	state = NULL ;
	if ((cp = strchr(name,'=')) != NULL) {

	    *cp++ = '\0' ;
	    state = cp ;

	} /* end if */

/* form a good device name */

	bdp = device ;
	rs = perm(device,-1,-1,NULL,R_OK) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("process: perm() 1 rs=%d\n",rs) ;
#endif

	if ((rs == SR_NOENT) && (device[0] != '/')) {

	    if ((basedir == NULL) || (basedir[0] == '\0'))
	        basedir = DEVDNAME ;

	    bdp = dfname ;
	    mkpath2(dfname, basedir,device) ;

	    rs = perm(bdp,-1,-1,NULL,R_OK) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("process: perm() 2 rs=%d\n",rs) ;
#endif

	} /* end if */

	if (pip->verboselevel > 0)
	    bprintf(pip->ofp,"device=%s\n",pip->progname,bdp) ;

	    if (rs < 0)
	        bprintf(pip->efp,"%s: could not access=%s (%d)\n",
	            pip->progname,bdp,rs) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("process: perm() F rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if (pip->f.fakeit)
	    goto ret0 ;

/* open the device */

	rs = u_open(bdp,O_F1,0666) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: u_open() 1 rs=%d\n", rs) ;
#endif

	if (rs == SR_ACCESS) {

		rs = u_open(bdp,O_F2,0666) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: u_open() 2 rs=%d\n", rs) ;
#endif

	}

	if (rs >= 0) {

	    struct termios	settings ;

	    int	tfd = rs ;


	    f_softcar = 2 ;
	    rs = u_ioctl(tfd,TIOCGSOFTCAR,&f_softcar) ;

	    if ((rs >= 0) && (state != NULL)) {

	        if (matstr(turnons,state,-1) >= 0) {

	            if (! f_softcar) {

	                if (pip->verboselevel > 0)
	                    bprintf(pip->ofp,"turning soft carrier ON\n") ;

	                rs = u_ioctl(tfd,TIOCGSOFTCAR,&f_softcar_on) ;

	            } else if (pip->verboselevel > 0)
	                f_tell = TRUE ;

	        } else {

	            if (f_softcar) {

	                if (pip->verboselevel > 0)
	                    bprintf(pip->ofp,"turning soft carrier OFF\n") ;

	                rs = u_ioctl(tfd,TIOCGSOFTCAR,&f_softcar_off) ;

	            } else 
	                if (pip->verboselevel > 0)
	                    f_tell = TRUE ;

	        } /* end if */

	    } else
	        f_tell = TRUE ;

	    if (rs < 0)
	        bprintf(pip->efp,"%s: bad IOCTL, rs=%d\n",
	            pip->progname,rs) ;

	    else if (f_tell)
	        bprintf(pip->ofp,"soft carrier for device \"%s\" is %s\n",
	            bdp,((f_softcar) ? "ON" : "OFF")) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: soft carrier for device \"%s\" = %d\n",
	            bdp,f_softcar) ;
#endif

	    u_close(tfd) ;

	} else if (! pip->f.quiet)
	    bprintf(pip->efp,"%s: could not open \"%s\" (%d)\n",
	        pip->progname,bdp,rs) ;

ret0:
	return rs ;
}
/* end subroutine (process) */



