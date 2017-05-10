/* mailenter */

/* enter the mail message into the mail stream */
/* version %I% last modified %G% */


#define	CF_DEBUG	1		/* run-time debug print-outs */


/* revision history:

	= 1997-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1997 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine is responsible for calling the mail processing
	program '/usr/lib/sendmail' to enter a mail message into
	the mail stream.

	There are (currently) three ways to send data to the mailer
	program.  These are:

	+ PIPE
	+ FILE
	+ SEEK

	The SEEK method can ONLY be used when the whole file is to be
	read by the mailer program.

	Notes:  

	Remember that the 'pipe(2)' system call creates two pipe file
	descriptors.  Both of these file descriptors are open for reading
	and writing on System V UNIX.  On BSD systems, or older BSD
	systems assuming that they have not yet upgraded to the System V
	behavior, the first file descriptor, the one in the zeroth array
	element, is open for reading.  The second file descriptor, the
	one in the oneth array element, is open for writing.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"address.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	u_rewind(fd)	u_seek((fd),0L,SEEK_SET)

#define	O_FLAGS		(O_WRONLY | O_CREAT | O_TRUNC)
#define	CLOSEDFDS	3
#define	NFORKS		3
#define	W_OPTIONS	0
#define	MAXOUTLEN	62



/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;

extern char	*strbasename(char *) ;
extern char	*strwcpy(char *,const char *,int) ;

#if	CF_DEBUG
extern char	*d_procmode() ;
#endif


/* externals variables */


/* local structures */


/* forward references */

static int	writeout(struct proginfo *,int,const char *) ;


/* local variables */


/* export subroutines */


int mailenter(gp,pp,nr,mode,offset,mlen,ifp,ifd, ofd, efd)
struct proginfo		*gp ;
struct prog_params	*pp ;
int		nr, mode ;
offset_t		offset ;
int		mlen ;
bfile		*ifp ;
int		ifd, ofd, efd ;
{
	VECSTR	args ;

	pid_t	pid ;

	int	rs ;
	int	i ;
	int	sl ;
	int	pfd[2], child_stat ;
	int	len ;
	int	tlen = 0 ;

	const char	*program, *arg0 ;
	const char	*cp ;

	char	buf[BUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (gp->debuglevel >= 4) {
	        debugprintf("mailenter: pp=%p\n",pp) ;
	    debugprintf("mailenter: entered, mode=(%s) offset=%lld mlen=%d\n",
	        d_procmode(mode,timebuf,TIMEBUFLEN),offset,mlen) ;
	    debugprintf("mailenter: entered, ofd=%d efd=%d\n",
	        ofd,efd) ;
	}
#endif /* CF_DEBUG */

	rs = vecstr_start(&args,10,VECSTR_PORDERED) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: vecstr init'ed\n") ;
#endif

/* prepare the program to execute */

	program = gp->prog_sendmail ;
	sl = strlen(program) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: ls=%d program=%s\n",sl,program) ;
#endif

	while ((sl > 0) && (program[sl - 1] == '/')) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("mailenter: inside sl=%d\n",sl) ;
#endif

	    sl -= 1 ;

	} /* end while */

	sfbasename(program,sl,&arg0) ;

#if	CF_DEBUG && 0
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: program=%s arg0=%s\n",program,arg0) ;
#endif

	vecstr_add(&args,arg0,-1) ;

#if	CF_DEBUG && 0
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: program=%s arg0=%s\n",program,arg0) ;
#endif

/* set some options for stupid SENDMAIL */

	vecstr_add(&args,"-o",2) ;

	vecstr_add(&args,"i",1) ;

/* the following is VEY bad !  stupid SENDMAIL will drop SUID root ! */

#ifdef	COMMENT
	vecstr_add(&args,"-o",2) ;

	vecstr_add(&args,"EightBitMode=p",1) ;
#endif /* COMMENT */

/* set the SENDMAIL "protocol" option */

	if ((pp->protocol != NULL) && (pp->transport_host != NULL)) {

	    vecstr_add(&args,"-p",2) ;

#if	CF_DEBUG
	    if (gp->debuglevel >= 4) {
	        debugprintf("mailenter: extra SENDMAIL opts, pp=%p\n",pp) ;
	        debugprintf("mailenter: protocol=%s transhost=%s\n",
			pp->protocol,pp->transport_host) ;
		}
#endif

	    sl = bufprintf(buf,BUFLEN,"%s:%s",
	        pp->protocol,pp->transport_host) ;

	    vecstr_add(&args,buf,sl) ;

	} /* end if (extra options for SENDMAIL) */

/* set the envelope sender address if we are directed to */

	if (gp->f.sender) {

	    if ((pp->envelope_from != NULL) && 
	        (pp->envelope_from[0] != '\0')) {

	        vecstr_add(&args,"-f",2) ;

	        vecstr_add(&args,pp->envelope_from,-1) ;

	    }

	} /* end if (sender address allowed on envelope) */

/* pop the recipients down there ! (Ooooh, them !) */

	for (i = 0 ; i < nr ; i += 1) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("mailenter: recipient=%s\n",pp->rp[i].a) ;
#endif

#ifdef	COMMENT
	    if (pp->rp[i].type == ADDRESSTYPE_LOCAL) {

	        sl = -1 ;
	        if ((pp->transport_host != NULL) && 
	            (pp->transport_host[0] != '\0')) {

	            sl = bufprintf(buf,BUFLEN,"%s@%s",
	                pp->rp[i].localpart,
	                pp->transport_host) ;

	        } else if ((pp->envelope_host != NULL) && 
	            (pp->envelope_host[0] != '\0')) {

	            sl = bufprintf(buf,BUFLEN,"%s@%s",
	                pp->rp[i].localpart,
	                pp->envelope_host) ;

	        } else
	            sl = strwcpy(buf,pp->rp[i].localpart,LOCALPARTLEN) - buf ;

	    } else
	        sl = addressarpa(buf,BUFLEN,
	            pp->rp[i].hostpart, pp->rp[i].localpart, pp->rp[i].type) ;
#else
	    sl = addressarpa(buf,BUFLEN,
	        pp->rp[i].hostpart, pp->rp[i].localpart, pp->rp[i].type) ;
#endif /* COMMENT */

	    vecstr_add(&args,buf,sl) ;

	} /* end for (recipients) */

