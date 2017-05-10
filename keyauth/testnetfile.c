/* testnetfile */

/* test NETFILE object */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_PRINTOUT	1		/* print out on STDOUT */


/* revision history:

	= 1999-08-17, David A­D­ Morano
	This subroutine was partly taken from the LOGDIR-LOGNAME program (fist
	written for the SunOS 4.xx environment in 1989).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testnetfile <file>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<bfile.h>
#include	<getxusername.h>
#include	<netfile.h>
#include	<vecstr.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"testnetfile.h"


/* local defines */


/* external subroutines */

extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	getnodedomain(char *,char *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	getournetname(char *,int,cchar *) ;
extern int	authfile(char *,char *,cchar *) ;
extern int	onckeyalready(const char *) ;
extern int	onckeygetset(cchar *,const char *) ;
extern int	havenis() ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	FILE		*ofp = stdout ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs ;
	int		rs1 ;
	cchar		*cp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((argc > 1) && (argv[1] != '\0')) {
	NETFILE		nfile ;
	NETFILE_ENT	*nep ;
	int		j ;
	int		f_keyed = FALSE ;
	cchar		*fname = argv[1] ;
	if ((rs = netfile_open(&nfile,fname)) >= 0) {
	    NETFILE_ENT		*nep ;
	    cchar		*fmt ;

	    for (j = 0 ; netfile_get(&nfile,j,&nep) >= 0 ; j += 1) {
	        if (nep != NULL) {

#if	CF_DEBUGS
		debugprintf("main: ent¬\n") ;
		debugprintf("main: m=%s\n",nep->machine) ;
		debugprintf("main: l=%s\n",nep->login) ;
		debugprintf("main: p=%s\n",nep->password) ;
#endif

#if	CF_PRINTOUT
		fprintf(ofp,"*\n") ;
		if (nep->machine != NULL)
		fprintf(ofp,"m=%s\n",nep->machine) ;
		if (nep->login != NULL)
		fprintf(ofp,"l=%s\n",nep->login) ;
		if (nep->password != NULL)
		fprintf(ofp,"p=%s\n",nep->password) ;
#endif /* CF_PRINTOUT */

		} /* end if (non-null) */
	        if (f_keyed) break ;
		if (rs < 0) break ;
	    } /* end for */

	    rs1 = netfile_close(&nfile) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (netfile) */
	} /* end if (arg) */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        cchar		*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",
	            mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        }
	        ucmallreg_curend(&cur) ;
	    }
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugclose() ;
#endif

	return 0 ;
}
/* end subroutine (testnetfile) */


