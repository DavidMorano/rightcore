/* main */

/* program test shared memory mapped file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	F_SLEEP		0


/* revision history:

	= 88/01/10, David A­D­ Morano

	This subroutine was written to test the 'u_mmap' subroutine.


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
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	LINELEN		100

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif

#define	MAPSIZE		(1024)
#define	OFLAGS		(O_RDWR | O_CREAT)

#define	NTIMES		100



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
int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	sigset_t		signalmask ;

	struct termios		ots, nts ;

	struct sigaction	sigs ;

	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	int	ex = EX_USAGE ;
	int	rs, len ;
	int	i, j ;
	int	mapsize, pagesize, prot ;
	int	fd = -1 ;
	int	fd_debug ;
	int	*buf1, *buf2 ;

	char	tmpbuf[BUFLEN + 1] ;
	char	*progname ;
	char	*fname = NULL ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_NOBUF,0) ;


	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0)
	    goto ret1 ;


/* mapped file */

	fname = "mapfile" ;
	if ((argc > 1) && (argv[1] != NULL))
		fname = argv[1] ;

	rs = u_open(fname,OFLAGS,0666) ;

	fd = rs ;
	if (rs < 0) {

	    bprintf(efp,"%s: could not open mapfile (%d)\n",
	        progname,rs) ;

	    goto badret ;
	}

/* write the file */

	mapsize = MAPSIZE ;
	pagesize = getpagesize() ;

	uc_malloc(mapsize,&buf1) ;

	memset(buf1,0,mapsize) ;

	rs = u_write(fd,buf1,mapsize) ;

#if	CF_DEBUGS
	debugprintf("main: u_write() rs=%d\n",rs) ;
#endif

	free(buf1) ;


/* get ready for mapping */

	prot = (PROT_WRITE | PROT_READ) ;

/* first map */

#if	CF_DEBUGS
	debugprintf("main: mapfile()\n") ;
#endif

	rs = u_mmap(NULL,(size_t) mapsize,prot,MAP_SHARED,
	    fd,0L,&buf1) ;

#if	CF_DEBUGS
	debugprintf("main: mapfile() rs=%d\n",rs) ;
#endif

	if ((rs < 0) || (buf1 == NULL)) {

	    bprintf(efp,"%s: could not map the file (%d)\n",
	        progname,rs) ;

	    goto badret ;

	} else
	    bprintf(ofp,"map1 buf1=%p\n",
	        buf1) ;

/* second map */

#if	CF_DEBUGS
	debugprintf("main: mapfile()\n") ;
#endif

	rs = u_mmap(NULL,(size_t) mapsize,prot,MAP_SHARED,
	    fd,0L,&buf2) ;

	if ((rs < 0) || (buf2 == NULL)) {

	    bprintf(efp,"%s: could not map the file (%d)\n",
	        progname,rs) ;

	    goto badret ;

	} else
	    bprintf(ofp,"map2 buf2=%p\n",
	        buf2) ;


#if	CF_DEBUGS
	debugprintf("main: test read of buffer\n") ;
#endif

	{
		int	n, sum ;


	n = mapsize / sizeof(int) ;

	bprintf(ofp,"mapsize=%u\n",mapsize) ;

	bprintf(ofp,"n=%u\n",n) ;

	for (j = 0 ; j < n ; j += 1) {

	for (i = 0 ; i < NTIMES ; i += 1) {

		buf1[j] += 1 ;
		buf2[j] += 1 ;

	} /* end for */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("main: about to sum\n") ;
#endif

	sum = 0 ;
	for (i = 0 ; i < n ; i += 1) {

		sum += buf1[i] ;

	} /* end for */

	bprintf(ofp,"sum=%u\n",sum) ;

	} /* end block */


/* sleep here for a map dump of our process virtual address space */

#if	F_SLEEP
	sleep(30) ;
#endif

/* get out */

	u_munmap(buf2,(size_t) mapsize) ;

	u_munmap(buf1,(size_t) mapsize) ;

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



