/* main */

/* program to read a shared memory mapped file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0


/* revision history:

	= 88/01/10, David A­D­ Morano

	This subroutine was originally written.


	= 01/09/06, David A­D­ Morano

	It always amazes me how many years go by (since 1988 !) before
	I do anything again with some things.  This was taken from the
	TESTMAP2 testing program but it is almost entirely rewritten to
	do more interactive tests of memory mapping and file locking.
	It kills me that I have spent so much of my life coding
	and have nothing to show for it !!


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This subroutine is the major part of a program that tests
	file mapping concepts.


***************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUF
#define	LINELEN		100
#endif

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	cmd_read(struct proginfo *,int,char *,bfile *) ;
static int	cmd_readlock(struct proginfo *,int,char *,bfile *) ;
static int	cmd_readsync(struct proginfo *,int,char *,bfile *) ;
static int	cmd_update(struct proginfo *,int,char *,bfile *) ;

static void		int_alarm() ;
static void		int_signal() ;


/* module variables */

static int		f_alarm ;
static int		f_signal ;


/* local variables */

static char	*const cmds[] = {
	    "read",
	    "readlock",
	    "readsync",
	    "update",
	    NULL
} ;

#define	CMD_READ	0
#define	CMD_READLOCK	1
#define	CMD_READSYNC	2
#define	CMD_UPDATE	3







int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct termios		ots, nts ;

	struct sigaction	sigs ;

	struct proginfo		pi, *pip = &pi ;

	sigset_t		signalmask ;

	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	errfile, *efp = &errfile ;

	time_t	daytime, endtime ;

	int	rs = SR_OK ;
	int	len, sl ;
	int	ex = EX_USAGE ;
	int	i, j ;
	int	fd = -1 ;
	int	f_msync ;
	int	fd_debug ;

	char	*progname ;
	char	linebuf[LINELEN + 1] ;
	char	seqbuf[LINELEN + 1] ;
	char	tmpbuf[BUFLEN + 1] ;
	char	*buf, *bp ;
	char	*sp, *cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	memset(pip,0,sizeof(struct proginfo)) ;

	progname = strbasename(argv[0]) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0664) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(pip->efp,BC_NOBUF,0) ;
	}

	if (argc < 2)
	    goto earlyret ;

	if (bopen(ifp,BFILE_STDIN,"r",0666) < 0)
	    goto earlyret ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0) {
	    bclose(ifp) ;
	    goto earlyret ;
	}

#if	CF_DEBUGS
	debugprintf("main: 'open()' file=%s\n",argv[1]) ;
#endif

	if ((fd = u_open(argv[1],O_RDWR,0666)) < 0) {

	    bprintf(pip->efp,"%s: could not open mapfile rs=%d\n",
	        progname,fd) ;

	    ex = EX_NOINPUT ;
	    goto done ;
	}

#if	CF_DEBUGS
	debugprintf("main: 'mapfile()'\n") ;
#endif

	rs = u_mmap(NULL,BUFLEN, (PROT_WRITE | PROT_READ) ,MAP_SHARED,
	    fd,0L,&buf) ;

	if ((rs < 0) || (buf == NULL)) {

	    bprintf(pip->efp,"%s: could not map the file rs=%d\n",
	        progname,rs) ;

	    ex = EX_DATAERR ;
	    goto done ;

	}


/* what about initialization */

	u_time(&daytime) ;

	if (argc > 2) {

	    int	*ip = (int *) buf ;


	    ip[0] = (int) daytime ;
	    ip[1] = 0 ;

	} /* end if (initialization) */


