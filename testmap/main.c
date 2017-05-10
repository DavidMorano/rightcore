/* main */

/* program to read a shared memory mapped file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0


/* revision history:

	= 88/01/10, David A­D­ Morano

	This subroutine was written to test the 'u_mmap(3u)' subroutine.


*/



/************************************************************************

	This subroutine is the main part of a program that
	tests file mapping concepts.



***************************************************************************/



#include	<sys/types.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<stdlib.h>

#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"



/* local defines */

#define	LINELEN		100
#define	BUFLEN		1024
#define	MAXLOOP		30



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





/* start of program */
int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	sigset_t		signalmask ;

	struct termios		ots, nts ;

	struct sigaction	sigs ;

	int	rs, len ;
	int	i, j ;
	int	fd = 0 ;
	int	ex = EX_USAGE ;
	int	fd_debug ;

	char	*progname ;
	char	tmpbuf[BUFLEN + 1] ;
	char	*buf, *bp ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_NOBUF,0) ;


	if (argc < 2)
	    goto ret1 ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0)
	    goto ret1 ;



#if	CF_DEBUGS
	debugprintf("main: 'open()' file=%s\n",argv[1]) ;
#endif

	if ((fd = u_open(argv[1],O_RDWR,0666)) < 0) {

	    bprintf(efp,"%s: could not open mapfile rs=%d\n",
	        progname,fd) ;

	    goto badret ;
	}

#if	CF_DEBUGS
	debugprintf("main: 'mapfile()'\n") ;
#endif

	rs = u_mmap(NULL,BUFLEN, (PROT_WRITE | PROT_READ) ,MAP_SHARED,
	    fd,0L,&buf) ;

	if ((rs < 0) || (buf == NULL)) {

	    bprintf(efp,"%s: could not map the file rs=%d\n",
	        progname,rs) ;

	    goto badret ;

	} else
	    bprintf(efp,"%s: buf=%08X\n",
	        progname,buf) ;


#if	CF_DEBUGS
	debugprintf("main: test read of buffer\n") ;
#endif

	tmpbuf[0] = buf[0] ;

#if	CF_DEBUGS
	debugprintf("main: about to 'for 1'\n") ;
#endif

	for (i = 0 ; i < MAXLOOP ; i += 1) {

		int	lines ;


	    bprintf(ofp,"%H%J") ;

#if	CF_DEBUGS
	    debugprintf("main: 'for 1' i=%d\n",i) ;
#endif

	    if ((i % 3) == 0) {

	        rs = uc_msync(buf,BUFLEN,MS_INVALIDATE) ;

	        if (rs < 0) {

	            bprintf(efp,"%s: bad 'msync()' rs=%d\n",
	                progname,rs) ;

			goto badret ;
	        }

	        bprintf(ofp,"%Hfilename: %s (msync)\n%J",argv[1]) ;

	    } else
	        bprintf(ofp,"%Hfilename: %s\n%J",argv[1]) ;

#if	CF_DEBUGS
	    debugprintf("main: bp=buf\n") ;
#endif

	    bp = buf ;

#if	CF_DEBUGS
	    debugprintf("main: bp[0]=0\n") ;
#endif

#ifdef	COMMENT
		*bp = '\0' ;
#endif /* COMMENT */

#if	CF_DEBUGS
	    debugprintf("main: about to 'for 2'\n") ;
#endif

		lines = 0 ;
	    for (j = 0 ; (lines < 10) && (j < BUFLEN) && 
		(*bp != '\0') ; j += 1) {

#if	CF_DEBUGS
	        debugprintf("main: 'bputc()' \n") ;
	        debugprintf("main: 'bputc()' c=%c\n",*bp) ;
#endif

	        bputc(ofp,*bp) ;

		if (*bp == '\n')
			lines += 1 ;

		bp += 1 ;

	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("main: 'bflush()'\n") ;
#endif

	    bflush(ofp) ;

#if	CF_DEBUGS
	    debugprintf("main: 'sleep()'\n") ;
#endif

	    sleep(10) ;

	} /* end for */

	u_close(fd) ;


	ex = EX_OK ;


done:
	bclose(ofp) ;

ret1:
	bclose(efp) ;

ret0:
	return ex ;

badret:
	bclose(efp) ;

	return EX_DATAERR ;
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



