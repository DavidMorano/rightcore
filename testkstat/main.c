/* main */

/* test out Sun Solaris UNIX 'kstat's */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1
#define	CF_TESTSYSINFO	0


/* revision history:

	= 1988-01-10, David A­D­ Morano

	This subroutine was written (originally) as a test of
	the Sun Solaris UNIX 'kstat' facility.


*/

/* Copyright © 1988 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Test the Sun Solaris UNIX® 'kstat' facility.



*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<sys/systeminfo.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<kstat.h>

#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	TO		11


/* external subroutines */

extern int	sfbasename(const char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;

static int	bprintla() ;


/* gloabal variables */

static volatile int		f_alarm ;
static volatile int		f_signal ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct sigaction	sigs ;
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;
	sigset_t	signalmask ;
	kstat_ctl_t	*kcp ;
	kstat_t		*ksp = NULL ;
	kid_t		kid ;
	long		lw ;
	const uint	hostid = gethostid() ;
	int		rs = SR_OK ;
	int		maxmsglen, trylen, len, conlen ;
	int		pagesize, ppm ;
	int		i, j ;
	int		sl ;
	int		s ;
	int		fd = -1 ;
	int		fd_debug = -1 ;
	const char	*progname ;
	cchar		*cp ;
	char		buf[BUFLEN + 1], buf2[BUFLEN + 1], *bp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
		cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) && (cfdeci(cp,-1,&fd_debug) >= 0)) {
	    debugsetfd(fd_debug) ;
	} else
		debugsetfd(-1) ;


	sfbasename(argv[0],-1,&progname) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;


/* print out some common accessible machine parameters */

	sl = u_sysinfo(SI_HW_PROVIDER,buf,BUFLEN) ;

#if	CF_DEBUGS
	debugprintf("main: hw_provider rs=%d\n",sl) ;
#endif

	bprintf(ofp,"provider> %W\n",buf,sl) ;


	sl = u_sysinfo(SI_HW_SERIAL,buf,BUFLEN) ;

	bprintf(ofp,"serial> %W\n",buf,sl) ;


#if	CF_TESTSYSINFO
	for (i = 7 ; i < 12 ; i += 1) {
	    rs = u_sysinfo(SI_HW_SERIAL,buf,i) ;
	    bprintf(ofp,"testsysinfo> len=%d rs=%d\n",i,rs) ;
	}
#endif /* CF_TESTSYSINFO */

#if	CF_DEBUGS
	debugprintf("main: hostid rs=%d\n",rs) ;
#endif

	bprintf(ofp,"hostid> %u (%08x)\n",
	    (uint) hostid,
	    (uint) hostid) ;


/* some extra host information */

	pagesize = getpagesize() ;

	ppm = (1024 * 1024) / pagesize ;

	uc_sysconf(_SC_PHYS_PAGES,&lw) ;

	bprintf(ofp,"physical pages=%0ld (%ld MiBytes)\n",
		lw,(lw / ppm)) ;

	uc_sysconf(_SC_AVPHYS_PAGES,&lw) ;

	bprintf(ofp,"available physical pages=%0ld (%ld MiBytes)\n",
		lw,(lw / ppm)) ;


/* OK, on to the KSTAT stuff */

#if	CF_DEBUGS
	debugprintf("main: about to open the KSTATs\n") ;
