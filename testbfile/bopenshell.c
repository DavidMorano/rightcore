/* bopenshell */

/* BASIC INPUT OUTPUT package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_READWRITE	1		/* pipes are O_RDWR */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens a SHELL and then passes to it the command
	string supplied by the user.

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

#include	<envstandards.h>	/* MUST be first to configure */

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

#define	CMDBUFLEN	(5 * MAXPATHLEN)
#define	RESERVEDFDS	3

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDNAME"
#endif

#ifndef	VARSHELL
#define	VARSHELL	"SHELL"
#endif

#define	PROG_RM		"/usr/bin/rm"
#define	PROG_KSH	"/usr/bin/ksh"
#define	PROG_SH		"/usr/bin/sh"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	bio_mktmpfile(char *,mode_t,char *) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;

extern char	*bio_strbasename(char *) ;
extern char	*strdcpy2(char *,int,const char *,const char *) ;


/* external variables */


/* forward references */


/* exported subroutines */


int bopenshell(fpa,cmd)
bfile		*fpa[3] ;
const char	cmd[] ;
{
	pid_t	child_pid ;

#if	CF_DEBUGS
	struct ustat	sb ;
#endif

	int	rs = SR_OK ;
	int	i, j, k ;
	int	wfd, fd ;
	int	cmdlen ;
	int	pipes[3][2] ;

	char	cmdbuf[CMDBUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;

	const char	*tmpdname = NULL ;
	const char	*sp, *cp ;


#if	CF_DEBUGS
	debugprintf("bopenshell: entered\n") ;
#endif

	if (fpa == NULL)
	    return SR_FAULT ;

	if (cmd == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	if (fstat(2,&sb) >= 0)
	    debugprintf("bopenshell: FD=2 is there!\n") ;
#endif

	if ((cmd == NULL) || (cmd[0] == '\0'))
	    return SR_INVALID ;

	for (i = 0 ; (rs >= 0) && (i < 3) ; i += 1) {

	    if ((fpa[i] != NULL) && (fpa[i]->magic == BFILE_MAGIC))
	        rs = SR_OPEN ;

	} /* end for */
	if (rs < 0) goto ret0 ;

/* clean up the command a little */

	cmdlen = strlen(cmd) ;

	while (CHAR_ISWHITE(*cmd)) {
	    cmd += 1 ;
	    cmdlen -= 1 ;
	}

	while ((cmdlen > 0) && CHAR_ISWHITE(cmd[cmdlen - 1]))
	    cmdlen -= 1 ;

/* does this program exist? */

	cp = cmd ;
	while (*cp && (! CHAR_ISWHITE(*cp)))
	    cp += 1 ;

	rs = mkpath1w(tmpfname,cmd,(cp-cmd)) ;
	if (rs < 0) goto bad0 ;

	rs = findfilepath(NULL,NULL,tmpfname,X_OK) ;
	if (rs < 0) goto bad0 ;

#if	CF_DEBUGS
	debugprintf("bopenshell: got in\n") ;
#endif

/* do we modify the user's command? */

	if ((cmdlen < 6) || (strncmp(cmd,"exec",4) != 0) || 
	    ((cmd[4] != '\0') && (! CHAR_ISWHITE(cmd[4])))) {

	    rs = sncpy2(cmdbuf,CMDBUFLEN,"exec ",cmd) ;
	    cmd = cmdbuf ;

	} /* end if (we needed to add an 'exec') */
	if (rs < 0) goto bad0 ;

/* create the temporary FIFO file for communication */

	if (tmpfname == NULL) tmpdname = getenv(VARTMPDNAME) ;
	if (tmpfname == NULL) tmpdname = TMPDNAME ;

	{
	    const int	oflags = (0600 | S_IFIFO) ;
	    char	template[MAXPATHLEN+1] ;
	    rs = mkpath2(template,tmpdname,"shcmdXXXXXXXXX") ;
	    if (rs >= 0)
		rs = bio_mktmpfile(template,oflags,tmpfname) ;
	}
	if (rs < 0) goto bad0 ;

/* open the FIFO for reading and writing so that we don't hang! */

	rs = u_open(tmpfname,O_RDWR,0666) ;
	if (rs < 0)
	    goto badtmpopen ;

	rs = uc_moveup(rs,RESERVEDFDS) ;
	wfd = rs ;
	if (rs < 0)
	    goto badmove1 ;

/* open up the necessary pipes */

	for (i = 0 ; i < 3 ; i += 1) if (fpa[i] != NULL) {

	    if ((rs = u_pipe(pipes[i])) < 0)
	        goto badpipe ;

#if	CF_DEBUGS
	    debugprintf("bopenshell: got pipes for %d r=%d w=%d\n",
		i,pipes[i][0],
	        pipes[i][1]) ;
#endif

	    if ((fd = uc_moveup(pipes[i][0],RESERVEDFDS)) < 0)
	        goto badpipe ;

	    pipes[i][0] = fd ;

	    if ((fd = uc_moveup(pipes[i][1],RESERVEDFDS)) < 0)
	        goto badpipe ;

	    pipes[i][1] = fd ;

	} /* end if (making the pipes for the child) */

#if	CF_DEBUGS
	debugprintf("bopenshell: opened the pipes\n") ;
#endif

/* we fork the SHELL */

#if	CF_DEBUGS
	debugprintf("bopenshell: about to fork\n") ;
#endif

	rs = uc_fork() ;
	child_pid = rs ;
	if (rs == 0) {

#if	CF_DEBUGS
	    debugprintf("bopenshell: inside fork\n") ;
#endif

/* close the FIFO FD that we will not be using */

	    u_close(wfd) ;

/* close all of the extra pipe FDs that we do not use */

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
	            sp = NULL ;

	    }

	    if (sp == NULL) {

	        sp = PROG_KSH ;
	        if (u_access(sp,X_OK) < 0)
	            sp = PROG_SH ;

	    }

#if	CF_DEBUGS
	    debugprintf("bopenshell: about to exec SHELL=%s\n",sp) ;
#endif

	    rs = execlp(sp,"shcmd",tmpfname,NULL) ;

#if	CF_DEBUGS
	    debugprintf("bopenshell: exec rs=%d\n",rs) ;
#endif

	    uc_exit(EX_NOEXEC) ;

	} else if (rs < 0)
	    goto badfork ;

#if	CF_DEBUGS
	debugprintf("bopenshell: main line continue\n") ;
#endif

/* close all of the pipe FDs that we (the parent) will not be using */

	for (i = 0 ; i < 3 ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("bopenshell: checking to open BIO file %d\n",i) ;
#endif

	    if (fpa[i] != NULL) {

#if	CF_DEBUGS
	        debugprintf("bopenshell: found a non-NULL one FPA[%d]=%08X\n",
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
	        debugprintf("bopenshell: opened successfully %d\n",i) ;
#endif

#ifdef	COMMENT
	        if ((i == 1) || (i == 2))
	            fpa[i]->f.bufline = TRUE ;
#endif

#if	BFILE_DUP
	        u_close(pipes[i][(i == 0) ? 1 : 0]) ;
#endif

	    } /* end if */

	} /* end for (looping through file pointers) */

#if	CF_DEBUGS
	debugprintf("bopenshell: 'bopen'ed the proper stuff and exiting\n") ;
#endif


/* have the child delete the temporary FIFO file */

	cmdlen = bufprintf(cmdbuf,CMDBUFLEN,"%s -f %s",
		PROG_RM,tmpfname) ;

	rs = uc_writen(wfd,cmdbuf,cmdlen) ;
	if (rs < cmdlen) {

	    if (rs >= 0)
	        rs = SR_NOBUFS ;

	    goto badwrite ;
	}

/* write the user's program string to the child SHELL */

	if ((rs = u_write(wfd,cmd,strlen(cmd))) < cmdlen) {

	    if (rs >= 0)
	        rs = SR_NOBUFS ;

	    goto badwrite ;
	}

	u_close(wfd) ;

	rs = child_pid ;

ret0:

#if	CF_DEBUGS
	debugprintf("bopenshell: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* handle the numerous possible errors */
badwrite:
	goto err4 ;

badbopen:

#if	CF_DEBUGS
	debugprintf("bopenshell: at bad open\n") ;
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

err4:
badfork:

/* close all of the pipe FDs below the one that bombed */
badpipe:
	for (j = 0 ; j < i ; j += 1) {
	    for (k = 0 ; k < 2 ; k += 1)
	        u_close(pipes[j][k]) ;
	}

err3:
	u_close(wfd) ;

badmove1:
badtmpopen:
	u_unlink(tmpfname) ;

bad0:
	goto ret0 ;
}
/* end subroutine (bopenshell) */



