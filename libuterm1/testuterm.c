/* main (testuterm) */

/* program to display characters */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* compile-time */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1998-01-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program provides a test for the UNIX Terminal library.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"

#include	"uterm.h"
#include	"termcmd.h"


/* local defines */

#define	BUFLEN	100


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	mkcleanline(char *,int,int) ;
extern int	uterm_readcmd(UTERM *,TERMCMD *,int,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	proctest(struct proginfo *,void *,UTERM *) ;
static int	istermcmd(int) ;


/* local variables */

static const uchar	termcmds[] = { CH_ESC, CH_CSI, CH_DCS, CH_SS3, 0 } ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile	ofile, *ofp = &ofile ;
	bfile	efile ;

	struct termios	saved ;

	UTERM	u ;

	long	lw ;

	uint	mo_start = 0 ;

	int	pan = 0 ;
	int	rs, rs1 ;
	int	i, j ;
	int	len, llen ;
	int	ttfd, tfd = 0 ;
	int	fd_debug ;
	int	ex = EX_INFO ;
	int	f_exit = FALSE ;

	uchar	c ;
	uchar	buf[BUFLEN + 1] ;
	uchar	*bp ;

	const char	*progname ;
	const char	*argval = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	if (bopen(&efile,BFILE_STDERR,"dwca",0664) >= 0) {
	    pip->efp = &efile ;
	    bcontrol(pip->efp,BC_NOBUF,0) ;
	}

	if ((ofname == NULL) || (ofname[0] == '-')) ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"dwct",0666)) >= 0) {

	    tfd = FD_STDIN ;
	    ttfd = u_dup(tfd) ;

	    uc_tcgetattr(ttfd,&saved) ;

#if	CF_DEBUGS
	    debugprintf("main: about to call 'uterm_start'\n") ;
#endif

	    if ((rs = uterm_start(&u,tfd)) >= 0) {

	        f_exit = FALSE ;
	        while ((rs >= 0) && (! f_exit)) {

	            rs = proctest(pip,ofp,&u) ;

	            f_exit = (rs > 0) ;
	        } /* end while */

	        uterm_finish(&u) ;
	    } /* end if (uterm) */

	    uc_tcsetattr(ttfd,TCSADRAIN,&saved) ;

	    bclose(ofp) ;
	} /* end if (opened-std-output) */

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: BFILE-mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	proginfo_finish(pip) ;

badprogstart:

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int proctest(struct proginfo *pip,void *ofp,UTERM *utp)
{
	const int	rlen = BUFLEN ;
	const int	to = 10 ;
	const int	to_cmd = 10 ;

	int	rs = SR_OK ;
	int	fc ;
	int	len = 0 ;

	char	rbuf[BUFLEN+1] ;


	fc = (fm_rawin | fm_noecho) ;
	rs = uterm_reade(utp,rbuf,rlen,to,fc,NULL,NULL) ;
	len = rs ;

#if	CF_DEBUGS
	debugprintf("testuterm/proctest: uterm_reade() rs=%d\n",rs) ;
	debugprintf("testuterm/proctest: rbuf=>%t<\n",
	    rbuf,strlinelen(rbuf,len,40)) ;
#endif

	if ((rs >= 0) && (len > 0)) {
	    const int	ch = (rbuf[len-1] & UCHAR_MAX) ;
	    int		cmdidx ;
	    if ((cmdidx = istermcmd(ch)) > 0) {
	        TERMCMD	cmd ;
#if	CF_DEBUGS
	debugprintf("testuterm/proctest: istermcmd cmdidx=%u\n",cmdidx) ;
#endif
	        rs = uterm_readcmd(utp,&cmd,to_cmd,ch) ;
	        if (rs >= 0) {
	            int	clen = rs ;
	            int	pi ;

	            bprintf(ofp,"cmdtype=%u\n",cmd.type) ;
	            bprintf(ofp,"cmdname=%c\n",cmd.name) ;
	            for (pi = 0 ; (cmd.p[pi] >= 0) && (pi < 16) ; pi += 1) {
	                bprintf(ofp,"param[%u]=%u\n",pi,cmd.p[pi]) ;
	            } /* end for (CMD parameters) */
	                bprintf(ofp,"intermediates=>%s< (%u)\n",
	                    cmd.istr,cmd.f_iover) ;
	                bprintf(ofp,"deviceinfo=>%s< (%u)\n",
	                    cmd.dstr,cmd.f_dover) ;

	        } /* end if (got a terminal CMD) */
	    } /* end if (istermcmd) */
	} /* end if (check for terminal CMD) */

	if (rs >= 0) {
	    mkcleanline(rbuf,len,1) ;
	    bprintf(ofp,"len=%d line>%t<\n",len,
	        rbuf,strlinelen(rbuf,len,40)) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (proctest) */


static int istermcmd(int ch)
{
	int	tch ;
	int	i = 0 ;
	int	f = FALSE ;
	for (i = 0 ; termcmds[i] != 0 ; i += 1) {
	    tch = (termcmds[i] & UCHAR_MAX) ;
	    f = (ch == tch) ;
	    if (f) break ;
	}
	return (f) ? (i+1) : 0 ;
}
/* end subroutine (istermcmd) */