#if	CF_DEBUG
	if (gp->debuglevel > 1) {
	    debugprintf("mailenter: arguments\n") ;
	    for (i = 0 ; vecstr_get(&args,i,&cp) >= 0 ; i += 1)
	        debugprintf("mailenter: arg[%d]=%s\n",i,cp) ;
	}
#endif /* CF_DEBUG */

	u_rewind(ofd) ;

	u_rewind(efd) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: mode=%d\n",mode) ;
#endif

	if (mode == IM_PIPE) {
	    u_pipe(pfd) ;

	} else if (mode == IM_SEEK) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("mailenter: SEEK ifd=%d offset=%lld mlen=%d\n",
	            ifd,offset,mlen) ;
#endif

	    tlen = mlen ;
	    u_seek(ifd,offset,SEEK_SET) ;

#if	CF_DEBUG && 0
	    if (gp->debuglevel > 1) {
	        int	rs1, rs2 ;
	        offset_t	offset2 ;
	        rs = u_tell(ifd,&offset2) ;
	        debugprintf("mailenter: 1 SEEK preview, rs=%d offset=%lld\n",
	            rs,offset2) ;
	        while ((len = uc_readline(ifd,buf,BUFLEN)) > 0)
	            debugprintf("mailenter: | %W",buf,len) ;
	        rs1 = u_seek(ifd,offset,SEEK_SET) ;
	        rs2 = u_tell(ifd,&offset2) ;
	        debugprintf("mailenter: rs1=%d rs2=%d offset=%lld\n",
	            rs1,rs2,offset2) ;
	    }
#endif /* CF_DEBUG */

	} else if (mode == IM_FILE) {

	    tlen = mlen ;
	    u_rewind(ifd) ;

	} /* end (modes) */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: about to fork\n") ;
#endif

	for (i = 0 ; (i < NFORKS) && ((pid = uc_fork()) < 0) ; i += 1) {
	    rs = pid ;

	    if (i == 0)
	        logfile_printf(&gp->lh,
	            "trouble forking, rs=%d\n",rs) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("mailenter: trouble forking\n") ;
#endif

	    sleep(2) ;

	} /* end for */

	if (pid < 0) {

	    rs = pid ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("mailenter: bad fork ! rs=%d\n",rs) ;
#endif

	    logfile_printf(&gp->lh,
	        "cannot fork for service daemon, rs=%d\n",rs) ;

	    goto badret3 ;

	} else if (pid == 0) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("mailenter: child here\n") ;
#endif

	    if ((gp->workdname != NULL) && (gp->workdname[0] != '\0') &&
	        (strcmp(gp->workdname,".") != 0))
	        u_chdir(gp->workdname) ;

	    for (i = 0 ; i < CLOSEDFDS ; i += 1)
	        u_close(i) ;

	    if (mode == IM_PIPE) {

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("mailenter: child PIPE\n") ;
#endif

	        u_close(pfd[1]) ;

	        ifd = pfd[0] ;
	    }

	    rs = u_dup(ifd) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("mailenter: child STDIN_FD=%d\n",rs) ;
