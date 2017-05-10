/* bopencmd */

/* BASIC INPUT OUTPUT package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_READWRITE	1		/* pipes are O_RDWR */
#define	CF_BFD		0		/* really optional (leave out) */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Notes:  

	Remember that the 'pipe(2)' system call creates two pipe file
	descriptors.  Both of these file descriptors are open for
	reading and writing on System V UNIX.  On BSD systems, or older
	BSD systems assuming that they have not yet upgraded to the
	System V behavior, the first file descriptor, the one in the
	zeroth array element, is open for reading.  The second file
	descriptor, the one in the oneth array element, is open for
	writing.


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	LINELEN		4096
#define	CMDBUFLEN	MAXPATHLEN
#define	RESERVEDFDS	3

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	VARSHELL
#define	VARSHELL	"SHELL"
#endif


/* external subroutines */


/* external variables */


/* forward references */


/* exported subroutines */


int bopencmd(fpa,cmd)
bfile		*fpa[3] ;
const char	cmd[] ;
{
	pid_t	pid_child ;

#if	CF_DEBUGS
	struct ustat	sb ;
#endif

	int	rs = SR_OK ;
	int	rs1 ;
	int	size ;
	int	cmdlen ;
	int	i, j, k ;
	int	fd ;
	int	pipes[3][2] ;

#if	CF_BFD
	int	bfd[3] ;
#endif

	char	cmdbuf[CMDBUFLEN + 1] ;
	char	*sp, *cp ;


#if	CF_DEBUGS
	debugprintf("bopencmd: entered\n") ;
#endif

	if (fpa == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	if (u_fstat(2,&sb) >= 0)
	    debugprintf("bopencmd: FD=2 is there!\n") ;
#endif

	for (i = 0 ; i < 3 ; i += 1) {

	    if ((fpa[i] != NULL) && (fpa[i]->magic == BFILE_MAGIC))
	        return SR_OPEN ;

	} /* end for */

	if ((cmd == NULL) || (cmd[0] == '\0'))
	    return SR_INVALID ;

/* clean up the command a little */

	cmdlen = strlen(cmd) ;

	while (CHAR_ISWHITE(*cmd)) {
	    cmd += 1 ;
	    cmdlen -= 1 ;
	}

	while ((cmdlen > 0) && CHAR_ISWHITE(cmd[cmdlen - 1]))
	    cmdlen -= 1 ;

#if	CF_DEBUGS
	debugprintf("bopencmd: got in\n") ;
#endif

/* open up some extra files if necessary */

#if	CF_BFD
	for (i = 0 ; i < 3 ; i += 1) {
	    const char	*nullfname = NULLFNAME ;

	    bfd[i] = -1 ;
	    if (u_fstat(i,&sb) >= 0)
	        continue ;

	    if (i == 0) {
	        rs = u_open(nullfname,RDONLY,0600) ;

	    } else
	        rs = u_open(nullfname,WRONLY,0600) ;

	    if (rs >= 0)
	        bfd[i] = rs ;

	} /* end for */
#endif /* CF_BFD */

/* open up the necessary pipes */

	for (i = 0 ; i < 3 ; i += 1) if (fpa[i] != NULL) {

	    rs = u_pipe(pipes[i]) ;
	    if (rs < 0)
	        goto badpipe ;

#if	CF_DEBUGS
	    debugprintf("bopencmd: got pipes for %d r=%d w=%d\n",
	        i,pipes[i][0], pipes[i][1]) ;
#endif

	    if ((fd = uc_moveup(pipes[i][0],RESERVEDFDS)) < 0)
	        goto badpipe ;

	    pipes[i][0] = fd ;

	    if ((fd = uc_moveup(pipes[i][1],RESERVEDFDS)) < 0)
	        goto badpipe ;

	    pipes[i][1] = fd ;

	} /* end if (making the pipes for the child) */

#if	CF_DEBUGS
	debugprintf("bopencmd: opened the pipes\n") ;
#endif

/* we fork the SHELL */

#if	CF_DEBUGS
	debugprintf("bopencmd: about to fork\n") ;
#endif

	rs = uc_fork() ;
	pid_child = rs ;
	if (rs < 0) goto badfork ;

	if (pid_child == 0) {

#if	CF_DEBUGS
	    debugprintf("bopencmd: inside fork\n") ;
#endif

	    for (i = 0 ; i < 3 ; i += 1) {

	        if (fpa[i] != NULL) {

	            u_close(pipes[i][(i == 0) ? 1 : 0]) ;

	            u_close(i) ;

	            fd = pipes[i][(i == 0) ? 0 : 1] ;
	            u_dup(fd) ;

	            u_close(fd) ;

	        } /* end if */

	    } /* end for */

/* get the user's SHELL command */

	    if ((sp = getenv(VARSHELL)) != NULL) {

	        if (u_access(sp,X_OK) < 0)
	            cp = NULL ;

	    }

	    if (sp == NULL) {

	        sp = "/usr/bin/ksh" ;
	        if (u_access(sp,X_OK) < 0)
	            sp = "/usr/bin/sh" ;

	    }

/* do we modify the command? */

	    cp = (char *) cmd ;
	    if ((cmdlen < 6) || (strncmp(cmd,"exec",4) != 0) || 
	        ((cmd[4] != '\0') && (! CHAR_ISWHITE(cmd[4])))) {

	        cp = cmdbuf ;
	        if (cmdlen > (CMDBUFLEN - 10)) {

		    size = (cmdlen + 1) ;
		    rs1 = uc_malloc(size,&cp) ;

		    if (rs1 < 0)
	                uc_exit(EX_UNAVAILABLE) ;

	        }

	        strcpy(cp,"exec ") ;

	        strcpy(cp + 5,cmd) ;

	    } /* end if (we needed to add an 'exec') */

#if	CF_DEBUGS
	    debugprintf("bopencmd: about to exec to \"%s\"\n",cp) ;
#endif

	    execlp(sp,"shcmd","-c",cp,NULL) ;

	    uc_exit(EX_NOEXEC) ;

	} /* end if (child process) */

#if	CF_DEBUGS
	debugprintf("bopencmd: main line continue\n") ;
#endif

	for (i = 0 ; i < 3 ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("bopencmd: checking to open BIO file %d\n",i) ;
#endif

	    if (fpa[i] != NULL) {

#if	CF_DEBUGS
	        debugprintf("bopencmd: found a non-NULL one FPA[%d]=%08X\n",
	            i,fpa[i]) ;
#endif

/* close some pipe ends that are not needed in the parent */

	        u_close(pipes[i][(i == 0) ? 0 : 1]) ;

/* open the parent side "basic" files */

#if	CF_READWRITE
	        if ((rs = bopen(fpa[i],(char *) pipes[i][(i == 0) ? 1 : 0],
	            "drw", 0666)) < 0)
	            goto badbopen ;
#else
	        if ((rs = bopen(fpa[i],(char *) pipes[i][(i == 0) ? 1 : 0],
	            (i == 0) ? "dw" : "dr",0666)) < 0)
	            goto badbopen ;
#endif /* CF_READWRITE */

#if	CF_DEBUGS
	        debugprintf("bopencmd: opened successfully %d\n",i) ;
#endif

#ifdef	COMMENT
	        if ((i == 1) || (i == 2)) {
	            fpa[i]->bm = bfile_bmline ;
		}
#endif

#if	BFILE_DUP
	        u_close(pipes[i][(i == 0) ? 1 : 0]) ;
#endif

	    } /* end if */

	} /* end for (looping through file pointers) */

done:

#if	CF_BFD
	for (i = 0 ; i < 3 ; i += 1) {
	    if (bfd[i] >= 0)
	        u_close(bfd[i]) ;
	} /* end for */
#endif /* CF_BFD */

ret0:

#if	CF_DEBUGS
	debugprintf("bopencmd: ret rs=%d pid=%u\n",rs,pid_child) ;
#endif

	if (rs >= 0) rs = (int) pid_child ;
	return rs ;

/* handle the numerous possible errors */
badbopen:

#if	CF_DEBUGS
	debugprintf("bopencmd: at bad open\n") ;
#endif

#if	BFILE_DUP
	u_close(pipes[i][(i == 0) ? 1 : 0]) ;
#endif

/* close all BIO files below the one that bombed */

	for (j = 0 ; j < i ; j += 1) {
	    if (fpa[j] != NULL)
	        bclose(fpa[j]) ;
	}

/* close all pipes from the one that bombed and above */

	for (j = i ; j < 3 ; j += 1) {
	    if (fpa[j] != NULL)
	        u_close(pipes[j][(j == 0) ? 1 : 0]) ;
	}

	goto err3 ;

badwrite:

badfork:

err3:

/* close all of the pipe FDs below the one that bombed */
badpipe:
err4:
	for (j = 0 ; j < i ; j += 1)
	    for (k = 0 ; k < 2 ; k += 1)
	        u_close(pipes[j][k]) ;

	goto done ;
}
/* end subroutine (bopencmd) */



