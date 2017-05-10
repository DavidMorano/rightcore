/* bopenrcmd */

/* BASIC INPUT OUTPUT package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DANGERHACK	1


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine spawns a shell and gives it a command to
	execute.  Input and output is connected back to the caller.


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/param.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* forward reference */


/* exported subroutines */


int bopenrcmd(fpa,remotehost,cmd)
bfile	*fpa[3] ;
char	remotehost[] ;
char	cmd[] ;
{
	pid_t	child_pid ;

	int	rs ;
	int	fd ;
	int	i, j, k ;
	int	pipes[3][2] ;

	char	pfname[MAXPATHLEN + 1] ;
	char	*cmd_rsh ;


#if	CF_DEBUGS
	debugprintf("bopencmd: entered\n") ;
#endif

	if (fpa == NULL)
		return SR_FAULT ;

/* check for bad input */

	if ((remotehost == NULL) || (remotehost[0] == '\0'))
	    return SR_INVALID ;

	if ((cmd == NULL) || (cmd[0] == '\0'))
	    return SR_INVALID ;

	if ((strlen(cmd) + 6) > LINEBUFLEN)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bopencmd: got in\n") ;
#endif

/* where is the RSH program */

	cmd_rsh = "/bin/rsh" ;
	if (u_access(cmd_rsh,X_OK) != 0) {

	    cmd_rsh = "/usr/ucb/rsh" ;
	    if (u_access(cmd_rsh,X_OK) != 0)
	        return SR_BAD ;

	} /* end if */

/* open up the necessary pipes */

	for (i = 0 ; i < 3 ; i += 1)
	    if (fpa[i] != NULL) {

/* file pointer already used? (yes this is a dangerous hack) */

#if	CF_DANGERHACK
	        if (fpa[i]->magic == BFILE_MAGIC) return SR_OPEN ;
#endif

#if	CF_DEBUGS
	        debugprintf("bopencmd: pipes for %d r=%08X w=%08X ?=%08X\n",
	            i,&pipes[i][0],
	            &pipes[i][1],pipes[i]) ;
#endif

	        if ((rs = u_pipe(pipes[i])) < 0)
	            goto badpipe ;

#if	CF_DEBUGS
	        debugprintf("bopencmd: got pipes for %d r=%d w=%d\n",
			i,pipes[i][0], pipes[i][1]) ;
#endif

	    } /* end if */

/* we fork RSH */

#if	CF_DEBUGS
	debugprintf("bopencmd: about to fork\n") ;
#endif

	if ((child_pid = uc_fork()) == 0) {

#if	CF_DEBUGS
	    debugprintf("bopencmd: inside fork\n") ;
#endif

	    for (i = 0 ; i < 3 ; i += 1) {

	        if (fpa[i] != NULL) {

	            u_close(i) ;

	            u_close(pipes[i][(i == 0) ? 1 : 0]) ;

	            fd = pipes[i][(i == 0) ? 0 : 1] ;
	            u_dup(fd) ;

	            u_close(fd) ;

	        } /* end if */

	    } /* end for */

/* do the exec */

	    execlp(cmd_rsh,"rsh",remotehost,cmd,NULL) ;

	    uc_exit(BAD) ;

	} else if (child_pid < 0) {

	    rs = child_pid ;
	    goto badfork ;

	}

#if	CF_DEBUGS
	debugprintf("bopencmd: main line continue\n") ;
#endif

/* close some pipe ends */

	for (i = 0 ; i < 3 ; i += 1)
	    if (fpa[i] != NULL)
	        u_close(pipes[i][(i == 0) ? 0 : 1]) ;

/* open the parent side "basic" files */

	for (i = 0 ; i < 3 ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("bopencmd: checking to open BIO file %d\n",i) ;
#endif

	    if (fpa[i] != NULL) {

#if	CF_DEBUGS
	        debugprintf("bopencmd: found a non-NULL one FPA[%d]=%08X\n",
			i,fpa[i]) ;
#endif

	        if ((rs = bopen(fpa[i],(char *) pipes[i][(i == 0) ? 1 : 0],
	            (i == 0) ? "dw" : "dr",0666)) < 0)
	            goto badbopen ;

#if	CF_DEBUGS
	        debugprintf("bopencmd: opened successfully %d\n",i) ;
#endif

#if	BFILE_DUP
	        u_close(pipes[i][(i == 0) ? 1 : 0]) ;
#endif

	    } /* end if */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("bopencmd: 'bopen'ed the proper stuff and exiting\n") ;
#endif

	return ((int) child_pid) ;

/* handle the numberous possible errors */
badbopen:

#if	CF_DEBUGS
	debugprintf("bopencmd: at bad open\n") ;
#endif

#if	BFILE_DUP
	u_close(pipes[i][(i == 0) ? 1 : 0]) ;
#endif

	for (j = 0 ; j < i ; j += 1)
	    if (fpa[j] != NULL)
	        bclose(fpa[j]) ;

	for (j = i ; j < 3 ; j += 1)
	    if (fpa[j] != NULL)
	        u_close(pipes[j][(j == 0) ? 1 : 0]) ;

	goto err3 ;

badfork:
err3:
	u_unlink(pfname) ;

badpipe:
err4:
	for (j = 0 ; j < i ; j += 1)
	    for (k = 0 ; k < 2 ; k += 1)
	        u_close(pipes[j][k]) ;

	return rs ;
}
/* end subroutine (bopenrcmd) */



