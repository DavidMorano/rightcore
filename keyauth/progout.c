/* progout */

/* handle program output */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1999-08-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We manage optional program output.

	Synopsis:

	int progout_begin(PROGINFO *pip)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<bfile.h>
#include	<netfile.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	onckeyalready(const char *) ;
extern int	onckeygetset(cchar *,const char *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	progout_check(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int progout_begin(PROGINFO *pip,cchar *ofn)
{
	int		rs = SR_OK ;
	if (ofn != NULL) {
	    cchar	**vpp = &pip->ofname ;
	    rs = proginfo_setentry(pip,vpp,ofn,-1) ;
	}
	return rs ;
}
/* end subroutine (progout_begin) */


int progout_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.outfile) {
	    pip->open.outfile = FALSE ;
	    if (pip->outfile != NULL) {
	        rs1 = bclose(pip->outfile) ;
	        if (rs >= 0) rs = rs1 ;
		rs1 = uc_free(pip->outfile) ;
	        if (rs >= 0) rs = rs1 ;
		pip->outfile = NULL ;
	    }
	}
	return rs ;
}
/* end subroutine (progout_end) */


int progout_printline(PROGINFO *pip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip->verboselevel > 0) {
	    if ((rs = progout_check(pip)) >= 0) {
	        rs = bprintline(pip->outfile,sp,sl) ;
	        wlen = rs ;
	    }
	} /* end if (verboselevel) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progout_printline) */


int progout_printf(PROGINFO *pip,const char *fmt,...)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip->verboselevel > 0) {
	    if ((rs = progout_check(pip)) >= 0) {
	        va_list	ap ;
	        va_begin(ap,fmt) ;
	        rs = bvprintf(pip->outfile,fmt,ap) ;
	        wlen = rs ;
	        va_end(ap) ;
	    }
	} /* end if (verboselevel) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progout_printf) */


/* local subroutines */


static int progout_check(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->outfile == NULL) {
	    int		size = sizeof(bfile) ;
	    cchar	*ofn = pip->ofname ;
	    void	*p ;
	    if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-')) {
		ofn = BFILE_STDOUT ;
	    }
	    if ((rs = uc_malloc(size,&p)) >= 0) {
		pip->outfile = p ;
	        if ((rs = bopen(pip->outfile,ofn,"wct",0666)) >= 0) {
		    pip->open.outfile = TRUE ;
		}
		if (rs < 0) {
		    uc_free(pip->outfile) ;
		    pip->outfile = NULL ;
		}
	    } /* end if (m-a) */
	} /* end if (initialization) */
	return rs ;
}
/* end subroutine (progout_check) */