#endif

	kcp = kstat_open() ;

	if (kcp != NULL) {

/* print out all module names */

	    if (argc > 1) {

#if	CF_DEBUGS
	        debugprintf("main: looping through names\n") ;
#endif

	        for (ksp = kcp->kc_chain ; ksp != NULL ; ksp = ksp->ks_next) {

	            bprintf(ofp,"module=%s name=%s type=%d\n",
	                ksp->ks_module,ksp->ks_name,ksp->ks_type) ;

	        } /* end for */

	        bflush(ofp) ;

	    } /* end if (extra printing) */


/* read out and print the load average */

#if	CF_DEBUGS
	    debugprintf("main: lookup 'system_misc' !\n") ;
#endif

	    ksp = kstat_lookup(kcp, "unix", 0, "system_misc") ;

	    if (ksp != NULL) {

	        long	la ;


#if	CF_DEBUGS
	        debugprintf("main: got a 'system_misc'\n") ;
#endif

	        if ((kid = kstat_read(kcp, ksp, NULL)) >= 0) {

	            kstat_named_t *ksn ;


/* now just print out all values as their types */

#if	CF_DEBUGS
	            debugprintf("main: read some data, kid=%d\n",kid) ;
#endif

	            bprintf(ofp,"system_localmisc.has %d records\n",
	                ksp->ks_ndata) ;

	            ksn = (kstat_named_t *) ksp->ks_data ;
	            for (i = 0 ; i < ksp->ks_ndata ; i += 1) {

	                bprintf(ofp,"misc> %W %d\n",
	                    ksn->name,strnlen(ksn->name,KSTAT_STRLEN),
	                    (int) ksn->data_type) ;

	                switch (ksn->data_type) {

	                case KSTAT_DATA_CHAR:
	                    bprintf(ofp,"| %c\n",
	                        ksn->value.c[0]) ;

	                    break ;

	                case KSTAT_DATA_INT32:
	                    bprintf(ofp,"| %d\n",
	                        ksn->value.i32) ;

	                    break ;

	                case KSTAT_DATA_UINT32:
	                    bprintf(ofp,"| %u\n",
	                        ksn->value.ui32) ;

	                    break ;

	                case KSTAT_DATA_INT64:
	                    bprintf(ofp,"| %lld\n",
	                        ksn->value.i64) ;

	                    break ;

	                case KSTAT_DATA_UINT64:
	                    bprintf(ofp,"| %llu\n",
	                        ksn->value.ui64) ;

	                    break ;

	                } /* end switch */

	                ksn += 1 ;

	            } /* end for */


/* OK, we just look at the load average record at this point */

	            ksn = (kstat_named_t *)
	                kstat_data_lookup(ksp,"avenrun_1min") ;

#ifdef	COMMENT
	            if (ksn != NULL) {

	                bprintf(ofp,"ul=%lu\n",ksn->value.ui32) ;

	                {
	                    int	la_i, la_f ;


	                    la_i = ksn->value.ui32 / FSCALE ;
	                    la_f = +((ksn->value.ui32 % FSCALE) * 1000) / 256 ;

	                    bprintf(ofp,"ula=%d.%03d\n",la_i,la_f) ;

	                } /* end block */

	                la = ((double) ksn->value.ui32 + (FSCALE / 2))
	                    / FSCALE ;

	                bprintf(ofp,"la=%ld\n",la) ;

	            } else
	                bprintf(ofp,"* no such load average *") ;

#endif /* COMMENT */

	            bprintla(ofp,ksn,"1min") ;

	            ksn = (kstat_named_t *)
	                kstat_data_lookup(ksp,"avenrun_5min") ;

	            bprintla(ofp,ksn,"5min") ;

	            ksn = (kstat_named_t *)
	                kstat_data_lookup(ksp,"avenrun_15min") ;

	            bprintla(ofp,ksn,"15min") ;


	        } /* end if (reading data) */

#if	CF_DEBUGS
	        debugprintf("main: afterwards kid=%d\n",kid) ;
#endif

	    } /* end if (getting UNIX_MISC) */


	    kstat_close(kcp) ;

	} /* end if (opened) */


	bclose(ofp) ;


done:
	bclose(efp) ;

	return EX_OK ;

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



static int bprintla(ofp,ksn,s)
bfile		*ofp ;
kstat_named_t	*ksn ;
char		s[] ;
{
	int	la_i, la_f ;
	int	la ;
	int	iw ;
	int	rs = BAD ;


	bprintf(ofp,"%s\n",s) ;

	if (ksn != NULL) {

	    bprintf(ofp,"ul=%lu\n",ksn->value.ui32) ;

	    la_i = ksn->value.ui32 / FSCALE ;
	    iw = (ksn->value.ui32 % FSCALE) * 1000 ;
	    la_f = iw / 256 ;

	    bprintf(ofp,"ula=%d.%03d\n",la_i,la_f) ;

	    la = ((double) ksn->value.ui32 + (FSCALE / 2)) ;
	    la = la / FSCALE ;

	    bprintf(ofp,"la=%ld\n",la) ;

	} else {

	    rs = BAD ;
	    bprintf(ofp,"* no such paameter *\n") ;

	}

	return rs ;
}
/* end subroutine (bprintla) */