/* do the loops */

	while (TRUE) {

	    bprintf(ofp,"> ") ;

	    rs = breadline(ifp,linebuf,LINELEN) ;

	    if (rs < 0)
	        break ;

	    len = rs ;
	    if (len <= 0)
	        break ;

	    sl = sfshrink(linebuf,len,&sp) ;

#if	CF_DEBUGS
	    debugprintf("main: cmd=%W\n",sp,sl) ;
#endif

	    if (sl > 0) {

	        if ((i = matstr(cmds,sp,sl)) >= 0) {

#if	CF_DEBUGS
	            debugprintf("main: match i=%d\n",i) ;
#endif

	            switch (i) {

	            case CMD_READ:
	                rs = cmd_read(pip,fd,buf,ofp) ;

	                break ;

	            case CMD_READLOCK:
	                rs = cmd_readlock(pip,fd,buf,ofp) ;

	                break ;

	            case CMD_READSYNC:
	                rs = cmd_readsync(pip,fd,buf,ofp) ;

	                break ;

	            case CMD_UPDATE:
	                rs = cmd_update(pip,fd,buf,ofp) ;

	                break ;

	            default:
	                bprintf(ofp,"program error (command switch)\n") ;

	            } /* end switch (commands) */

	        } else
	            bprintf(ofp,"invalid command\n") ;

	    } /* end if (non-empty line) */

	} /* end while (reading and executing commands) */


	u_close(fd) ;


	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


done:
	bclose(ofp) ;

	bclose(ifp) ;

earlyret:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;

badret:
	ex = EX_DATAERR ;
	goto earlyret ;
}
/* end subroutine (main) */


static void int_alarm(sig)
int	sig ;
{

	f_alarm = TRUE ;
}


static void int_signal(sig)
int	sig ;
{

	f_signal = TRUE ;
}


static int cmd_read(pip,fd,buf,ofp)
struct proginfo	*pip ;
int		fd ;
char		buf[] ;
bfile		*ofp ;
{
	int	rs ;
	int	*ip = (int *) buf ;

	char	timebuf[TIMEBUFLEN + 1] ;


	bprintf(ofp,"%s %d\n",
	    timestr_log((time_t) ip[0],timebuf),
	    ip[1]) ;

	return 0 ;
}
/* end subroutine (cmd_read) */


static int cmd_readlock(pip,fd,buf,ofp)
struct proginfo	*pip ;
int		fd ;
char		buf[] ;
bfile		*ofp ;
{
	int	rs ;
	int	*ip = (int *) buf ;

	char	timebuf[TIMEBUFLEN + 1] ;


	rs = lockfile(fd,F_RLOCK,0L,0) ;

	if (rs >= 0) {

	    bprintf(ofp,"%s %d\n",
	        timestr_log((time_t) ip[0],timebuf),
	        ip[1]) ;

	    lockfile(fd,F_UNLOCK,0L,0) ;

	} else
	    bprintf(ofp,"lockfile() rs=%d\n",rs) ;

	return rs ;
}
/* end subroutine (cmd_readlock) */


static int cmd_readsync(pip,fd,buf,ofp)
struct proginfo	*pip ;
int		fd ;
char		buf[] ;
bfile		*ofp ;
{
	int	rs ;
	int	*ip = (int *) buf ;

	char	timebuf[TIMEBUFLEN + 1] ;


	rs = uc_msync(buf,BUFLEN,MS_INVALIDATE) ;

	if (rs >= 0) {

	    bprintf(ofp,"%s %d\n",
	        timestr_log((time_t) ip[0],timebuf),
	        ip[1]) ;

	    lockfile(fd,F_UNLOCK,0L,0) ;

	} else
	    bprintf(ofp,"uc_msync() rs=%d\n",rs) ;

	return rs ;
}
/* end subroutine (cmd_readsync) */


static int cmd_update(pip,fd,buf,ofp)
struct proginfo	*pip ;
int		fd ;
char		buf[] ;
bfile		*ofp ;
{
	time_t	daytime ;

	int	rs ;
	int	*ip = (int *) buf ;
	int	wtime, val ;

	char	timebuf[TIMEBUFLEN + 1] ;


	rs = lockfile(fd,F_WLOCK,0L,0) ;

	if (rs >= 0) {

	    wtime = (time_t) ip[0] ;
	    val = ip[1] ;

	    u_time(&daytime) ;

	    ip[0] = (int) daytime ;
	    ip[1] = val + 1 ;

	    lockfile(fd,F_UNLOCK,0L,0) ;

	    bprintf(ofp,"%s %d\n",
	        timestr_log(wtime,timebuf),
	        val) ;

	} else
	    bprintf(ofp,"lockfile() rs=%d\n",rs) ;

	return rs ;
}
/* end subroutine (cmd_update) */


