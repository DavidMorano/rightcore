/* logfile_printfold */

/* perform logging operations on a file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This object module was originally written to create a logging mechanism
        for PCS application programs.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an extra method for LOGFILE.


*******************************************************************************/


#define	LOGFILE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<linefold.h>
#include	<localmisc.h>

#include	"logfile.h"


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	mklogid(char *,int,const char *,int,int) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	getnodename(char *,int) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;
extern int	charcols(int,int,int) ;
extern int	isprintlatin(int) ;
extern int	iceil(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwset(char *,int,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int logfile_printfi(LOGFILE *,int,cchar *,int,cchar *,int) ;


/* local variables */


/* exported subroutines */


int logfile_printfold(LOGFILE *lhp,cchar *pre,cchar *sp,int sl)
{
	LINEFOLD	fo ;
	const int	pl = strlen(pre) ;
	int		rs ;
	int		rs1 ;
	int		n ;
	n = (LOGFILE_FMTLEN - pl - 2) ;
	if ((rs = linefold_start(&fo,n,0,sp,sl)) >= 0) {
	    int		i ;
	    int		c = 0 ;
	    cchar	*sp ;
	    for (i = 0 ; (rs = linefold_getline(&fo,i,&sp)) > 0 ; i += 1) {
		int	sl = rs ;
		while (sl > n) {
	            rs = logfile_printfi(lhp,c++,pre,pl,sp,n) ;
		    sl -= n ;
		    sp = (sp+n) ;
		    if (rs < 0) break ;
		} /* end while */
		if ((rs >= 0) && (sl > 0)) {
	            rs = logfile_printfi(lhp,c++,pre,pl,sp,sl) ;
		} /* end if */
		if (rs < 0) break ;
	    } /* end for */
	    rs1 = linefold_finish(&fo) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (linefold) */
	return rs ;
}
/* end subroutine (logfile_printfold) */


/* local subroutines */


static int logfile_printfi(LOGFILE *lhp,int li,cchar *pp,int pl,
		cchar *sp,int sl)
{
	int		rs ;
	if (li == 0) {
	    cchar	*f0 = "%s| %t" ;
	    rs = logfile_printf(lhp,f0,pp,sp,sl) ;
	} else {
	    cchar	*f1 = "%*s| %t" ;
	    rs = logfile_printf(lhp,f1,pl," ",sp,sl) ;
	}
	return rs ;
}
/* end subroutine (logfile_printfi) */