#endif

	    u_close(ifd) ;

#if	CF_DEBUG && 0
	    if (gp->debuglevel > 1) {
	        offset_t	offset2 ;
	        rs = u_tell(0,&offset2) ;
	        debugprintf("mailenter: 2 SEEK preview, rs=%d offset=%lld\n",
	            rs,offset2) ;
	        while ((len = uc_readline(0,buf,BUFLEN)) > 0)
	            debugprintf("mailenter: | %W",buf,len) ;
	        u_seek(0,offset,SEEK_SET) ;
	        rs = u_tell(0,&offset2) ;
	        debugprintf("mailenter: rs=%d offset=%lld\n",
	            rs,offset2) ;
	    }
#endif /* CF_DEBUG */

	    u_dup(ofd) ;

	    u_close(ofd) ;

	    u_dup(efd) ;

	    u_close(efd) ;

	    rs = u_execvp(program,args.va) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("mailenter: bad exec rs=%d\n",rs) ;
#endif

	    logfile_printf(&gp->lh,"program execution failed (%d)\n",
	        rs) ;

	    uc_exit(EX_NOEXEC) ;

	} /* end if (child) */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: child PID=%d\n",pid) ;
#endif

#ifdef	COMMENT
	logfile_printf(&g.lh,"starting server=%s pid=%d\n",
	    arg0,pid) ;
#endif

	if (mode == IM_PIPE) {

	    u_close(pfd[0]) ;

	    if (mlen < 0) {

	        tlen = 0 ;
	        rs = SR_OK ;
	        while ((len = bread(ifp,buf,BUFLEN)) > 0) {

	            tlen += len ;
	            if ((rs = uc_writen(pfd[1],buf,len)) < 0)
	                break ;

	        } /* end while (reading input) */

	    } else {

	        tlen = 0 ;
	        rs = SR_OK ;
	        while ((tlen < mlen) && 
	            ((rs = bread(ifp,buf,MIN((mlen - tlen),BUFLEN))) > 0)) {

		    len = rs ;
	            tlen += len ;
	            rs = uc_writen(pfd[1],buf,len) ;
		    if (rs < 0)
	                break ;

	        } /* end while (reading input) */

	        if (rs < 0) {

	            while ((tlen < mlen) && 
	                ((len = bread(ifp,buf,MIN((mlen - tlen),BUFLEN))) > 0))
	                tlen += len ;

	        }

	    } /* end if */

	    u_close(pfd[1]) ;

	} /* end if (mode PIPE) */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: waiting\n") ;
#endif

	u_waitpid(pid,&child_stat,W_OPTIONS) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: capturing outputs, ofd=%d efd=%d\n",
	        ofd,efd) ;
#endif

	writeout(gp,ofd,"standard output\n") ;

	writeout(gp,efd,"standard error \n") ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("mailenter: ret rs=%d tlen=%d\n",rs,tlen) ;
#endif

ret1:
	vecstr_finish(&args) ;

ret0:
	return (rs >= 0) ? tlen : rs ;

/* handle bad returns */
badret3:
	if (mode = IM_PIPE) {

	    if (mlen >= 0) {

	        tlen = 0 ;
	        while ((tlen < mlen) && 
	            ((len = bread(ifp,buf,MIN((mlen - tlen),BUFLEN))) > 0))
	            tlen += len ;

	    } else
	        while (bread(ifp,buf,BUFLEN) > 0) ;

	} /* end if (mode PIPE) */

badret2:
bad1:
	goto ret1 ;
}
/* end subroutine (mailenter) */


/* local subroutines */


/* write out the output files from the executed program */
static int writeout(gp,fd,s)
struct proginfo	*gp ;
int		fd ;
const char	s[] ;
{
	struct ustat	sb ;

	bfile	file, *fp = &file ;

	int	rs = SR_OK ;
	int	len ;
	int	tlen = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;


	if ((u_fstat(fd,&sb) >= 0) && (sb.st_size > 0)) {

	    u_rewind(fd) ;

	    logfile_printf(&gp->lh,s) ;

	    if (bopen(fp,(char *) fd,"dr",0666) >= 0) {

	        while ((rs = breadline(fp,linebuf,LINEBUFLEN)) > 0) {

		    len = rs ;
	            tlen += len ;
	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&gp->lh,"| %W\n",
	                linebuf,MIN(len,MAXOUTLEN)) ;

	        } /* end while (reading lines) */

	        bclose(fp) ;

	    } /* end if (opening file) */

	} /* end if (non-zero file size) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (writeout) */



