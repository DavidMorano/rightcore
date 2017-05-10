/* main */

/* program to test the SAVE/RESTORE feature of terminals */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	F_CATCH		0


/* revision history:

	= 88/01/01, David A­D­ Morano

	This subroutine (it's the whole program also)
	was originally written.


*/



/************************************************************************

	This program tests to see if the SAVE/RESTORE feature of
	terminals works.


***************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<sys/conf.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<termstr.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	LINELEN
#define	LINELEN		2048		/* must be greater then 1024 */
#endif

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#define	O_CLIFLAGS	(O_WRONLY | O_CREAT | O_NONBLOCK)
#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct termios		ots, nts ;

	struct sigaction	sigs ;

	bfile	outfile, *ofp = &outfile ;
	bfile	errfile, *efp = &errfile ;

	sigset_t		signalmask ;

	int	ex = EX_INFO ;
	int		len, rs ;
	int		i, j ;
	int		pipefds[2] ;
	int		fd_debug ;

	char	buffer[BUFLEN + 1], *bp ;
	char	tmpbuf[100] ;
	char	*progname ;
	char	*cp ;


	cp = getenv(VARDEBUGFD) ;

	if ((cp != NULL) && (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	f_signal = FALSE ;
	f_alarm = FALSE ;


	bopen(&outfile,BFILE_STDOUT,"wct",0666) ;


	bprintf(&outfile,"this is before..^") ;

	bprintf(&outfile,"%s%12.0H%s\nthis is in the middle%s\n%s%s",
		TERMSTR_SAVE,
		TERMSTR_EL, TERMSTR_EL, TERMSTR_EL,
		TERMSTR_RESTORE) ;

	bprintf(&outfile,"..this is afterwards\n") ;


	bclose(&outfile) ;


	ex = EX_OK ;


done:
ret1:
retearly:
	bclose(efp) ;

ret0:
	return ex ;

badin:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: could not open input, rs=%d\n",
	    progname,rs) ;

	goto retearly ;

badout:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open output, rs=%d\n",
	    progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */


void int_alarm(sig)
int	sig ;
{

	f_alarm = TRUE ;
}


void int_signal(sig)
int	sig ;
{

	f_signal = TRUE ;
}


/* the server reads */
static int readthem(name,fp)
char	name[] ;
bfile	*fp ;
{
	int	rs, len ;
	int	fd ;

	char	linebuf[LINELEN + 1] ;


#if	CF_DEBUGS
	debugprintf("readthem: name=%s\n",name) ;
#endif

/* make the FIFO and open it for reading in message discard mode */

	(void) uc_mkfifo(name,0666) ;

	if ((rs = u_open(name,O_RDONLY | O_NONBLOCK,0666)) < 0)
	    return rs ;

#if	CF_DEBUGS
	debugprintf("readthem: open rs=%d\n",rs) ;
#endif

	fd = rs ;

/* set message discard mode */

	rs = u_ioctl(fd,I_SRDOPT,RMSGD) ;

#if	CF_DEBUGS
	debugprintf("readthem: ioctl rs=%d\n",rs) ;
#endif

	while (! f_signal) {

	    int	time ;


	    time = 0 ;
	    while (TRUE) {

	        len = u_read(fd,linebuf,LINELEN) ;

#if	CF_DEBUGS
	        if (len == 0)
	            debugprintf("readthem: no writers !!\n") ;
#endif

	        if ((len != SR_AGAIN) && (len != 0)) 
			break ;

	        if (f_signal) break ;

	        sleep(1) ;

#if	CF_DEBUGS
	        time += 1 ;
	        if (time >= 4) {

	            time = 0 ;
	            debugprintf("readthem: looping\n") ;

	        }
#endif /* CF_DEBUGS */

	    } /* end while */

	    if (len < 0) break ;

#if	CF_DEBUGS
	    debugprintf("readthem: receive len=%d\n",len) ;
#endif

	    rs = bwrite(fp,linebuf,len) ;

#if	CF_DEBUGS
	    debugprintf("readthem: bwrite rs=%d\n",rs) ;
#endif

	} /* end while */

#if	F_UNLINK
	u_unlink(name) ;
#endif

	u_close(fd) ;

	if (f_signal)
		len = SR_INTR ;

#if	CF_DEBUGS
	debugprintf("readthem: exiting len=%d\n",len) ;
#endif

	return len ;
}
/* wnd subroutine (readthem) */


/* the client writes */
static int writethem(name,fp)
char	name[] ;
bfile	*fp ;
{
	struct ustat	sb ;

	int	rs, len ;
	int	fd ;

	char	linebuf[LINELEN + 1] ;


	rs = u_stat(name,&sb) ;

#if	CF_DEBUGS
	    debugprintf("writethem: stat rs=%d mode=%08o\n",rs,sb.st_mode) ;
#endif

	if (rs < 0) {

	    (void) uc_mkfifo(name,0666) ;

	} else if (! S_ISFIFO(sb.st_mode)) {

	    (void) u_unlink(name) ;

	    (void) uc_mkfifo(name,0666) ;

	}

	if ((rs = u_open(name,O_CLIFLAGS,0666)) < 0) {

#if	CF_DEBUGS
	    debugprintf("writethem: open rs=%d\n",rs) ;

	    if (rs == SR_NXIO)
	        debugprintf("writethem: no readers !\n") ;
#endif

	    return rs ;
	}

	fd = rs ;
	while (! f_signal) {

	    if ((len = breadline(fp,linebuf,LINELEN)) <= 0) 
		break ;

#if	CF_DEBUGS
	        debugprintf("writethem: breadline len=%d\n",len) ;
#endif

	    while (((rs = u_write(fd,linebuf,len)) < 0) &&
			((rs == SR_HANGUP) || (rs == SR_PIPE))) {

#if	CF_DEBUGS
	        debugprintf("writethem: write rs=%d\n",rs) ;
	        debugprintf("writethem: no readers !\n") ;
#endif



	        sleep(1) ;

	        if (f_signal) 
			break ;

	    } /* end while */

#if	CF_DEBUGS
	        debugprintf("writethem: bottom loop\n") ;
#endif

	} /* end while (reading lines) */

#if	CF_DEBUGS
	debugprintf("writethem: exiting, len=%d\n",len) ;
#endif

	u_close(fd) ;

	return len ;
}
/* wnd subroutine (writethem) */



